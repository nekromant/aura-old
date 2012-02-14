#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <termios.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/cdefs.h>
#include <stdio.h>
#include <lua.h>
#include <errno.h>
#include <lauxlib.h>
#include <string.h>
#include <azra/azra.h>
#include "config.h"


/* We handle all the cmdline stuff here */

static void handle_client_disconnection(struct azra_epoll_hook* h)
{
	struct azra_client_data* cdata = h->data;
	cdata->server->client_count--;
	azra_broadcaster_drop_client(cdata);
	azra_drop_epollhook(h);
	//Now clean up memory
	free(h->data);
	free(h->name);
}

#define is_cmd(cmd) (strncmp(cmd, cdata->inbuf, strlen(cmd))==0)
static int azra_execute_buffer(struct azra_epoll_hook* h)
{
	struct azra_client_data* cdata = h->data;
	int err;
	if (is_cmd("-- tag"))
	{
		azra_broadcastf(cdata, "tag change '%s' -> '%s'\n", h->name, &cdata->inbuf[7]);
		free(h->name);
		h->name=strdup(&cdata->inbuf[7]);
		return 0;
	}
// 	printf("DFGHJ");
    azra_broadcastf(cdata, "%s\n", cdata->inbuf);

//  	azra_broadcastf (cdata, cdata->inbuf, h->name);

	err = luaL_loadbuffer(cdata->server->L, cdata->inbuf, strlen(cdata->inbuf), h->name) ||
                    lua_pcall(cdata->server->L, 0, 0, 0);
            if (err) {
                  azra_cbroadcastf(" %s\n", lua_tostring(cdata->server->L, -1));
                lua_pop(cdata->server->L, 1);
            }
            //Output from lua
            //printf("az");
            fflush(cdata->server->lua_stream);
			if (cdata->server->lua_streamsz)
				azra_cbroadcastf(" %s\n", cdata->server->lua_iodata);
            rewind(cdata->server->lua_stream);
            
	return 0;
}

static int handle_io(struct epoll_event *ev)
{
	struct azra_epoll_hook* h = ev->data.ptr;
	struct azra_client_data* cdata = h->data;
	printf("azra: handling io for client %s\n", h->name);
 	int count=0;
 	int startpos = cdata->inbpos;
 	int i;
	if (ev->events & EPOLLHUP)
	{
		printf("azra: peer disconnected (HUP)\n");
		handle_client_disconnection(h);
		return 0;
	}
	
	if (ev->events & EPOLLERR)
	{
		printf("azra: peer disconnected (ERR)\n");
		handle_client_disconnection(h);
		return 0;
	}
	
	if (ev->events & EPOLLIN)
	{
		while (cdata->inbpos < 4096)
		{
			count = read(h->fd,&cdata->inbuf[cdata->inbpos],AZRA_CLIBUF_SZ-cdata->inbpos);
			if (0 == count)
			{
				printf("azra: peer disconnected\n");
				handle_client_disconnection(h);
				break;
			}
			if ((-1 == count) && (errno == EAGAIN) ) break;
			cdata->inbpos+=count;
		}
		for (i=startpos; i<cdata->inbpos; i++)
		{
// 			printf("%hhx %hhx\n", cdata->inbuf[i],0xfe);
			//Look for linebreak: 0xd 0xa, we expect 0xa to replace with 0x00
			if ('\xfe' == cdata->inbuf[i])
			{
			cdata->inbuf[i]=0x00; 
			//ToDo: Actually interpret the received string
			azra_execute_buffer(h);
			count = cdata->inbpos-i-1;
			memmove(&cdata->inbuf[0],&cdata->inbuf[i+1],count);
			cdata->inbpos=count;
			break;
			}
		}
	}
	if (ev->events & EPOLLOUT)
	{
		printf("azra: ready to write\n");
		struct azra_charbuf* msg;
		while (1)
		{
			msg = broadcaster_get_message(cdata);
			if (NULL==msg)
			{
				printf("azra: No more stuff to transfer\n");
				count=-2;
				break;
			}
			count = write(h->fd,&msg->buffer[cdata->outpos],msg->len - cdata->outpos);
			
			printf("azra: %d/%d bytes at offset %d written for %s\n", count,msg->len,cdata->outpos, cdata->h->name);
			if ((-1 == count) && (errno == EAGAIN) )
				break;
			cdata->outpos+=count;
			if (cdata->outpos == msg->len)
			{
				broadcaster_put_message(cdata,msg);
				cdata->outpos=0;
			}
		}
		if (count==-2) azra_epollout(h,0);
	}
	
	return 0;
}

char* azra_shell_exec(char* cmd,int sz) {
    FILE* pipe = popen(cmd, "r");
    if (!pipe) return "ERROR";
    char *tmp = malloc(sz);
    fgets(tmp, sz, pipe);
    pclose(pipe);
    return tmp;
}

const char welcomestr[] = "<b>azra:</b> Welcome <font color=red>" PACKAGE_STRING "</font>! Send bugs to <font color=green>" PACKAGE_BUGREPORT "</font>\nCurrently running at: <font color=blue>";
const char userstr[] = "You are client number <b>%d</b>\n";
const char loginhook[] = "hook_login()";
const char floginhook[] = "hook_first_login()";

int azra_setup_client(struct azra_epoll_hook* h)
{
	printf("azra: setting up io for '%s'\n", h->name);
	h->io_handler =  handle_io;
	struct azra_client_data *cdata = h->data;
	h->ev.events = EPOLLIN | EPOLLERR | EPOLLHUP;
	azra_make_fd_nonblock(h->fd);
	char* uname = azra_shell_exec("uname -a",1024);
	write(h->fd,welcomestr,strlen(welcomestr));
	write(h->fd,uname,strlen(uname));
	sprintf(uname,userstr,cdata->server->client_count);
	write(h->fd,uname,strlen(uname));
	azra_broadcaster_add_client(cdata);
	free(uname); 
	if (!cdata->server->firstlogin++)
		strcpy(cdata->inbuf,floginhook);
	azra_execute_buffer(h);
	strcpy(cdata->inbuf,loginhook);
	azra_execute_buffer(h);	
    //TODO: Add client to broadcast list
    return  azra_add_epollhook(h);
}

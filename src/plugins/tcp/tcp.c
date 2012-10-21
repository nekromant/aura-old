#include <sys/cdefs.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <sys/cdefs.h>
#include <lua.h>
#include <getopt.h>
#include <lualib.h>
#include <lauxlib.h>
#include <termios.h>
#include <string.h>
#include <aura/aura.h>

#define DEBUG 1
#define COMPONENT "tcp"
#include <aura/debug.h>


/* IO hack. This replaces the io library's idea of stdin/out/err
 * with 3 new FILE* handles, without altering the C stdin/out/err
 * values.
 *
 * It's a hugely ugly hack, fiddling with the internals of the io
 * library. Tested with 5.1.2, may or may not work with other
 * versions...
 */
static void hackio(lua_State *L, FILE *in, FILE *out, FILE *err) {
	DBG("hi-jacking lua io streams...\n");
	FILE **pf;
	lua_getglobal(L, "io"); /* Get the IO library */
	lua_pushstring(L, "open");
	lua_gettable(L, -2); /* io, io.open */
	lua_getfenv(L, -1); /* io, io.open, io.open's env */
	lua_getglobal(L, "io"); /* io, open, env, io */
	
	lua_pushstring(L, "stdin");
	lua_gettable(L, -2); /* io, open, env, io, io.stdin */
	pf = (FILE **)lua_touserdata(L, -1);
	*pf = in;
	lua_rawseti(L, -3, 1); /* IO_INPUT = 1 */
	
	lua_pushstring(L, "stdout");
	lua_gettable(L, -2); /* io, open, env, io, io.stdout */
	pf = (FILE **)lua_touserdata(L, -1);
	*pf = out;
	lua_rawseti(L, -3, 2); /* IO_OUTPUT = 2 */
	
	lua_pushstring(L, "stderr");
	lua_gettable(L, -2); /* io, open, env, io, io.stderr */
	pf = (FILE **)lua_touserdata(L, -1);
	*pf = err;
	lua_pop(L, 5);
}


/*
static int handle_io(struct epoll_event *ev)
{
	struct aura_epoll_hook* h = ev->data.ptr;
	struct aura_client_data* cdata = h->data;
	printf("aura: handling io for client %s\n", h->name);
 	int count=0;
 	int startpos = cdata->inbpos;
 	int i;
	if (ev->events & EPOLLHUP)
	{
		printf("aura: peer disconnected (HUP)\n");
		handle_client_disconnection(h);
		return 0;
	}
	
	if (ev->events & EPOLLERR)
	{
		printf("aura: peer disconnected (ERR)\n");
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
				printf("aura: peer disconnected\n");
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
			aura_execute_buffer(h);
			count = cdata->inbpos-i-1;
			memmove(&cdata->inbuf[0],&cdata->inbuf[i+1],count);
			cdata->inbpos=count;
			break;
			}
		}
	}
	if (ev->events & EPOLLOUT)
	{
		printf("aura: ready to write\n");
		struct aura_charbuf* msg;
		while (1)
		{
			msg = broadcaster_get_message(cdata);
			if (NULL==msg)
			{
				printf("aura: No more stuff to transfer\n");
				count=-2;
				break;
			}
			count = write(h->fd,&msg->buffer[cdata->outpos],msg->len - cdata->outpos);
			
			printf("aura: %d/%d bytes at offset %d written for %s\n", count,msg->len,cdata->outpos, cdata->h->name);
			if ((-1 == count) && (errno == EAGAIN) )
				break;
			cdata->outpos+=count;
			if (cdata->outpos == msg->len)
			{
				broadcaster_put_message(cdata,msg);
				cdata->outpos=0;
			}
		}
		if (count==-2) aura_epollout(h,0);
	}
	
	return 0;
}
*/

int aura_setup_client(struct aura_epoll_hook* h)
{
	printf("aura: setting up io for '%s'\n", h->name);
//	h->io_handler =  handle_io;
	struct aura_client_data *cdata = h->data;
	h->ev.events = EPOLLIN | EPOLLERR | EPOLLHUP;
	aura_make_fd_nonblock(h->fd);
	/* 
	char* uname = aura_shell_exec("uname -a",1024);
	write(h->fd,welcomestr,strlen(welcomestr));
	write(h->fd,uname,strlen(uname));
	sprintf(uname,userstr,cdata->server->client_count);
	write(h->fd,uname,strlen(uname));
	aura_broadcaster_add_client(cdata);
	free(uname); 
	if (!cdata->server->firstlogin++)
		strcpy(cdata->inbuf,floginhook);
	aura_execute_buffer(h);
	strcpy(cdata->inbuf,loginhook);
	aura_execute_buffer(h);	
	//TODO: Add client to broadcast list
	*/
	return  aura_add_epollhook(h);
}


int aura_plugin_init(lua_State* L) 
{
	INF("aura tcp console v 0.1");
	lua_pushnumber(L,1);
	return 1;
}

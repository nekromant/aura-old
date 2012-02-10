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


/* We handle all the cmdline stuff here */

static void handle_client_disconnection(struct azra_epoll_hook* h)
{
	azra_drop_epollhook(h);
	//Now clean up memory
	free(h->data);
	free(h->name);
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
			//Look for linebreak: 0xd 0xa, we expect 0xa to replace with 0x00
			if ((cdata->inbuf[i]==0xa))
			{
			cdata->inbuf[i]=0x00; //
			//ToDo: Actually interpret the received string
			printf("azra: %s\n", cdata->inbuf);
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
	}
	
	return 0;
}

int azra_setup_client(struct azra_epoll_hook* h)
{
//  	struct azra_client_data *cdata = h->data;
	printf("azra: setting up io for '%s'\n", h->name);
	h->io_handler =  handle_io;
	h->ev.events = EPOLLIN | EPOLLET | EPOLLERR | EPOLLHUP;
	azra_make_fd_nonblock(h->fd);
    //TODO: Add client to broadcast list
    return  azra_add_epollhook(h);
}
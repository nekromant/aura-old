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
#include <lauxlib.h>
#include <string.h>
#include "include/azra.h"


/* We handle all the cmdline stuff here */

static int handle_io(struct epoll_event *ev)
{
	printf("azra: handling io for client\n");
	return 0;
}

int azra_setup_client(struct azra_epoll_hook* h)
{
//  	struct azra_client_data *cdata = h->data;
	printf("azra: setting up io for '%s'\n", h->name);
	h->io_handler =  handle_io;
	h->ev.events = EPOLLIN | EPOLLET;
	azra_make_fd_nonblock(h->fd);
    //TODO: Add client to broadcast list
    return  azra_add_epollhook(h);
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#include <errno.h>
#include <string.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <termios.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <aura/aura.h>


static int efd;
static int wait_interval = 200;

int aura_init_loop()
{
	efd = epoll_create(10);
	if (-1 == efd)
	{
		perror("epoll_create: ");
		return 1;
	}
	return 0;
}

int aura_make_fd_nonblock(int sfd)
{
	int flags, s;
	flags = fcntl (sfd, F_GETFL, 0);
	if (flags == -1)
	{
		perror ("fcntl");
		return -1;
	}

	flags |= O_NONBLOCK;
	s = fcntl (sfd, F_SETFL, flags);
	if (s == -1)
	{
		perror ("fcntl");
		return -1;
	}

	return 0;
}

int aura_epollout(struct aura_epoll_hook* hook, int status)
{
	if (status)
		hook->ev.events|=EPOLLOUT;
	else
		hook->ev.events&=~EPOLLOUT;
	if (epoll_ctl(efd, EPOLL_CTL_MOD, hook->fd, &hook->ev)!=0)
	{
		fprintf(stderr, "Failed to add fd to epoll (%d)\n", hook->fd);
		perror("epoll:");
		return 1;
	}
	return 0;
}

int aura_add_epollhook(struct aura_epoll_hook* hook)
{
	hook->ev.data.ptr = (void*) hook;
	if (epoll_ctl(efd, EPOLL_CTL_ADD, hook->fd, &hook->ev)!=0)
	{
		fprintf(stderr, "Failed to add fd to epoll (%d)\n", hook->fd);
		perror("epoll:");
		return 1;
	}
	printf("aura: added hook '%s' to main loop\n", hook->name);
	return 0;
}


void aura_drop_epollhook(struct aura_epoll_hook* hook)
{
	hook->ev.data.ptr = (void*) hook;
	printf("aura: dropping epoll hook: %s\n", hook->name);
	if (epoll_ctl(efd, EPOLL_CTL_DEL, hook->fd, &hook->ev)!=0)
	{
		fprintf(stderr, "BAD!!! Failed to drop a epollhook for fd %d.\n", hook->fd);
		perror("epoll:");
	}
	close(hook->fd);
}


void inline aura_loop_once(lua_State* L)
{
	int c;
	struct epoll_event ev;
	struct aura_epoll_hook *hook;
	printf("aura: entering main loop\n");
	c = epoll_wait(efd, &ev, 1, wait_interval);
	if (c) {
		hook = ev.data.ptr;
		printf("aura: event from %s \n", hook->name);
		hook->io_handler(&ev, L);
	}	
}

void aura_loop_forever(lua_State* L)
{
	while (1) {
		aura_loop_once(L);
	}
}

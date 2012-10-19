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

#define COMPONENT "asyncio"
#define DEBUG 1

#include <aura/debug.h>

struct aura_async_xfer* aura_create_async_xfer(struct aura_epoll_hook* h) 
{
	struct aura_async_xfer* x = malloc(sizeof(struct aura_async_xfer));
	if (!x)
		return NULL;
	INIT_LIST_HEAD(&x->transferlist);
	x->expect_bytes = 0;
	x->readbufer = NULL;
	h->io_handler = aura_async_xfer_handler; /* set up async handler for io */
        int ret = aura_add_epollhook(h);
	if ( 0 != ret )
		goto fail;
	ret = aura_epollout(h, 0); /* We have nothing to write. Yet */
	if (0 != ret)
		goto fail;
        DBG("Created asyncio instance");
	return x;
fail:
	free(x);
	return NULL;
}
 
int aura_async_xfer_handler(struct epoll_event *ev)
{
	DBG("Handling async io...");
	return 0;
}

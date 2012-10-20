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
	x->pos = 0;
	x->recv = NULL;
	h->io_handler = aura_async_xfer_handler; /* set up async handler for io */
	h->data = x;
	x->h = h;
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
	struct aura_epoll_hook *h = ev->data.ptr;
	struct aura_async_xfer *x = h->data;
	struct aura_chunk_queue *q;
	struct list_head *i, *tmp;
	size_t n;
	if (ev->events & EPOLLIN) {
		n = read(h->fd, &x->recv->data[x->recv->size],
			 x->expect_bytes);
		if (n == -1) {
			if (errno == EAGAIN) {
				DBG("Done for now");
			};
			//TODO: handle errors
		} else {
			x->expect_bytes-=n;
			x->recv->size+=n;
			if (!x->expect_bytes)
				x->handle_data(x);
		}
	} else if (ev->events & EPOLLOUT) {
		list_for_each_safe(i, tmp, &x->transferlist)
		{
			q = list_entry(i, struct aura_chunk_queue, list); 
			DBG("Poof: %d", q->chunk->size);
			n = write(h->fd, 
				  &q->chunk->data[x->pos], 
				  q->chunk->size - x->pos); 
			if (n == -1) {
				if (errno == EAGAIN) {
					DBG("Done for now");
					break;
				}
			} else {
				x->pos += n;
				if (q->chunk->size - x->pos == 0) {
					DBG("Chunk transfered, removing from queue");
					aura_chunk_put(q->chunk);
					list_del(&q->list);
					
					x->pos=0;
				}
				/* No more data, disable epollout for now */
				if (list_empty(&x->transferlist)) {
					DBG("All transfers done, disabling epollout");
					aura_epollout(h, 0);
				}
			}
		}	
	} else {
		DBG("Shit happens");
	}

	return 0;
}

int aura_async_enqueue_chunk(struct aura_async_xfer* x, struct aura_chunk* chunk) 
{
	int ret;
	if (list_empty(&x->transferlist)) 
	{
		ret = aura_epollout(x->h, 1); /* We need to enable the EPOLLOUT event */
		if (ret != 0) {
			return ret;
		}
	}
	struct aura_chunk_queue *q = malloc(sizeof(struct aura_chunk_queue));
	if (!q)
		return -ENOMEM;
	q->chunk = chunk;
	list_add_tail(&q->list, &x->transferlist);
	DBG("Chunk of size %d added to queue", chunk->size);
	return 0;
}

void aura_async_xfer_set_receiver(struct aura_async_xfer *x, struct aura_chunk *c) 
{
	if (x->recv)
		aura_chunk_put(x->recv);
	x->recv = c;
	x->expect_bytes = 0;
}


int aura_async_expect_bytes(struct aura_async_xfer* x, size_t n) 
{
	int ret = 0;
	int delta = x->recv->alloc - n;
	if (delta < 0) 
	{
		ret = aura_chunk_realloc(x->recv, -delta*2);
	}
	x->expect_bytes=n;
	return ret;
}

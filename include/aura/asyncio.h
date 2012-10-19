#ifndef __ASYNCIO_H
#define __ASYNCIO_H

struct aura_chunk_queue {
	struct list_head list;
	struct aura_chunk *chunk;
};

struct aura_async_xfer {
	struct list_head transferlist; /* aura_chunk_queue */
	struct aura_chunk *readbufer;
	int df;
	int expect_bytes;
};


struct aura_epoll_hook;
struct aura_async_xfer* aura_create_async_xfer(struct aura_epoll_hook* h);

/* Enqueue chunk for transmission */
int aura_async_enqueue_chunk(
	struct aura_async_xfer* x, 
	struct aura_chunk* chunk);

/* Terminate any pending transfers */
void aura_async_terminate(struct aura_async_xfer* x);

/* Expect n bytes from fd  */
int aura_async_expect_bytes(struct aura_async_xfer* x, size_t n);

int aura_async_xfer_handler(struct epoll_event* ev);

#endif

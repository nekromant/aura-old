#ifndef __ASYNCIO_H
#define __ASYNCIO_H

struct aura_chunk_queue {
	struct list_head list;
	struct aura_chunk *chunk;
};

struct aura_async_xfer {
	struct list_head transferlist; /* aura_chunk_queue */
	int pos; /* position of writer */
	int expect_bytes;	
	struct aura_chunk *recv;
	struct aura_epoll_hook *h;
	void* data; /* userdata */
	int (*handle_data)(struct aura_async_xfer* );
};


struct aura_epoll_hook;
struct aura_async_xfer* aura_create_async_xfer(struct aura_epoll_hook* h);

/* Enqueue chunk for transmission */
int aura_async_enqueue_chunk(
	struct aura_async_xfer* x, 
	struct aura_chunk* chunk);

#define aura_async_xfer_reset_receiver(x)	\
	{					\
		x->recv->size=0;		\
	}					\

/* Terminate any pending transfers */
void aura_async_terminate(struct aura_async_xfer* x);

/* Expect n bytes from fd  */
int aura_async_expect_bytes(struct aura_async_xfer* x, size_t n);

int aura_async_xfer_handler(struct epoll_event* ev);

void aura_async_xfer_set_receiver(struct aura_async_xfer *x, struct aura_chunk *c);

#endif

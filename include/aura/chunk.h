#ifndef __CHUNK_H
#define __CHUNK_H

/* a chunk of data */
struct aura_chunk {
	char* data;
	int size; /* used data bytes */
	int alloc; /* total allocated data bytes */
	int refcnt; /* reference count */
};

struct aura_chunk* aura_chunk_allocate(int sz);

/** 
 * Increments reference counter of the chunk
 *
 * @param ptr Pointer to aura_chunk
 */
#define aura_chunk_get(ptr)			\
	{					\
		ptr->refcnt++;			\
	}					\

#define aura_chunk_free(chunk)			\
	{					\
		free(chunk->data);		\
		free(chunk);			\
	}

/** 
 * decrements reference counter, frees the chunk once it's not 
 * referenced any more
 *
 * @param ptr pointer to aura_chunk
 */
#define aura_chunk_put(ptr)			\
	{					\
		ptr->refcnt--;			\
		if (0 == ptr->refcnt)		\
			aura_chunk_free(ptr);	\
	}


int aura_chunk_realloc(struct aura_chunk* chunk, int extra);

#endif

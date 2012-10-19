#include <stdio.h>
#include <stdlib.h>
#include <sys/cdefs.h>
#include <string.h>
#include <aura/chunk.h>


struct aura_chunk* aura_chunk_allocate(int sz) 
{
	struct aura_chunk* chunk = malloc(sizeof(struct aura_chunk));
	if (!chunk) 
		return NULL;
	chunk->data = malloc(sz);
	if(!chunk->data)
		goto err_alloc;
	chunk->size=0;
	chunk->alloc=sz;
	chunk->refcnt=1;
	return chunk;
err_alloc:
	free(chunk);
	return NULL;
}


int aura_chunk_realloc(struct aura_chunk* chunk, int extra) 
{
	printf("urpc: buffer to small, reallocating\n");
	printf("urpc: This can cause slowdowns \n");
	int sz = chunk->alloc+extra;
	char* data = realloc(chunk->data, sz);
	if (!data) 
		return -1;
	chunk->data = data;
	chunk->alloc = sz;
	return 0;
}

#include <stdlib.h>
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
#include <aura/asyncio.h>
#include <aura/urpc.h>
#include "uart.h"

#define DEBUG 1
#define COMPONENT "urpc-serial"
#include <aura/debug.h>

/* Notes: current serial io implementation sucks, is hacky and 
 * overcomplicated in places. 
 * It should be updated and cleaned up.
 */

struct serialinstance {
	struct uart_settings_t* us;
	struct aura_epoll_hook hook;
	struct aura_async_xfer *xfer;
	struct urpc_object * objects;
	struct urpc_instance* inst;
	int swap;
	int id_sz; /* size of id   field */
	int sz_sz; /* size of size field */
	int rxstate; /* Receiving state-machine */
        int state; /* discovery/normal op flag */
	int objcount; /* discovered object count */
	lua_State *L;
};


#define MAGIC_START '['
#define MAGIC_STOP  ']'

#define STATE_WAIT_START 0
#define STATE_GET_LEN    1
#define STATE_GET_BODY   2
#define STATE_DISCOVERY  3


#define STATE(m) (m & 0x0f) 

unsigned static int number_to_int(void* data, int n) 
{
	uint8_t* n8;
	uint16_t* n16;
	uint32_t* n32;
	switch (n){
	case 1:
		n8 = data;
		return (unsigned int) *n8;
		break;
	case 2:
		n16 = data;
		return (unsigned int) *n16;
		break;
	case 4:
		n32 = data;
		return (unsigned int) *n32;
		break;
	default:
		return 0;
	}
}

static char* next_string(char* s)
{
	while (*s++);;
	return s;
}


static unsigned char csum(unsigned char* data, size_t len) {
	unsigned char csum=0;
	while (len--)
		csum+=data[len];
	return csum;
}


void dump_chunk(struct aura_chunk* c) 
{
	int i;
	for (i=0; i < c->size; i++ ) {
		if ((i % 8) == 0)
			printf("\n");
		printf(" %hhx ", c->data[i]);
	}
	printf("\n");
}
int urpc_xfer_handler_packets(struct aura_async_xfer *x) 
{
	struct serialinstance *nl = x->data;
	int n, i;
	char* s;
	int id;
	struct urpc_object *o; 
	switch (nl->rxstate)
	{
	case STATE_WAIT_START:
		DBG("SYNC?");
		if (*x->recv->data != MAGIC_START ) {
			aura_async_xfer_reset_receiver(x);
			x->expect_bytes=1;		
			return 0;
		}
		nl->rxstate = STATE_GET_LEN;
		aura_async_expect_bytes(x,nl->sz_sz);
		return 0;
	case STATE_GET_LEN:
		nl->rxstate = STATE_GET_BODY;
		n = number_to_int(&x->recv->data[1],nl->sz_sz);
		DBG("Expecting %d bytes in packet body", n);
		aura_async_expect_bytes(x, n);
		break;
	case STATE_GET_BODY:	
                /* TODO: check csum */	
		if (nl->state) {
			/* Events and responses */
			nl->state++;
			goto next_packet; /* Last one */
		} else
		{
			/* All the discovery stuff */
			n = number_to_int(&x->recv->data[1], nl->sz_sz);
			if (n == 1) {
				nl->state++;
				goto next_packet; /* Last one */
			}
			s = &x->recv->data[ 1 + nl->sz_sz ];
			o = malloc(sizeof(struct urpc_object));
			o->name = NULL;
			o->args = NULL;
			o->reply = NULL;
			o->flags = *s++;
			id = number_to_int(s, 1);
			*s++;
			DBG("ID# %d", id);
			if (strlen(s)) o->name = strdup(s);
			s = next_string(s);
			if (strlen(s)) o->args = strdup(s);
			s = next_string(s);
			if (strlen(s)) o->reply = strdup(s);
			list_add_tail(&o->list, &nl->inst->objlist);
			nl->objcount++;
			DBG("name: %s",  o->name);
			DBG("args: %s",  o->args);
			DBG("reply: %s", o->reply);
		}
	next_packet:
		aura_async_xfer_reset_receiver(x);
		x->expect_bytes=1;
		nl->rxstate = STATE_WAIT_START;
		/* Now, let's parse response */
		
	}
	return NULL;
	
}

int urpc_xfer_handler_discovery_init(struct aura_async_xfer *x) 
{
	DBG("Handling initial discovery data");
	struct serialinstance *nl = x->data;
	char *stag, *itag, *etag;
	switch (nl->rxstate)
	{
	case STATE_WAIT_START:
		if (*x->recv->data != MAGIC_START ) {
			aura_async_xfer_reset_receiver(x);
			x->expect_bytes=1;		
			return 0;
		}
		nl->rxstate = STATE_DISCOVERY;
		aura_async_expect_bytes(x, 5);
		break;
	case STATE_DISCOVERY:
		stag = &x->recv->data[1];
		//DBG("Unit feature packet: %s", stag);
		itag = &x->recv->data[2];
		etag = &x->recv->data[3];
		nl->sz_sz = (int) (stag[0]);
		nl->id_sz = (int) (itag[0]);
		nl->swap = 0;
		if ((host_is_little_endian()) && (etag[0] == 'b'))
			nl->swap = 1;
		if ((!host_is_little_endian()) && (etag[0] == 'l'))
			nl->swap = 1;
		DBG("IDs are %d byte(s) each", nl->id_sz);
		DBG("SIZEs are %d byte(s) each", nl->sz_sz);
		DBG("Endianness swapping is %s needed", nl->swap ? "" : "NOT");
		aura_async_xfer_reset_receiver(x);
		x->handle_data = urpc_xfer_handler_packets;
		nl->rxstate = STATE_WAIT_START;
		aura_async_xfer_reset_receiver(x);
		x->expect_bytes=1;	
		break;
	}	 
return 0;
}

static void* urpc_serial_open(lua_State* L)
{
	int argc = lua_gettop(L);
	if (argc<2) {
		printf("urpc-serial: need another arg\n");
		return 0;
	}
	const char *settings = lua_tostring(L,2);
	struct serialinstance* nl = malloc(sizeof(struct serialinstance));
	nl->L = L;
	if (!nl)
		return 0;
	printf("urpc-serial: opening serial: %s\n", settings);
	nl->us = urpcserial_make_settings(settings);
	if (!nl->us)
		goto error_parse;
	if (0 > urpcserial_uart_init(nl->us))
		goto error_init;
	nl->hook.name = settings;
	//nl->hook.data = nl;
	nl->hook.fd = nl->us->fd;
	nl->hook.ev.events = EPOLLIN; 
	nl->xfer = aura_create_async_xfer(&nl->hook);
	nl->xfer->data = nl;
	if (!nl->xfer)
		goto error_init;
	struct aura_chunk *rxb = aura_chunk_allocate(256);
	aura_async_xfer_set_receiver(nl->xfer, rxb);
	return nl;
error_init:
	free(nl->us);
error_parse:
	free(nl);
	return 0;
}

/* FIXME: Only 1-byte sizes and ids are supported */
char reply[64] = "(none)" ;
static int urpc_serial_call(lua_State* L, struct  urpc_instance* inst, int id)
{
	DBG("Running a call to id #%d\n", id);	
	struct serialinstance *s = URPC_INSTANCE_PRIVATE(inst);
	int offset = 1 + s->sz_sz + s->id_sz;
	struct aura_chunk *chunk = urpc_pack_data(L, inst, 256, offset, 
						  inst->objects[id]->acache, 0);
	if (!chunk)
		return 0;
	chunk->data[0]=MAGIC_START;
	chunk->data[1]=chunk->size-1;
	chunk->data[2]=id;
	chunk->data[chunk->size] = csum(&chunk->data[1],chunk->size-1);
	chunk->size++;
	chunk->data[chunk->size++] = MAGIC_STOP;
	dump_chunk(chunk);
	aura_async_enqueue_chunk(s->xfer, chunk);
	
	/* If we need a reply, loop  until the call is complete */
	if (inst->objects[id]->reply) {
		DBG("Waiting for reply");
		s->state = 0;
		while ((s->state==0))
			aura_loop_once(L);
	}

	return 0;
}


/* Should return the count of discovered objects and set the head
 * to the very first in the linked list 
 */


const char discovery[] = { '[', 0x0, 0x0, ']' };  
static int urpc_serial_discovery(lua_State* L, struct urpc_instance* inst)
{	
	struct serialinstance *s = URPC_INSTANCE_PRIVATE(inst);
	struct aura_chunk *c = aura_chunk_allocate(ARRAY_SIZE(discovery));
	memcpy(c->data, discovery, ARRAY_SIZE(discovery));
	c->size=ARRAY_SIZE(discovery);
	s->inst=inst;
	s->objcount=0;
	aura_async_enqueue_chunk(s->xfer, c);
	aura_async_expect_bytes(s->xfer, 1);
	s->rxstate = 0;
	s->xfer->handle_data = urpc_xfer_handler_discovery_init;
	int loops = 1000;
	s->rxstate = STATE_WAIT_START;
	s->state=0;
	/* Loop until discovery is complete */
	while ((s->state==0))
		aura_loop_once(L);
	
	return s->objcount;
}


static struct urpc_transport ntrans = {
	.name = "serial",
	.open = urpc_serial_open,
	.call= urpc_serial_call,
	.discovery = urpc_serial_discovery,
};

int aura_plugin_init(lua_State* L) 
{
	urpc_register_transport(L,&ntrans);
	printf("urpc-serial: uRPC 'serial' transport\n");
	lua_pushnumber(L,1);
	return 1;
}

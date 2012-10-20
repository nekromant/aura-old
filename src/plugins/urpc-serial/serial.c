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
};


#define MAGIC_START '['
#define STATE_WAIT_START 0
#define STATE_GET_LEN    1
#define STATE_GET_BODY   2
#define STATE_DISCOVERY  3


#define STATE(m) (m & 0x0f) 

unsigned int number_to_int(void* data, int n) 
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

int urpc_xfer_handler_packets(struct aura_async_xfer *x) {
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
		aura_async_expect_bytes(x, n+1);
		break;
	case STATE_GET_BODY:		
		if (nl->state) {
			/* Events and responses */
		} else
		{
			/* All the discovery stuff */
			/* TODO: check csum */
			DBG("data @ %d", 1 + nl->sz_sz );
			n = number_to_int(&x->recv->data[1], nl->sz_sz);
			if (n == 1) {
				nl->rxstate++;
				goto next_packet; /* Last one */
			}
			s = &x->recv->data[ 1 + nl->sz_sz ];
#if 0			
			for (i=0; i < x->recv->size; i++ ) {
				DBG(" %hhx | %c ", s[i], s[i]);
			}
#endif
			o = malloc(sizeof(struct urpc_object));
			o->flags = *s++;
			id = number_to_int(s, 1);
			*s++;
			DBG("ID# %d", id);
			o->name = strdup(s);
			s = next_string(s);
			o->args = strdup(s);
			s = next_string(s);
			o->reply = strdup(s);
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

char reply[64] = "(none)" ;
static int urpc_serial_call(lua_State* L, struct  urpc_instance* inst, int id)
{
	DBG("Running a call to id #%d\n", id);	
/*
  
  struct urpc_chunk *chunk = urpc_pack_data(L, inst, 256, 0, 
  inst->objects[id]->acache, 0);
  int i;
  if (!chunk) {
  printf("WTF?\n");
  return 0;
  }
  gethostname(reply, 64);
  struct nullinstance *prv = URPC_INSTANCE_PRIVATE(inst);
  printf("\nurpc-nullt: %s", prv->tag);
  for (i=0; i<chunk->size; i++) {
  if ((i % 8) == 0)
  printf("\nurpc-nullt: ");
  printf(" 0x%2hhx ", chunk->data[i]);
  }
  urpc_chunk_free(chunk);
  printf("\nurpc-nullt: ----------\n");
  if (inst->objects[id]->reply){
  int n = urpc_unpack_data(L, reply,
  inst->objects[id]->rcache, 0);
  return n;
		
  }
*/
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

	int loops = 100;
	s->rxstate = STATE_WAIT_START;
	s->state=0;
	/* Do up to 100 loops and wait for discovery to complete */
	while (loops-- && (s->state==0))
	{
		aura_loop_once(L);
	}      
	return 0;
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

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
#include <aura/urpc.h>
#include "uart.h"

struct serialinstance {
	struct uart_settings_t* us;
	struct aura_epoll_hook hook;
	int swap;
};



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
	nl->hook.data = nl;
	nl->hook.fd = nl->us->fd;
	nl->hook.ev.events = EPOLLIN; 
	//aura_add_epollhook(&nl->hook);
	aura_init_loop();
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
/*
	printf("urpc-nullt: Running a call to id #%d and dumping packed data\n", id);
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

static int urpc_serial_discovery(lua_State* L, struct urpc_instance* inst)
{	
//	inst->head = &nullobjs2;
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

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
#include <azra/azra.h>
#include <azra/urpc.h>



struct nullinstance {
	const char* tag;
};

static void* urpc_null_open(lua_State* L)
{
	printf("urpc-nullt: Opened a null transport\n");
	int argc = lua_gettop(L);
	if (argc<2) {
		printf("urpc-nullt: need another arg\n");
		return 0;
	}
	const char *tag = lua_tostring(L,2);
	printf("urpc-nullt: instance with tag '%s'\n", tag);
	struct nullinstance* nl = malloc(sizeof(struct nullinstance));
	nl->tag=tag;
	return nl;
}


char reply[64] = "(none)" ;
static int urpc_null_call(lua_State* L, struct  urpc_instance* inst, int id)
{	
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
	return 0;
}

static struct urpc_object sobj = {
	.flags = FLAG_METHOD,
	.name = "hostname",
	.reply ="s;"
};

static struct urpc_object nullobjs = {
	.flags = FLAG_METHOD,
	.name = "packnumbers",
	.args = "1d;2d;1u;2u;",
	.next = &sobj
};

static struct urpc_object nullobjs2 = {
	.flags = FLAG_METHOD,
	.name = "unpacknumbers",
	.reply = "1d;2d;1u;2u;",
	.next = &nullobjs
};


/* Should return the count of discovered objects and set the head
 * to the very first in the linked list 
 */

static int urpc_null_discovery(lua_State* L, struct urpc_instance* inst)
{	
	inst->head = &nullobjs2;
	return 3;
}


static struct urpc_transport ntrans = {
	.name = "null",
	.open = urpc_null_open,
	.call= urpc_null_call,
	.discovery = urpc_null_discovery,
};

int azra_plugin_init(lua_State* L) 
{
	urpc_register_transport(L,&ntrans);
	printf("urpc-null: uRPC 'NULL' transport\n");
	lua_pushnumber(L,1);
	return 1;
}

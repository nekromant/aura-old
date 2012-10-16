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
	char* tag;
};

static void* urpc_null_open(lua_State* L)
{
	printf("urpc-nullt: Opened a null transport\n");
	int argc = lua_gettop(L);
	if (argc<2) {
		printf("urpc-nullt: need another arg\n");
		return 0;
	}
	char *tag = lua_tostring(L,2);
	printf("urpc-nullt: instance with tag '%s'\n", tag);
	struct nullinstance* nl = malloc(sizeof(struct nullinstance));
	nl->tag=tag;
	return nl;
}

static int urpc_null_call(lua_State* L)
{	
	
}

static int urpc_null_discovery(lua_State* L)
{	
	return 0;
}


static struct urpc_transport ntrans = {
	.name = "nullt",
	.open = urpc_null_open,
	.call= urpc_null_call,
	.discovery = urpc_null_discovery,
};

int azra_plugin_init(lua_State* L) 
{
	printf("urpc-null: uRPC 'NULL' transport\n");
	urpc_register_transport(L,&ntrans);
	lua_pushnumber(L,1);
	return 1;
}

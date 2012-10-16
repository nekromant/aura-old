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

int host_is_little_endian()
{
	short int t = 1;
	char* nptr = (char*) &t;
	return (nptr[0] == 1);
}

/* transport, instance, rest of args follow */
int l_urpc_call(lua_State *L) 
{
	struct urpc_instance *i = lua_touserdata(L,1);
	i->transport->call(L,i);
}

int l_urpc_discovery(lua_State *L) 
{
	struct urpc_instance *i = lua_touserdata(L,1);
	printf("urpc: Running discovery via '%s' transport\n", i->transport->name);
	return i->transport->discovery(L,0);
}


/* Arguments. First an urpc_transport, then go options to the backend */
/* Open should return a userdata pointer to instance or nil*/
int l_urpc_open(lua_State *L) 
{
	struct urpc_instance* inst;
	struct urpc_transport *t = lua_touserdata(L,1);
	printf("urpc: Opening '%s' transport\n", t->name);
	void* private_data =  t->open(L);
	if (private_data) { 
		inst = malloc(sizeof(struct urpc_instance));
		inst->private_data = private_data;
		inst->transport=t;
		lua_pushlightuserdata(L,inst);
		return 1;
	}
	return 0;
}


void urpc_register_transport(lua_State* L, struct urpc_transport* t) 
{
	printf("urpc: Registering new urpc transport: %s\n", t->name);
	lua_getglobal(L,"__urpc_transports");
	lua_pushstring(L, t->name);
	lua_pushlightuserdata(L, t);
	lua_settable(L, -3);
}

int azra_plugin_init(lua_State *L) 
{
	lua_newtable(L);
	lua_setglobal(L,"__urpc_transports");
	printf("urpc: urpc framework plugin 0.1\n");
	printf("urpc: running on a %s-endian machine\n", 
	       host_is_little_endian() ? "little" : "big");
	lua_pushnumber(L,1);
	lua_register(L,"__urpc_open", l_urpc_open);
	lua_register(L,"__urpc_discovery", l_urpc_discovery);
	lua_register(L,"__urpc_call", l_urpc_call);

	return 1;
}

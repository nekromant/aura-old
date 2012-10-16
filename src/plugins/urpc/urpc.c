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

void lua_pushtablestring(lua_State* L , char* key , char* value) {
	lua_pushstring(L, key);
	lua_pushstring(L, value);
	lua_settable(L, -3);
} 

void lua_pushtablebool(lua_State* L , char* key , int value) {
	lua_pushstring(L, key);
	lua_pushboolean(L, value);
	lua_settable(L, -3);
} 


/* instance, id, rest of args follow */
static int l_urpc_call(lua_State *L) 
{
	struct urpc_instance *i = lua_touserdata(L, 1);
	int id = lua_tonumber(L, 2);	
	return i->transport->call(L, i, id);
}

static int l_urpc_discovery(lua_State *L) 
{
	struct urpc_instance *i = lua_touserdata(L,1);
	printf("urpc: Running discovery via '%s' transport\n", i->transport->name);
	int n =  i->transport->discovery(L,i);
	printf("urpc: Generating object cache\n", i->transport->name);
	i->objects = malloc(sizeof(void*)*n);
	int j=0;
	struct urpc_object* h = i->head;
	lua_newtable(L);
	while (h) {
		i->objects[j++]=h;
		lua_pushnumber(L,j);
		lua_newtable(L);
		lua_pushtablebool(L,"is_method", OBJECT_IS_METHOD(h));
		lua_pushtablestring(L,"name",h->name);
		lua_pushtablestring(L,"args",h->args);
		lua_pushtablestring(L,"reply",h->reply);
		lua_settable(L, -3);
		h=h->next;
	}
	return 1;
}

/* Arguments. First an urpc_transport, then go options to the backend */
/* Open should return a userdata pointer to instance or nil*/
static int l_urpc_open(lua_State *L) 
{
	struct urpc_instance* inst;
	struct urpc_transport *t = lua_touserdata(L,1);
	printf("urpc: Opening '%s' transport\n", t->name);
	void* private_data =  t->open(L);
	if (private_data) { 
		inst = malloc(sizeof(struct urpc_instance));
		inst->private_data = private_data;
		inst->transport=t;
		inst->objects=NULL;
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

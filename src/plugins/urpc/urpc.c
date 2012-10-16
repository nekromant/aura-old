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

int is_little_endian()
{
	short int t = 1;
	char* nptr = (char*) &t;
	return (nptr[0] == 1);
}

int l_urpc_call(lua_State *L) 
{

}

/* Arguments. First an urpc object, then go options to the backend */
int l_urpc_open(lua_State *L) 
{
	
}

void urpc_register_transport(struct urpc_transport* t) 
{
	printf("urpc: Registering new urpc transport: %s\n", t->name);
}

int azra_plugin_init(lua_State *L) 
{
	printf("urpc: urpc framework plugin 0.1\n");
	printf("urpc: running on a %s-endian machine\n", 
	       is_little_endian() ? "little" : "big");
	lua_pushnumber(L,1);
	return 1;
}

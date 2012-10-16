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

int azra_plugin_init(lua_State* L) 
{
	printf("dummy: Hello, world!\n");
	lua_pushnumber(L,1);
	return 1;
}

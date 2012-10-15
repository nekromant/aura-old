#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <termios.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/cdefs.h>
#include <stdarg.h>
#include <stdio.h>
#include <lua.h>
#include <errno.h>
#include <lauxlib.h>
#include <string.h>
#include <azra/azra.h>
#include "config.h"



static void handle_runaway(lua_State *L, lua_Debug *ar)
{
	luaL_error(L,"Process is taking up too much time - aborted!");
}

static int count;

int azra_setup_vm_protection(lua_State *L, int cnt)
{
	count = cnt;
	lua_sethook (L, handle_runaway, LUA_MASKCOUNT, count);
	return 0;
}

static int Lsetprot(lua_State *L)
{
	int n = lua_gettop(L); 
	if (n!=1)
	{
		luaL_error(L,"Invalid number of arguments!");
		return 1;
	}
	if (!lua_isnumber(L, 1)) {
            luaL_error(L, "Argument must be a number");
            return 1;
	}
	n = lua_tonumber(L, count);
	return azra_setup_vm_protection(L, n);
}

static int Lgetprot(lua_State *L)
{
	int n = lua_gettop(L); 
	if (n!=0)
	{
		luaL_error(L,"Invalid number of arguments!");
		return 1;
	}
	lua_pushnumber(L, count);
	return 1;
}

static struct azra_hook hooks[] = {
{
    .func = Lsetprot,
    .name = "azra_setvmprot",
	.args = "( n )",
    .help = "Setup runaway protection for n VM instructions"
}, {
    .func = Lgetprot,
    .name = "azra_getvmprot",
	.args = "()",
    .help = "Get the number of allowed by protector instructions"
}};


int azra_protector_init(lua_State *L)
{
	azra_setup_vm_protection(L, 90000000);
	azra_func_reg_list(L, hooks, 2);
	return 0;	
}

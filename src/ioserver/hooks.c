#include <sys/cdefs.h>

#include <stdio.h>
#include "lua.h"
#include "lauxlib.h"
#include <string.h>
#include <termios.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <sys/cdefs.h>
#include <azra/azra.h>

static struct azra_hook* base;
static struct azra_hook* end;

/* Pushes a list of registered core functions onto the stack */
static int l_azra_help(lua_State *L)
{
	struct azra_hook* h = base;
	int i=1;
	lua_newtable(L);
	do 
	{

		lua_pushnumber(L,i);	
		lua_newtable(L);

		lua_pushnumber(L,1);
		lua_pushstring(L,h->name);
		lua_settable(L,-3);

		lua_pushnumber(L,2);
		lua_pushstring(L,h->args);
		lua_settable(L,-3);

		lua_pushnumber(L,3);
		lua_pushstring(L,h->help);
		lua_settable(L,-3);

		lua_settable(L,-3);

		h=h->next;
		i++;

	} while (h);

	return 1;
}



void azra_func_reg(lua_State* L, struct azra_hook* hook)
{
	struct azra_hook* h = end;
	h->next = hook;
	h=h->next;
	h->next=0;
	lua_register(L,h->name,h->func);
	end=h;
}

void azra_func_reg_list(lua_State* L, struct azra_hook* hook, int count) {
	int i; 
	for (i=0; i<count; i++)
		azra_func_reg(L,&hook[i]);
}


static struct azra_hook azra_helph = 
{
	.func = l_azra_help,
	.name = "azra_hooks",
	.help = "Get a list of exported C hooks"
};


void azra_func_init(lua_State* L)
{
	printf("azra: Intializing lua subsystem\n");
	base = &azra_helph;
	end = base;
	lua_register(L,base->name,base->func);
	printf("azra: subsystem ready\n");
}

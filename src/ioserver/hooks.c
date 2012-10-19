#include <sys/cdefs.h>
#include <stdlib.h>
#include <stdio.h>
#include "lua.h"
#include "lauxlib.h"
#include <string.h>
#include <termios.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <sys/cdefs.h>
#include <aura/aura.h>

static struct aura_hook* base;
static struct aura_hook* end;

/* Pushes a list of registered core functions onto the stack */
static int l_aura_help(lua_State *L)
{
	struct aura_hook* h = base;
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



void aura_func_reg(lua_State* L, struct aura_hook* hook)
{
	struct aura_hook* h = end;
	h->next = hook;
	h=h->next;
	h->next=0;
	lua_register(L,h->name,h->func);
	end=h;
}

void aura_func_reg_list(lua_State* L, struct aura_hook* hook, int count) {
	int i; 
	for (i=0; i<count; i++)
		aura_func_reg(L,&hook[i]);
}


static struct aura_hook aura_helph = 
{
	.func = l_aura_help,
	.name = "aura_hooks",
	.help = "Get a list of exported C hooks"
};


void aura_func_init(lua_State* L)
{
	printf("aura: Intializing lua subsystem\n");
	base = &aura_helph;
	end = base;
	lua_register(L,base->name,base->func);
	printf("aura: subsystem ready\n");
}

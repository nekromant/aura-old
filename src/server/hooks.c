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
static int azra_help(lua_State *L)
{
  struct azra_hook* h = base;
  int i=0;
  do 
  {
    lua_pushfstring(L, "%s - %s", h->name, h->help);
    h=h->next;
    i++;
  } while (h);
  return i;
}


void azra_register_hook(lua_State* L, struct azra_hook* hook)
{
  struct azra_hook* h = end;
  h->next = hook;
  h=h->next;
  h->next=0;
  lua_register(L,h->name,h->func);
  end=h;
}

static struct azra_hook azra_helph = 
{
  .func = azra_help,
  .name = "azra_hooks",
  .help = "Get a list of exported C hooks"
};

void azra_hooklist_init(lua_State* L)
{
  printf("azra: Intializing lua subsystem\n");
  base = &azra_helph;
  end = base;
  lua_register(L,base->name,base->func);
  printf("azra: subsystem ready\n");
}

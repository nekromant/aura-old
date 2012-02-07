#ifndef _AZRA_H
#define _AZRA_H

struct azra_hook
{
  lua_CFunction func;
  char* name;
  char* help;
  void* next;
};

void azra_register_hook(lua_State* L, struct azra_hook* hook);
void azra_hooklist_init(lua_State* L);
int network_init(lua_State* l, char* host, int portno);


#endif
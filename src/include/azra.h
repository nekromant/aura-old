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
struct uart_settings_t
{
 tcflag_t ifl;
 tcflag_t cfl;
 tcflag_t ofl;
 char * port;
 char* tag;
 int fd;
};

struct uart_settings_t* str_to_uart_settings();

#endif
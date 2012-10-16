#ifndef __URPC_H
#define __URPC_H

struct urpc_transport {
	char* name;
	int (*urpc_open)(lua_State* L);
	int (*urpc_call)(lua_State* L);
	int (*urpc_discovery)(lua_State* L);
};
	
#endif

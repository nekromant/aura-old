#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <termios.h>
#include <dlfcn.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <azra/azra.h>


int l_plugin_load(lua_State* L) 
{
	char* error;
	int argc = lua_gettop(L);
	if (argc != 1)
		return 0;
	const char* plugin = lua_tostring(L,1);
	if (0 != access(plugin, R_OK)) {
		//printf("azra: Can't access plugin: %s\n", plugin);
		return 0;
	}
	void* handle = dlopen(plugin, RTLD_LAZY|RTLD_GLOBAL);
	if (!handle) {
		printf("azra: Failed to load plugin: %s\n", dlerror());
		return 0;
	}
	dlerror();
	int (*plugin_init)(lua_State* L) = dlsym(handle, "azra_plugin_init");
	if ((error = dlerror()) != NULL)  {
		fprintf(stderr, "%s\n", error);
		return 0;
	}
	/* TODO: May be push handle as userdata to lua plugin array */
	return plugin_init(L);
}


static struct azra_hook azra_pload = 
{
	.func = l_plugin_load,
	.name = "do_azra_load_plugin",
	.help = "Load a binary plugin",
	.args = "path"
};


void azra_pluginloader_init(lua_State* L) 
{
	/* Make symbols from core avaliable for libs */
	dlopen(0 , RTLD_NOW | RTLD_GLOBAL);
	azra_func_reg(L,&azra_pload);
}

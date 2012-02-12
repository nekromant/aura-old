#include <sys/cdefs.h>

#include <stdio.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <sys/cdefs.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <termios.h>

#include <string.h>
#include <azra/azra.h>


/* TODO: Set config from cmd line */
int Lgetconf(lua_State* L)
{
	printf("!!!\n");
    lua_pushstring(L,"config.lua");
    return 1;
}

static struct azra_hook getconf = {
    .func = Lgetconf,
    .name = "azra_getconf",
    .help = "Get config file name"
};


static int call_me_from_lua (lua_State *L) {
    printf("QWERTYUIOL~\n");
    int n = lua_gettop(L);    /* number of arguments */
    lua_Number sum = 0;
    int i;
    for (i = 1; i <= n; i++) {
        if (!lua_isnumber(L, i)) {
            lua_pushstring(L, "incorrect argument");
            lua_error(L);
        }
        sum += lua_tonumber(L, i);
    }
    lua_pushnumber(L, sum/n);        /* first result */
    lua_pushnumber(L, sum);         /* second result */
    return 2;                   /* number of results */
}

static struct azra_hook testhook = {
    .func = call_me_from_lua,
    .name = "runme",
    .help = "Test function"
};


/*
 *  char buff[1024];
     int error;
     strcpy(buff,"dofile(\"lua/init.lua\")\n");
    do {
        error = luaL_loadbuffer(l, buff, strlen(buff), "line") ||
                lua_pcall(l, 0, 0, 0);
        if (error) {
          fprintf(stderr, "%s\n", lua_tostring(l, -1));
          lua_pop(l, 1);
        }
        printf("azra# ");
        fflush(stdout);
      } while (fgets(buff, sizeof(buff), stdin) != NULL);
 */

static char initstr[] = "dofile(\"lua/init.lua\")\n";

int main(argc, argv)
int argc;
char **argv;
{
    lua_State *l = luaL_newstate();
    luaL_openlibs(l);
    azra_hooklist_init(l);

    azra_register_hook(l,&testhook);
    azra_register_hook(l,&getconf);

    int error = luaL_loadbuffer(l, initstr, strlen(initstr), "line") ||
            lua_pcall(l, 0, 0, 0);
    if (error) {
        fprintf(stderr, "%s\n", lua_tostring(l, -1));
        lua_pop(l, 1);
    }
    //TODO: Argument parsing
    //Lua scripts directory
    azra_broadcaster_init(stdout);
	azra_init_loop();
	azra_server_init(l,"0.0.0.0",8888);
	azra_main_loop();
	return 0;

}

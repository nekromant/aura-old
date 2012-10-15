#include <sys/cdefs.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <sys/cdefs.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <termios.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <string.h>
#include <azra/azra.h>

static int azra_check_lua_err(lua_State* L, int status)
{
	if (status && !lua_isnil(L, -1))
	{
		const char* msg = lua_tostring(L, -1);
		if (!msg)
			msg = "(error object is not a string)";
 		fprintf(stderr, "azra: %s\n", msg);
		lua_pop(L, 1);		
	}
	
	return status;
}


void interactive_loop(lua_State* L) {

	char* input, shell_prompt[100];
	for(;;)
	{
		// getting the current user 'n path
		snprintf(shell_prompt, sizeof(shell_prompt), "azra # " );
		// inputing...
		input = readline(shell_prompt);
		// eof
		if (input==NULL)
			break;
		// path autocompletion when tabulation hit
		rl_bind_key('\t', rl_complete);
		// adding the previous input into history
		add_history(input);
		int error = luaL_loadbuffer(L, input, strlen(input), "shell") ||
			lua_pcall(L, 0, 0, 0);
		azra_check_lua_err(L,error);
		/* do stuff */
 
		// Т.к. вызов readline() выделяет память, но не освобождает(а возвращает), то эту память нужно вернуть(освободить).
		free(input);
	}
}


#define LUAPATH "./lua/"
#define CONFIGFILE "./azra.conf.example"

int main(int argc, char **argv)
{
	//This is the control state
	lua_State *L = luaL_newstate();
	if (argc<2)
	{
		printf("Azra IO server\n");
		printf("USAGE: %s\n",argv[0]);
		printf("(c) -- Necromant 2011-2012 --\n");
		exit(1);
	}
	luaL_openlibs(L);
	azra_func_init(L);
	azra_protector_init(L);
	azra_pluginloader_init(L);
	lua_pushstring(L,CONFIGFILE);
	lua_setglobal(L,"configfile");
	
	int s = luaL_loadfile( L, LUAPATH "init.lua");
	if (0 == s) 
		s = lua_pcall(L, 0, LUA_MULTRET, 0);
        azra_check_lua_err(L,s);
	interactive_loop(L);	
	
	//Basically just loop here
	return azra_main_loop();

}

#include <sys/cdefs.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <sys/cdefs.h>
#include <lua.h>
#include <getopt.h>
#include <lualib.h>
#include <lauxlib.h>
#include <termios.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <string.h>
#include <aura/aura.h>

static int aura_check_lua_err(lua_State* L, int status)
{
	if (status && !lua_isnil(L, -1))
	{
		const char* msg = lua_tostring(L, -1);
		if (!msg)
			msg = "(error object is not a string)";
 		fprintf(stderr, "aura: %s\n", msg);
		lua_pop(L, 1);		
	}
	
	return status;
}


void interactive_loop(lua_State* L) {

	char* input, shell_prompt[100];
	for(;;)
	{
		// getting the current user 'n path
		snprintf(shell_prompt, sizeof(shell_prompt), "aura # " );
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
		aura_check_lua_err(L,error);
		/* do stuff */
 
		// Т.к. вызов readline() выделяет память, но не освобождает(а возвращает), то эту память нужно вернуть(освободить).
		free(input);
	}
}


#define LUAPATH "./lua/"
#define CONFIGFILE "./aura.conf.example"

static const char opt_str[] = "dh?if:";
static const struct option lopts[] = {
	{ "daemonize", no_argument, NULL, 'd' },
	{ "file", required_argument, NULL, 'f' },
	{ "interactive", no_argument, NULL, 'i' },
	{ "help", no_argument, NULL, 'h' },
	{ NULL, no_argument, NULL, 0 },
};


struct run_options  {
	int daemonize;
	int interactive;
	char* luapath; 
	char* configfile;
};

void run_lua_script(lua_State *L, char* name) 
{
	int s = luaL_loadfile( L, name);
	if (0 == s) 
		s = lua_pcall(L, 0, LUA_MULTRET, 0);
        aura_check_lua_err(L, s);
}

int main(int argc, char **argv)
{
	int lindex, opt;
	struct run_options* opts = calloc(1,sizeof(struct run_options));
	opts->luapath = LUAPATH;
	opts->configfile = CONFIGFILE;

	lua_State *L = luaL_newstate();
	if (argc<2)
	{
		printf("Azra IO server\n");
		printf("USAGE: %s\n",argv[0]);
		printf("(c) -- Necromant 2011-2012 --\n");
		exit(1);
	}
	luaL_openlibs(L);
	aura_func_init(L);
	char* file=NULL;
	aura_protector_init(L);
	aura_pluginloader_init(L);

	while (1) {
		opt=getopt_long(argc, argv, opt_str, lopts, &lindex);
		if (-1 == opt)
			break;
		switch (opt) {
		case 'd':
			opts->daemonize = 1;
			break;
		case 'i':
			opts->interactive = 1;
			printf("Interactive...\n");
			break;
		case 'f':
			printf("file: %s\n", optarg);
			file = optarg;
			break;
		case '?':
		case 'h':
			printf("Help me!\n");
			exit(1);
			break;
		}
	}
	
	lua_pushstring(L, CONFIGFILE);
	lua_setglobal(L, "configfile");
	run_lua_script(L, LUAPATH "init.lua");
	if (file)
		run_lua_script(L, file);

	if (opts->interactive)
		interactive_loop(L);
	if (opts->daemonize)
		printf("Daemonizing...\n");
	
	aura_loop_forever(L);
	return 0;
}

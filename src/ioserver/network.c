#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <termios.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/cdefs.h>
#include <stdio.h>
#include <lua.h>
#include <lauxlib.h>
#include <string.h>
#include <aura/aura.h>

/* IO hack. This replaces the io library's idea of stdin/out/err
 * with 3 new FILE* handles, without altering the C stdin/out/err
 * values.
 *
 * It's a hugely ugly hack, fiddling with the internals of the io
 * library. Tested with 5.1.2, may or may not work with other
 * versions...
 */
static void hackio(lua_State *L, FILE *in, FILE *out, FILE *err) {
	printf("aura: Hi-jacking lua io streams...\n");
	FILE **pf;
	lua_getglobal(L, "io"); /* Get the IO library */
	lua_pushstring(L, "open");
	lua_gettable(L, -2); /* io, io.open */
	lua_getfenv(L, -1); /* io, io.open, io.open's env */
	lua_getglobal(L, "io"); /* io, open, env, io */
	
	lua_pushstring(L, "stdin");
	lua_gettable(L, -2); /* io, open, env, io, io.stdin */
	pf = (FILE **)lua_touserdata(L, -1);
	*pf = in;
	lua_rawseti(L, -3, 1); /* IO_INPUT = 1 */
	
	lua_pushstring(L, "stdout");
	lua_gettable(L, -2); /* io, open, env, io, io.stdout */
	pf = (FILE **)lua_touserdata(L, -1);
	*pf = out;
	lua_rawseti(L, -3, 2); /* IO_OUTPUT = 2 */
	
	lua_pushstring(L, "stderr");
	lua_gettable(L, -2); /* io, open, env, io, io.stderr */
	pf = (FILE **)lua_touserdata(L, -1);
	*pf = err;
	lua_pop(L, 5);
}

void error(const char *msg)
{
	perror(msg);
	exit(1);
}


static int handle_server_io(struct epoll_event *ev)
{
	struct aura_epoll_hook *shook = (struct aura_epoll_hook*) ev->data.ptr;
 	struct aura_server_data *sdata = shook->data;
	printf("aura: accepting connection to '%s'\n", shook->name);
 	struct aura_epoll_hook *sh = malloc(sizeof(struct aura_epoll_hook));
	struct aura_client_data *cdata = calloc(1,sizeof(struct aura_client_data));
	cdata->clilen = sizeof(struct sockaddr_in);
 	sh->fd = accept(shook->fd, (struct sockaddr *) &cdata->cli_addr, &cdata->clilen);
 	if (sh->fd < 0)
	{
		perror("aura: Failed to accept incoming connection\n");
		goto accept_fail;
	}
	char *ip = inet_ntoa(cdata->cli_addr.sin_addr);
	char* name = malloc(128);
	sprintf(name,"client:%s:%hu",ip,cdata->cli_addr.sin_port);
	sdata->client_count++;
	cdata->server = sdata;
	sh->name = name;
	sh->data = cdata;
	cdata->h = sh;
	
	return aura_setup_client(sh);
	//TODO: Proper error handling
	//
accept_fail:
	free(sh);
	return 1;
}



int aura_server_init(lua_State *L, char *host, int portno)
{
	int i,err;
	struct aura_epoll_hook *shook = malloc(sizeof(struct aura_epoll_hook));
	struct aura_server_data *sdata = malloc(sizeof(struct aura_server_data));
	char* name = malloc(128);
	sprintf(name,"server %s:%d",host,portno);
	shook->name = name;
	sdata->client_count=0;
	shook->io_handler = handle_server_io;
	shook->ev.events = EPOLLIN;
	if ((!sdata) || (!shook))
	{
		printf("Out of memory!\n");
	}
	shook->fd = socket(AF_INET, SOCK_STREAM, 0);
	if (shook->fd < 0)
		error("ERROR opening socket");
	bzero((char *) &sdata->serv_addr, sizeof(struct sockaddr_in));
	sdata->serv_addr.sin_family = AF_INET;
	sdata->serv_addr.sin_addr.s_addr = INADDR_ANY;
	shook->data=sdata;
	sdata->L=L;
	sdata->firstlogin=0;
	i=5;
	while (i--)
	{
		printf("aura: using port %d\n",portno);
		sdata->serv_addr.sin_port = htons(portno);
		err = bind(shook->fd, (struct sockaddr *) &sdata->serv_addr,
			sizeof(struct sockaddr));
		if (0 == err) break;
		perror("ERROR on binding ");
		portno++;
	}
	//TODO: Proper error handling and memory mgr
	if (err!=0) return err;
	listen(shook->fd,5);
	aura_make_fd_nonblock(shook->fd);
	aura_add_epollhook(shook);
	//TODO: Move this. Somewhere
	sdata->lua_stream = open_memstream (&sdata->lua_iodata, &sdata->lua_streamsz);
// 	setbuf(sdata->lua_stream,4096);
	hackio(L,sdata->lua_stream,sdata->lua_stream,sdata->lua_stream);
	return 0;
}


void dump_table(lua_State *L)
{
	printf("!\n");
	lua_pushnil(L);
	while(lua_next(L, -2) != 0)
	{
		if(lua_isstring(L, -1))
			printf("%s = %s\n", lua_tostring(L, -2), lua_tostring(L, -1));
		else if(lua_isnumber(L, -1))
			printf("%s = %d\n", lua_tostring(L, -2), (int) lua_tonumber(L, -1));
		else if(lua_istable(L, -1))
			dump_table(L);

		lua_pop(L, 1);
	}
}

#define H_SZ	20
static char cmd_history[4096][H_SZ];
static int history_pos=0;


#define history_next history_pos+=1;		\
	if (history_pos==H_SZ) history_pos=0;
#define history_prev  history_pos-=1;		\
	if (history_pos==0) history_pos=H_SZ-1;


#define handle_ctrl(char,code) case char:	\
	buff =					\
	

// static char* return_code(FILE* io, char* code)
// {
// 	char* buff = cmd_history[history_pos];
// 	strcpy(buff, code);
// 	fprintf(io,"\r%s\r\n",code);
// 	history_next
// 	return buff;
// };

static char * smart_fgets(FILE* nio)
{
	char* tmp;

	printf("\n");
	history_next
		char* buff = &cmd_history[history_pos][0];
	tmp = fgets(buff , 4096,nio);
		
	if (tmp == NULL) {
		printf("Client disconnected, didn't expect it.");
		sleep(1);
		return "-- logout";	
	};
#if 0
	int i;
	for (i=0;i<strlen(buff);i++)
		printf("%hhx ", buff[i]);
#endif		
	if (strchr(buff,0x18))
		buff = "aura_reconf();";
	if (strchr(buff,0xfd))
		buff = "-- logout";
	printf("==> %s\n",buff);
	return buff;
}


static int sockfd, newsockfd;
static socklen_t clilen;
static struct sockaddr_in serv_addr, cli_addr;

//Init telnet
//\377\373\001\377\375\042
static const char telnet_initstr[] = "\n";

#define is_cmd(cmd) (strncmp(cmd, buff, strlen(cmd))==0)
int network_init(lua_State* l, char* host, int portno)
{
	printf("Listening at %s:%d\n", host, portno);
	char *buff;
	int err;
	char tmp[5];
	int i=0;
	FILE* newio;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error("ERROR opening socket");
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	i=5;
	while (i--)
	{
		printf("Using port %d\n",portno);
		serv_addr.sin_port = htons(portno);
		err = bind(sockfd, (struct sockaddr *) &serv_addr,
			sizeof(serv_addr));
		if (0 == err) break;
		perror("ERROR on binding ");
		portno++;
	}
	if (err!=0) return err;
	listen(sockfd,5);
	clilen = sizeof(cli_addr);
	while(1)
	{
		newsockfd = accept(sockfd,
				(struct sockaddr *) &cli_addr,
				&clilen);
		if (newsockfd < 0)
			error("ERROR on accept");
		//Send telnet init
 		buff = "hook_login();\n";
		newio = fdopen(newsockfd, "w+");
		//fprintf(newio,telnet_initstr);
		fprintf(newio,"Press ENTER to enter interactive shell\n\r");
		hackio(l,newio,newio,newio);
		fgets(tmp,5,newio);
		fprintf(newio,"\rAzra remote interactive shell (c) Necromant 2012\n\r");
		do {
			if (is_cmd("-- logout")) {
				fprintf(newio,"-- Logged out\n\r");
				fclose(newio);
				close(newsockfd);
				break;
			}else if (is_cmd("-- shutdown")) 
			{
				fprintf(newio,"-- Shutting down aura daemon\n\r");
				fclose(newio);
				close(newsockfd);
				close(sockfd);
				return 0; 
			}else if (is_cmd("-- reset")) 
			{
				fprintf(newio,"-- Hard resetting the environment\n\r");
				fprintf(newio,"-- FIXME: actual hardreset goes here\n\r");
			}
			err = luaL_loadbuffer(l, buff, strlen(buff), "line") ||
				lua_pcall(l, 0, 0, 0);
			if (err) {
				fprintf(newio, "%s\n\r", lua_tostring(l, -1));
				lua_pop(l, 1);
			}
			fprintf(newio,"aura# ");
			fflush(stdout);
			buff = smart_fgets(newio);
		} while (buff);
	}
}

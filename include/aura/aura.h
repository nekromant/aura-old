#ifndef _AURA_H
#define _AURA_H

#include <aura/list.h>
#include <aura/chunk.h>
#include <aura/asyncio.h>

struct aura_hook
{
  lua_CFunction func;
  char* name;
  char* help;
  char* args;
  void* next;
};

#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))

struct aura_epoll_hook
{
	struct epoll_event ev;
	int fd;
	char* name;
	int (*io_handler)(struct epoll_event *ev);
	void* data; //userdata
};


/* Function registration mechanism */
void aura_func_init(lua_State* L);
void aura_func_reg(lua_State* L, struct aura_hook* hook);
void aura_func_reg_list(lua_State* L, struct aura_hook* hook, int count);

/* Runaway process protection */
int aura_protector_init(lua_State *L);


/* Main loop control functions */
extern int aura_init_loop();
int aura_add_epollhook(struct aura_epoll_hook* hook);
/* Set/Clear epollout on descriptor */
int aura_epollout(struct aura_epoll_hook* hook, int status);
void aura_drop_epollhook(struct aura_epoll_hook* hook);
void aura_loop_once(lua_State* L);
void aura_loop_forever(lua_State* L);
int aura_make_fd_nonblock(int sfd);

/* aura plugin loader */
void aura_pluginloader_init(lua_State* L);


/* TODO: Ditch this shit or rework */
/* 
int aura_setup_client(struct aura_epoll_hook* h);
int network_init(lua_State* l, char* host, int portno);
int aura_server_init(lua_State* l, char* host, int portno);


struct aura_server_data
{
	char* name;
	lua_State* L;
	size_t lua_streamsz;
	char* lua_iodata;
	int client_count;
	FILE* lua_stream;
	struct sockaddr_in serv_addr;
	int firstlogin;
};

#define AZRA_CLIBUF_SZ	4096
#define AZRA_CBUF_COUNT	10


struct aura_client_data
{
	struct list_head bclist;
	socklen_t clilen;
	struct aura_epoll_hook* h;
	struct aura_server_data* server;
	char inbuf[AZRA_CLIBUF_SZ]; //input buffer
	int inbpos;
	struct sockaddr_in cli_addr;
	struct aura_charbuf* wrptrs[AZRA_CBUF_COUNT];
	int bufhead; //pos in buffer
	int buftail; //pos in buffer
	int outpos; //output buffer pos
};


void _aura_broadcastf(struct aura_client_data* cli, const char* fmt, ...);
#define aura_broadcastf(cli,fmt,...) \
	_aura_broadcastf(cli, "<b>%s</b>: " fmt, cli->h->name, ##__VA_ARGS__)

#define aura_cbroadcastf(fmt,...) \
	_aura_broadcastf(NULL, "<b>aura</b>: " fmt, ##__VA_ARGS__)
	
void aura_broadcaster_add_client(struct aura_client_data* cli);
int aura_broadcaster_init(FILE* log);
*/


#endif

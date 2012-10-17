#ifndef _AZRA_H
#define _AZRA_H

#include <azra/list.h>

struct azra_hook
{
  lua_CFunction func;
  char* name;
  char* help;
  char* args;
  void* next;
};

/* Function registration mechanism */
void azra_func_init(lua_State* L);
void azra_func_reg(lua_State* L, struct azra_hook* hook);
void azra_func_reg_list(lua_State* L, struct azra_hook* hook, int count);

/* Runaway process protection */
int azra_protector_init(lua_State *L);

struct azra_epoll_hook
{
	 struct epoll_event ev;
	 int fd;
	 char* name;
	 int (*io_handler)(struct epoll_event *ev);
	 void* data; //userdata
};

/* Main loop control functions */
extern int azra_init_loop();
int azra_add_epollhook(struct azra_epoll_hook* hook);
void azra_drop_epollhook(struct azra_epoll_hook* hook);

void azra_loop_once(lua_State* L);
void azra_loop_forever(lua_State* L);

int azra_make_fd_nonblock(int sfd);

/* Azra plugin loader */
void azra_pluginloader_init(lua_State* L);


int azra_setup_client(struct azra_epoll_hook* h);
int network_init(lua_State* l, char* host, int portno);
int azra_server_init(lua_State* l, char* host, int portno);



struct azra_server_data
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

struct azra_charbuf
{
	int refcnt;
	int len;
	char* buffer;
};


struct azra_client_data
{
	struct list_head bclist;
	socklen_t clilen;
	struct azra_epoll_hook* h;
	struct azra_server_data* server;
	char inbuf[AZRA_CLIBUF_SZ]; //input buffer
	int inbpos;
	struct sockaddr_in cli_addr;
	struct azra_charbuf* wrptrs[AZRA_CBUF_COUNT];
	int bufhead; //pos in buffer
	int buftail; //pos in buffer
	int outpos; //output buffer pos
};


void _azra_broadcastf(struct azra_client_data* cli, const char* fmt, ...);
#define azra_broadcastf(cli,fmt,...) \
	_azra_broadcastf(cli, "<b>%s</b>: " fmt, cli->h->name, ##__VA_ARGS__)

#define azra_cbroadcastf(fmt,...) \
	_azra_broadcastf(NULL, "<b>azra</b>: " fmt, ##__VA_ARGS__)
	
void azra_broadcaster_add_client(struct azra_client_data* cli);
int azra_broadcaster_init(FILE* log);

int azra_epollout(struct azra_epoll_hook* hook, int status);
struct azra_charbuf* broadcaster_get_message(struct azra_client_data* cdata);
#define azra_broadcaster_drop_client(cli) \
list_del(&cli->bclist);
void azra_charbuf_put(struct azra_charbuf* ptr);
void broadcaster_put_message(struct azra_client_data* cdata, 
	struct azra_charbuf* msg);

#endif

#ifndef _AZRA_H
#define _AZRA_H

#include <azra/list.h>

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
int azra_server_init(lua_State* l, char* host, int portno);

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

struct azra_epoll_hook
{
	 struct epoll_event ev;
	 int fd;
	 char* name;
	 int (*io_handler)(struct epoll_event *ev);
	 void* data; //userdata
};


struct azra_server_data
{
	char* name;
	lua_State* L;
	size_t lua_streamsz;
	char* lua_iodata;
	int client_count;
	FILE* lua_stream;
	struct sockaddr_in serv_addr;
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

int azra_init_loop();
int azra_add_epollhook(struct azra_epoll_hook* hook);
void azra_drop_epollhook(struct azra_epoll_hook* hook);
int azra_main_loop();
int azra_make_fd_nonblock(int sfd);
int azra_setup_client(struct azra_epoll_hook* h);
void _azra_broadcastf(struct azra_client_data* cli, const char* fmt, ...);
#define azra_broadcastf(cli,fmt,...) \
	_azra_broadcastf(cli, "<b>%s</b>: " fmt, cli->h->name, ##__VA_ARGS__)

#define azra_cbroadcastf(fmt,...) \
	_azra_broadcastf(NULL, "<b>azra</b>:" fmt, ##__VA_ARGS__)
	
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
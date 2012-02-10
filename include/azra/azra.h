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
	struct sockaddr_in serv_addr;
};

#define AZRA_CLIBUF_SZ	4096
struct azra_charbuf
{
	int refcnt;
	int pos;
	int len;
	char* buffer;
};

struct azra_client_data
{
	socklen_t clilen;
	char inbuf[AZRA_CLIBUF_SZ]; //input buffer
	int inbpos;
	struct sockaddr_in cli_addr;
};

int azra_init_loop();
int azra_add_epollhook(struct azra_epoll_hook* hook);
void azra_drop_epollhook(struct azra_epoll_hook* hook);
int azra_main_loop();
int azra_make_fd_nonblock(int sfd);
int azra_setup_client(struct azra_epoll_hook* h);

#endif
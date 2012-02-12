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
#include <stdarg.h>
#include <stdio.h>
#include <lua.h>
#include <errno.h>
#include <lauxlib.h>
#include <string.h>
#include <azra/azra.h>
#include "config.h"


static FILE* io;
static LIST_HEAD(clients);

int azra_broadcaster_init(FILE* log)
{
	io=log;
	return 0;
}

void azra_broadcaster_add_client(struct azra_client_data* cli)
{
	list_add(&cli->bclist,&clients);
}




static char *make_message(const char *fmt, va_list ap) {
    int n, size = 100;
    char *p;
    if ((p = malloc (size)) == NULL)
        return NULL;
    while (1) {
        n = vsnprintf (p, size, fmt, ap);
        va_end(ap);
        if (n > -1 && n < size)
            return p;
        if (n > -1)    /* glibc 2.1 */
            size = n+1; /* Это то, что необходимо */
        else           /* glibc 2.0 */
            size *= 2;  /* Удвоить стаpый pазмеp */
        if ((p = realloc (p, size)) == NULL)
            return NULL;
    }
}


struct azra_charbuf *azra_allocate_charbuf(char* msg)
{
	struct azra_charbuf* ptr = malloc(sizeof(struct azra_charbuf));
	ptr->buffer = msg;
	ptr->len = strlen(msg);
	ptr->refcnt = 0;
	return ptr;
}


 void azra_charbuf_put(struct azra_charbuf* ptr)
{
	if (1 == ptr->refcnt--)
	{
		printf("azra: No more references - deleting message\n");
		free(ptr->buffer);
		free(ptr);
	}
}



#define _inc(n,top) \
	n++;\
	if (n==top) n=0;

#define _dec(n,top)\
	n--;\
	if (n==0) n=top;
	
void broadcaster_put_message(struct azra_client_data* cdata, 
	struct azra_charbuf* msg)
{
	_inc(cdata->buftail, AZRA_CBUF_COUNT); 
	azra_charbuf_put(msg);
}



void broadcaster_push_message(
	struct azra_client_data* cdata, 
	struct azra_charbuf* buf
	)
{
	printf("azra: pushbuf: %d %d\n", cdata->buftail, cdata->bufhead);
	int next = cdata->bufhead;
	_inc(next,AZRA_CBUF_COUNT);
	if (cdata->buftail!=next)
	{
		cdata->wrptrs[cdata->bufhead] = buf;
		cdata->bufhead=next;
		buf->refcnt++;
	}else
	{
		printf("azra: Queue space exhausted, dropping message\n");
	}
}

struct azra_charbuf* broadcaster_get_message(
	struct azra_client_data* cdata)
{
	printf("azra: getbuf: %d %d\n", cdata->buftail, cdata->bufhead);
	if (cdata->buftail!=cdata->bufhead)
	{
		return cdata->wrptrs[cdata->buftail];
	}
	return NULL;
}


// #define bufhead_inc(buf)
void _azra_broadcastf(struct azra_client_data* cli, const char* fmt, ...)
{
	va_list ap;
	va_start(ap,fmt);
	
	char* from = "azra:";
	if (cli) from = cli->h->name;
	char* msg = make_message(fmt,ap);
	fprintf(io, "%s: %s\n", from, msg);
	struct azra_charbuf *buf = azra_allocate_charbuf(msg);
	buf->buffer=msg;
	va_end(ap);
	struct list_head* cur;
	struct azra_client_data* cdata;
	list_for_each(cur, &clients)
	{
		cdata = list_entry(cur, struct azra_client_data, bclist);
		if (cdata!=cli)
		{
			printf("azra: broadcast dispatch to %s\n", cdata->h->name);
			broadcaster_push_message(cdata, buf);
			azra_epollout(cdata->h,1); //enable epollout reporting
		}
	}
	//Now we need to patch this to all the clients.
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

/* IO hack. This replaces the io library's idea of stdin/out/err
 * with 3 new FILE* handles, without altering the C stdin/out/err
 * values.
 *
 * It's a hugely ugly hack, fiddling with the internals of the io
 * library. Tested with 5.1.2, may or may not work with other
 * versions...
 */
static void hackio(lua_State *L, FILE *in, FILE *out, FILE *err) {
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


static int sockfd, newsockfd;
static socklen_t clilen;
static struct sockaddr_in serv_addr, cli_addr;
FILE* newio;
int network_init(char* host, int portno, lua_State* L)
{
	 char tmp[4096];
	 sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        error("ERROR opening socket");
     bzero((char *) &serv_addr, sizeof(serv_addr));
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              error("ERROR on binding");
     listen(sockfd,5);
     clilen = sizeof(cli_addr);
	 while(1)
	 {
		newsockfd = accept(sockfd, 
                 (struct sockaddr *) &cli_addr, 
                 &clilen);
		if (newsockfd < 0) 
          error("ERROR on accept");
        bzero(tmp,256); 
        newio = fdopen(newsockfd, "r+");
        hackio(L,newio,newio,newio); 
	 }
}
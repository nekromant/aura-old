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
#include <string.h>
#include <azra/azra.h>
#include <azra/urpc.h>

/* This features a very curious parser to optimize the pack/unpack
 * times. To avoid tokenizing the string each time, we build an array of
 * pointers to pack/unpack functions, that serve as a cache
 *
 */


#define STACK_START 3
/* Packs data from lua starting with element, return number of pushed elements */
char* urpc_pack_data(lua_State* L, size_t sz, int reserved, char* format) 
{
	char* data = malloc(sz);

}

/* Unpacks data, pushes it to lua, return number of pushed elements */
int urpc_unpack_data(lua_State* L, char* data, char* format) 
{
	
}

int strdlmcnt(char* str, char d) 
{
	int i;
	int count=0;
	for (i=0; i<strlen(str);i++) {
		if (str[i]==d) 
			count++;
	}
	return count;
}


/* VALID FORMAT TOKENS 
 *
 * s    - null-terminated string
 * 1d/u - byte, signed/unsigned
 * 2d/u - uint16_t signed/unsigned
 * 4d/u - uint32_t signed/unsigned
 * 8d/u - uint64_t signed/unsigned
 * 123r - raw data of length 123 (lightuserdata)
 * 
 */

int urpc_pack_string(lua_State *L, int n, char* dest, int destsz, int swap)
{
	const char* s = lua_tostring(L, n);
	int tsz = strlen(s)+1;
	if (destsz<tsz)
		return -1;
	strcpy(dest, s);
	return tsz;
}

int urpc_unpack_string(lua_State *L, char* src, int swap)
{
	lua_pushstring(L, src);
	return strlen(src)+1;
}

int urpc_pack_u8(lua_State *L, int n, unsigned char* dest, int destsz, int swap)
{
	int tsz = 1;
	if (destsz<tsz)
		return -1;
	int num = lua_tonumber(L, n);
	unsigned char dt = (unsigned char) num;
	*dest=dt;
	return tsz;
}

int urpc_unpack_u8(lua_State *L, unsigned char* src, int swap)
{
	lua_pushnumber(L, *src);
	return 1;
}


void* urpc_argcache(lua_State* L, char* format, int swap, int pack) {
	int argcount = strdlmcnt(format,';');
	char* tmp = strdup(format);
	char* tok = strtok(tmp, ";");
	while (tok) {
		if (*tok == 's') {
		}
	}
}

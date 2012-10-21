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
#include <aura/aura.h>
#include <aura/urpc.h>



/* This features a very curious parser to optimize the pack/unpack
 * times. To avoid tokenizing the string each time, we build an array of
 * pointers to pack/unpack functions, that serves as a dumb cache.
 * 
 */

typedef int (*packfunc_t)(lua_State *L, int n, char* dest, int destsz, int swap);
typedef int (*unpackfunc_t)(lua_State *L, char* src, int swap);


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
 * -- NOT SUPPORTED YET --- 123r - raw data of length 123 (lightuserdata)
 * 
 */

int urpc_pack_string(lua_State *L, int n, char* dest, int destsz, int swap)
{
	const char* s = lua_tostring(L, n);
	int tsz = strlen(s)+1;
	if (destsz<tsz)
		return -tsz;
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

int urpc_pack_s8(lua_State *L, int n, char* dest, int destsz, int swap)
{
	int tsz = 1;
	if (destsz<tsz)
		return -1;
	int num = lua_tonumber(L, n);
	char dt = (char) num;
	*dest=dt;
	return tsz;
}

int urpc_unpack_s8(lua_State *L, char* src, int swap)
{
	lua_pushnumber(L, *src);
	return 1;
}


int urpc_pack_u16(lua_State *L, int n, char* dest, int destsz, int swap)
{
	int tsz = 2;
	if (destsz<tsz)
		return -tsz;
	union u16 number;		
	number.n = lua_tonumber(L, n);
	if (swap) {
		dest[0]=number.bytes[1];
		dest[1]=number.bytes[0];

	} else {
		dest[0]=number.bytes[0];
		dest[1]=number.bytes[1];

	}
	return tsz;
}

int urpc_pack_s16(lua_State *L, int n, char* dest, int destsz, int swap)
{
	int tsz = 2;
	if (destsz<tsz)
		return -tsz;
	union s16 number;		
	number.n = lua_tonumber(L, n);
	if (swap) {
		dest[0]=number.bytes[1];
		dest[1]=number.bytes[0];
	} else {
		dest[0]=number.bytes[0];
		dest[1]=number.bytes[1];
	}
	return tsz;
}

int urpc_unpack_s16(lua_State *L, int16_t* src, int swap)
{
	int16_t tmp;
	if (swap)
		tmp = __swap16(*src);
	else
		tmp = *src;
	lua_pushnumber(L, tmp);
	return 1;
}

int urpc_unpack_u16(lua_State *L, uint16_t* src, int swap)
{
	uint16_t tmp;
	if (swap)
		tmp = __swap16(*src);
	else
		tmp = *src;
	lua_pushnumber(L, tmp);
	return 1;
}




int urpc_pack_u32(lua_State *L, int n, char* dest, int destsz, int swap)
{
	int tsz = 4;
	if (destsz<tsz)
		return -tsz;
	union u32 number;		
	number.n = lua_tonumber(L, n);
	if (swap) {
		dest[0]=number.bytes[3];
		dest[1]=number.bytes[2];
		dest[2]=number.bytes[1];
		dest[3]=number.bytes[0];
	} else {
		dest[0]=number.bytes[0];
		dest[1]=number.bytes[1];
		dest[2]=number.bytes[2];
		dest[3]=number.bytes[3];
	}
	return tsz;
}

int urpc_pack_s32(lua_State *L, int n, char* dest, int destsz, int swap)
{
	int tsz = 4;
	if (destsz<tsz)
		return -tsz;
	union s32 number;		
	number.n = lua_tonumber(L, n);
	if (swap) {
		dest[0]=number.bytes[3];
		dest[1]=number.bytes[2];
		dest[2]=number.bytes[1];
		dest[3]=number.bytes[0];
	} else {
		dest[0]=number.bytes[0];
		dest[1]=number.bytes[1];
		dest[2]=number.bytes[2];
		dest[3]=number.bytes[3];
	}
	return tsz;
}

int urpc_unpack_s32(lua_State *L, int32_t* src, int swap)
{
	int32_t tmp;
	if (swap)
		tmp = __swap32(*src);
	else
		tmp = *src;
	lua_pushnumber(L, tmp);
	return 1;
}

int urpc_unpack_u32(lua_State *L, uint32_t* src, int swap)
{
	uint32_t tmp;
	if (swap)
		tmp = __swap32(*src);
	else
		tmp = *src;
	lua_pushnumber(L, tmp);
	return 1;
}



void** urpc_argcache(lua_State* L, char* format, int pack) 
{
	if (!format)
		return NULL;
	int argcount = strdlmcnt(format,';');
	void** cache = malloc(sizeof(void*)*(argcount+1));
	int i=0;
	char* tmp = strdup(format);
	char* tok = strtok(tmp, ";");
	int len;
	char fmt;
	while (tok) {
		sscanf(tok, "%d%c", &len, &fmt);
		if (*tok == 's') {
			cache[i] = pack ? (void*) urpc_pack_string : (void*) urpc_unpack_string;
		} else if (('d' == fmt)) {
			switch(len) {
			case 1:
				cache[i] =  pack ? (void*) urpc_pack_s8 : (void*) urpc_unpack_s8;
				break;
			case 2:
				cache[i] = pack ? (void*) urpc_pack_s16 : (void*) urpc_unpack_s16;
				break;
			case 4:
				cache[i] = pack ? (void*) urpc_pack_s32 : (void*) urpc_unpack_s32;
				break;
			default:
				goto error;
				break;
			}
		} else if (('u' == fmt)) {
			switch(len) {
			case 1:
				cache[i] = pack ? (void*) urpc_pack_u8 : (void*) urpc_unpack_u8;
				break;
			case 2:
				cache[i] = pack ? (void*) urpc_pack_u16 : (void*) urpc_unpack_u16;
				break;
			case 4:
				cache[i] = pack ? (void*) urpc_pack_u32 : (void*) urpc_unpack_u32;
				break;
			default:
				goto error;
				break;
			} 
		} else
			goto error;
		i++;
		tok = strtok(NULL, ";");
	}
	cache[i]=NULL;
	free(tmp);
	return cache;
error:
	free(tmp);
	free(cache);
	return 0;
}


#define STACK_START 3
/* Packs data from lua starting with element, return number of pushed elements */
/* leaves 'reserved' bytes at the very start */
struct aura_chunk* urpc_pack_data(
	lua_State* L, 
	struct urpc_instance* inst,
	size_t sz, 
	int reserved, 
	void** cache, 
	int swap) 
{
	struct aura_chunk* chunk = aura_chunk_allocate(sz);
	if (!chunk) 
		return NULL;
	packfunc_t pack;
	int i=0;
	int n;
	int lua_arg=STACK_START;
	chunk->size = reserved;
	if (!cache)
		return chunk;
	while(1) {
	retry: 
		pack = cache[i];
		if (!pack) 
			break;
		n = pack(L, lua_arg++, 
			 &chunk->data[chunk->size], 
			 chunk->alloc - chunk->size, 
			 swap);
		if (n<0) {
			aura_chunk_realloc(chunk,-n*2);
			goto retry;
		}
		chunk->size+=n;	
		i++;
	}
	return chunk;
}

/* Unpacks data, pushes it to lua, return number of pushed elements */
int urpc_unpack_data(lua_State* L, char* data, void** cache, int swap) 
{
	if (!cache)
		return 0;
	unpackfunc_t unpck;
	int i=0;
	int bytes;
	while(1) {
		unpck = cache[i];
		if (!unpck)
			break;
		bytes = unpck(L, data, swap);
		data+=bytes;
		i++;
	}
	return i;
}

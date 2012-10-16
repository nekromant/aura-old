#ifndef __URPC_H
#define __URPC_H

#include <stdint.h>

#define FLAG_METHOD 1
#define FLAG_EVENT  2

#define OBJECT_IS_EVENT(obj)			\
	(obj->flags & FLAG_EVENT)

#define OBJECT_IS_METHOD(obj)			\
	(obj->flags & FLAG_METHOD)

#define __swap16(value)					\
	((((uint16_t)((value) & 0x00FF)) << 8) |	\
	 (((uint16_t)((value) & 0xFF00)) >> 8))

#define __swap32(value)					\
	((((uint32_t)((value) & 0x000000FF)) << 24) |	\
         (((uint32_t)((value) & 0x0000FF00)) << 8) |	\
         (((uint32_t)((value) & 0x00FF0000)) >> 8) |	\
         (((uint32_t)((value) & 0xFF000000)) >> 24))

#define __swap64(value)						\
	(((((UInt64)value)<<56) & 0xFF00000000000000ULL)  |	\
	 ((((UInt64)value)<<40) & 0x00FF000000000000ULL)  |	\
	 ((((UInt64)value)<<24) & 0x0000FF0000000000ULL)  |	\
	 ((((UInt64)value)<< 8) & 0x000000FF00000000ULL)  |	\
	 ((((UInt64)value)>> 8) & 0x00000000FF000000ULL)  |	\
	 ((((UInt64)value)>>24) & 0x0000000000FF0000ULL)  |	\
	 ((((UInt64)value)>>40) & 0x000000000000FF00ULL)  |	\
	 ((((UInt64)value)>>56) & 0x00000000000000FFULL))

/* represents individual methods and events from an instance */
struct urpc_object {
unsigned int id;
	char flags;
	char* name;
	char* args;
	char* reply;
	struct urpc_object* next;
};	

/* represents an instance with object cache */
struct urpc_instance {
	struct urpc_transport *transport;
	struct urpc_object* head;
	struct urpc_object** objects;
	void* private_data;
};

/* represents a transport plugin */
struct urpc_transport {
	char* name;
	int (*open)(lua_State* L);
	int (*call)(lua_State* L, struct urpc_instance* instance, int id);
	int (*discovery)(lua_State* L, struct urpc_instance* instance);
};

#define URPC_INSTANCE_PRIVATE(data)		\
	data->private_data

#endif

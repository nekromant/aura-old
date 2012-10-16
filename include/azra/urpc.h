#ifndef __URPC_H
#define __URPC_H


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
	struct urpc_object** objects;
	void* private_data;
};

/* represents a transport plugin */
struct urpc_transport {
	char* name;
	int (*open)(lua_State* L);
	int (*call)(lua_State* L, struct urpc_instance* instance);
	int (*discovery)(lua_State* L, struct urpc_instance* instance);
};

#define URPC_INSTANCE_PRIVATE(data) \
	data->private_data

#endif

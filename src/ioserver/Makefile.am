bin_PROGRAMS = aura
aura_SOURCES = \
	lua_perflimits.c \
	hooks.c \
	main_loop.c \
	main.c \
	plugin.c \
	asyncio.c \
	chunk.c

AM_CFLAGS = $(lua_CFLAGS) -Wall -fPIC -ldl -rdynamic -lreadline -Wall -Werror -I$(top_srcdir)/include
aura_LDADD = $(lua_LIBS)

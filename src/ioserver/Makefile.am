bin_PROGRAMS = azra
azra_SOURCES = \
	lua_perflimits.c \
	hooks.c \
	main_loop.c \
	main.c \
	plugin.c

AM_CFLAGS = $(lua_CFLAGS) -Wall -fPIC -ldl -rdynamic -lreadline -Wall -Werror -I$(top_srcdir)/include
azra_LDADD = $(lua_LIBS)

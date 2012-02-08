bin_PROGRAMS = azra
azra_SOURCES = hooks.c \
	azra.c \
	network.c \
	uart.c \
	main_loop.c \
	netcmdio.c
AM_CFLAGS = $(lua_CFLAGS) -Wall -Werror -g
azra_LDADD = $(lua_LIBS)

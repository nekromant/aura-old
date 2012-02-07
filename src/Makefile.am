bin_PROGRAMS = azra
azra_SOURCES = hooks.c \
	azra.c \
	network.c \
	uart.c
AM_CFLAGS = $(lua_CFLAGS) -Wall -Werror
azra_LDADD = $(lua_LIBS)

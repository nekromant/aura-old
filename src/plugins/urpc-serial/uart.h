#ifndef _UART_H
#define _UART_H

struct uart_settings_t {
	tcflag_t ifl;
	tcflag_t cfl;
	tcflag_t ofl;
	char * port;
	char* tag;
	int fd;
	int speed;
	struct termios oldtio;
	struct termios newtio;

};

struct uart_settings_t* str_to_uart_settings();
#endif

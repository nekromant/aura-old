#define _GNU_SOURCE 1 /* POSIX compliant source */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "uart.h"

//tag:/dev/ttyUSB0:115200:8:n:1\n
//#define set_prop(key,value) if (t)
#define MAPSPEED(speed) case speed: s->cfl|=B##speed; break;
#define MAPBITS(bits) case bits: s->cfl|=CS##bits; break;
#define FAULT(code, text, ...)			\
	default:\
	fprintf(stderr, text, __VA_ARGS__);  \
	goto error;
			


struct uart_settings_t* urpcserial_make_settings(const char* settings) 
{
	struct uart_settings_t* s = calloc(1,sizeof(struct uart_settings_t));
	s->fd=0;
	if (!s)
		return NULL;
	char* str = strdup(settings);
	char* t = strtok(str, ":");
	s->port = t;
	t = strtok(NULL, ":");
	int speed;
	if (t) {
		sscanf(t, "%d", &speed);
		printf("urpc-serial: running at %d baud,", speed);
/* cat /usr/include/bits/termios.h |grep define|grep " B" */
		switch(speed) {
			MAPSPEED(230400);
			MAPSPEED(115200);
			MAPSPEED(57600);
			MAPSPEED(38400);
			MAPSPEED(19200);
			MAPSPEED(9600);
			MAPSPEED(4800);
			MAPSPEED(2400);
			MAPSPEED(1200);
			MAPSPEED(300);
			FAULT(1,"The requested speed (%d) doesn't seem to be valid.\n", speed);
		}
	}
	t = strtok(NULL, ":");
	int bits=0;
	if (t) {
		sscanf(t, "%d", &bits);
		printf(" %d data bits,", bits);
		switch(bits) {
			MAPBITS(5);
			MAPBITS(6);
			MAPBITS(7);
			MAPBITS(8);
			FAULT(1,"The requested data bit count (%d) doesn't seem to be valid.\n", bits);
		}
	}
	t = strtok(NULL, ":");
	if (t) {
		if (strcmp(t,"o")==0) {
                        /* odd */
			printf(" odd parity,");;
			s->cfl|=PARENB|PARODD;
		} else if (strcmp(t,"e")==0) {
                        /* even */
			printf(" even parity,");;
			s->cfl|=PARENB;
		} else {
			printf(" no parity,");
		}
	}

	t = strtok(NULL, ":");
	if (t && *t=='2') {
		s->ofl|=CSTOPB;
		printf(" 2 stop bits\n");
	} else {
		printf(" 1 stop bit\n");
	}
	return s;
error:
	free(str);
	free(s);
	return 0;
}

int urpcserial_uart_init(struct uart_settings_t* us)
{
	if (us->fd<=0) {
		us->fd = open(us->port, O_RDWR | O_NOCTTY | O_NONBLOCK  );
		if (us->fd <0) {
			fprintf(stderr, "failed to open port %s\n", us->port);
			perror("error: ");
			return -EIO;
		}
		fcntl(us->fd, F_SETFL, 0);
	}
	tcdrain(us->fd);
	tcflush(us->fd, TCOFLUSH);
	tcgetattr(us->fd,&us->oldtio); /* save current port settings */
	bzero(&us->newtio, sizeof(us->newtio));
	cfmakeraw(&us->newtio);
	us->newtio.c_cflag =  us->cfl | CLOCAL | CREAD;
	us->newtio.c_iflag = us->ifl;
	us->newtio.c_oflag = us->ofl;
	/* set input mode (non-canonical, no echo,...) */
	us->newtio.c_lflag = 0;
	us->newtio.c_cc[VTIME]    = 0; 
	us->newtio.c_cc[VMIN]     = 0; 
	tcdrain(us->fd);
	tcsetattr(us->fd, TCSADRAIN, &us->newtio);
	return us->fd;
}

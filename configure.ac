#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.67])
AC_INIT([azra], [0.1], [azrabugs@necromant.ath.cx])
AM_INIT_AUTOMAKE
LT_INIT
AC_CONFIG_SRCDIR([include/config.h.in])
AC_CONFIG_HEADERS([include/config.h])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([netinet/in.h stdlib.h string.h sys/socket.h unistd.h])
AC_CHECK_HEADERS([termios.h])
PKG_CHECK_MODULES([lua], [lua >= 5.1])

AC_CHECK_HEADERS([endian.h],
	AC_TRY_COMPILE([#include <endian.h>], [switch (1) { case __LITTLE_ENDIAN: break; case __BYTE_ORDER: break; } ],[is_little_endian=0], [is_little_endian=1]),
)

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_CHECK_FUNCS([bzero socket strchr])


AC_CONFIG_FILES([Makefile
                 src/Makefile
		 src/plugins/Makefile
		 src/plugins/dummy/Makefile	 
		 src/plugins/urpc/Makefile
		 src/plugins/urpc-null/Makefile
                 src/ioserver/Makefile
                 src/client/Makefile
                 ])
AC_OUTPUT

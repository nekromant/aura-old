#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.67])
AC_INIT([azra], [0.1], [aurabugs@ncrmnt.org])
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
		 src/plugins/urpc-serial/Makefile	
                 src/ioserver/Makefile
                 src/client/Makefile
                 ])
AC_OUTPUT

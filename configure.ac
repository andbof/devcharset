AC_PREREQ([2.67])
AC_INIT([devcharset],m4_esyscmd_s([cat .tarball-version 2>/dev/null || git describe --always --dirty]))
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])
AC_CONFIG_SRCDIR([src])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

AC_CONFIG_FILES([Makefile])

AC_PROG_CC
AC_PROG_CC_C99
if test "x$ac_cv_prog_cc_c99" = "xno"; then
	AC_MSG_ERROR([A c99 compiler is required])
fi
AM_PROG_CC_C_O
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])    # Fixes automake 1.12 (where -Wall implies portability warnings), but automake <= 1.11 does not recognize it
AC_LANG_C
AC_PROG_INSTALL
AX_PROG_BISON([],
              AS_IF([test ! -f "$srcdir/parser.c"],
                    [AC_MSG_ERROR([bison not found - unable to compile parser.c])]))
AX_PROG_FLEX([],
             AS_IF([test ! -f "$srcdir/lexer.c"],
                   [AC_MSG_ERROR([flex not found - unable to compile lexer.c])]))

AC_CHECK_PROG([has_pkg_config],[pkg-config],[yes])
if test "x$has_pkg_config" != "xyes"; then
	AC_MSG_ERROR([pkg-config not found. You must re-run bootstrap after installing it or you will get a syntax error message.])
fi
PKG_CHECK_MODULES([fuse], [fuse >= 2.8.0])

AC_OUTPUT

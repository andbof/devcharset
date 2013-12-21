ACLOCAL_AMFLAGS = -I m4

TESTS = parser_test

bin_PROGRAMS = devcharset
check_PROGRAMS = parser_test

AM_CFLAGS = -Wall $(fuse_CFLAGS)
AM_CPPFLAGS = $(fuse_CPPFLAGS)
AM_LDFLAGS = $(fuse_LIBS)
AM_YFLAGS = -d

BUILT_SOURCES = parser.c parser.h lexer.c

devcharset_SOURCES = \
		     devcharset.c \
		     lexer.l \
		     parse.h \
		     parser.y

parser_test_SOURCES = \
		      lexer.l \
		      parse.h \
		      parser.y \
		      test/parser-test.c

# This is a way of keeping version information in the distribution tarball
# even when there is no version control system available.
dist-hook:
	echo $(PACKAGE_VERSION) > $(distdir)/.tarball-version

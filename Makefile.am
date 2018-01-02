ACLOCAL_AMFLAGS = -I m4

TESTS = parser_test

bin_PROGRAMS = devcharset
check_PROGRAMS = parser_test

AM_CFLAGS = -Wall $(fuse_CFLAGS)
AM_CPPFLAGS = $(fuse_CPPFLAGS)
AM_LDFLAGS = $(fuse_LIBS)
AM_YFLAGS = -d

BUILT_SOURCES = \
		src/parser.c \
		src/parser.h \
		src/lexer.c

devcharset_SOURCES = \
		     src/devcharset.c \
		     src/lexer.l \
		     src/parse.h \
		     src/parser.y

parser_test_CPPFLAGS = -I$(srcdir)/src
parser_test_SOURCES = \
		      src/lexer.l \
		      src/parse.h \
		      src/parser.y \
		      test/parser-test.c

# This is a way of keeping version information in the distribution tarball
# even when there is no version control system available.
dist-hook:
	echo $(PACKAGE_VERSION) > $(distdir)/.tarball-version

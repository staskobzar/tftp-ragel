SUBDIRS=lib

AM_CFLAGS = -Ilib

bin_PROGRAMS = tftpclient
tftpclient_SOURCES = main.c
tftpclient_CFLAGS = @APR_CFLAGS@ -I$(top_builddir)/src/lib
tftpclient_LDADD = -L$(top_builddir)/src/lib -ltftp @APR_LIBS@

#AUTOMAKE_OPTIONS=dejagnu
#DEJATOOL = frontend
#RUNTESTFLAGS=LUHN=$(top_builddir)/src/luhn

if HAVE_CMOCKA
  TESTS = tftp_msg_read_test tftp_msg_create_test
  check_PROGRAMS = tftp_msg_read_test tftp_msg_create_test

  tftp_msg_read_test_SOURCES = tftp_msg_read_test.c
  tftp_msg_read_test_CFLAGS = @CMOCKA_CFLAGS@ -I$(top_builddir)/src/lib @APR_CFLAGS@
  tftp_msg_read_test_LDADD = -L$(top_builddir)/src/lib -ltftp @CMOCKA_LIBS@ @APR_LIBS@

  tftp_msg_create_test_SOURCES = tftp_msg_create_test.c
  tftp_msg_create_test_CFLAGS = @CMOCKA_CFLAGS@ -I$(top_builddir)/src/lib @APR_CFLAGS@
  tftp_msg_create_test_LDADD = -L$(top_builddir)/src/lib -ltftp @CMOCKA_LIBS@ @APR_LIBS@
endif


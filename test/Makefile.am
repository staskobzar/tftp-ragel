#AUTOMAKE_OPTIONS=dejagnu
#DEJATOOL = frontend
#RUNTESTFLAGS=LUHN=$(top_builddir)/src/luhn

if HAVE_CMOCKA
  TESTS = tftp_read_pack_test tftp_create_pack_test
  check_PROGRAMS = tftp_read_pack_test tftp_create_pack_test

  tftp_read_pack_test_SOURCES = tftp_read_pack_test.c
  tftp_read_pack_test_CFLAGS = @CMOCKA_CFLAGS@ -I$(top_builddir)/src/lib @APR_CFLAGS@
  tftp_read_pack_test_LDADD = -L$(top_builddir)/src/lib -ltftp @CMOCKA_LIBS@ @APR_LIBS@

  tftp_create_pack_test_SOURCES = tftp_create_pack_test.c
  tftp_create_pack_test_CFLAGS = @CMOCKA_CFLAGS@ -I$(top_builddir)/src/lib @APR_CFLAGS@
  tftp_create_pack_test_LDADD = -L$(top_builddir)/src/lib -ltftp @CMOCKA_LIBS@ @APR_LIBS@
endif


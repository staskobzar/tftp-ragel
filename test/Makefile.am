#AUTOMAKE_OPTIONS=dejagnu
#DEJATOOL = frontend
#RUNTESTFLAGS=LUHN=$(top_builddir)/src/luhn

if HAVE_CMOCKA
  TESTS = tftp_test
  check_PROGRAMS = tftp_test
  tftp_test_SOURCES = tftp_test.c
  tftp_test_CFLAGS = @CMOCKA_CFLAGS@ -I$(top_builddir)/src/lib @APR_CFLAGS@
  tftp_test_LDADD = -L$(top_builddir)/src/lib -ltftp @CMOCKA_LIBS@ @APR_LIBS@
endif


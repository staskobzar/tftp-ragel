noinst_LIBRARIES=libtftp.a
libtftp_a_SOURCES=tftp.c tftp.h
libtftp_a_CFLAGS = @APR_CFLAGS@
libtftp_a_LIBTOOLFLAGS = @APR_LIBS@

tftp.c: tftp.rl
	@echo "RAGEL tftp.rl"
	@ragel tftp.rl

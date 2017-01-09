noinst_LIBRARIES=libtftp.a
libtftp_a_SOURCES=tftp_msg.c tftp_msg.h tftp_proto.c tftp_proto.h util.c util.h
libtftp_a_CFLAGS = @APR_CFLAGS@
libtftp_a_LIBTOOLFLAGS = @APR_LIBS@

tftp_msg.c: tftp_msg.rl
	@echo "RAGEL tftp_msg.rl"
	@ragel tftp_msg.rl

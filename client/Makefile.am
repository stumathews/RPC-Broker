AM_CPPFLAGS = $(LIBINC)
bin_PROGRAMS = client
client_SOURCES = \
    client.c \
    ../common/common.c \
    pack_client_request_data.c \
    send_and_receive.c \
    client-proxy.c
client_LDFLAGS = $(LIBLINK)
BUILT_SOURCES = client-proxy.c
CLEANFILES = client-proxy.c
client-proxy.c: Makefile 
	./generate_proxy.pl



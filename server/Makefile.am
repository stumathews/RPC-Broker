AM_CPPFLAGS = $(LIBINC)
bin_PROGRAMS = server
server_SOURCES = \
    server-proxy.c \
    server.c \
    ../common/common.c \
    unpack_marshal_call.c \
    service_register_with_broker.c \
    pack_client_response_data.c
server_LDFLAGS = $(LIBLINK)

AM_CPPFLAGS = $(LIBINC)
bin_PROGRAMS = server
server_SOURCES = \
    server_interface.c \
    server.c \
    ../common/common.c \
    unpack_marshal_call.c \
    service_register_with_broker.c \
    pack_client_response_data.c \
    proxy.gen.c
server_LDFLAGS = $(LIBLINK)

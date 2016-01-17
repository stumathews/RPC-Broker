AM_CPPFLAGS = $(LIBINC)
bin_PROGRAMS = broker
broker_SOURCES = \
    broker.c \
    broker_support.c \
    ../common/common.c \
    unpack_service_registration_buffer.c \
    find_server_for_request.c \
    register_service.c \
    forward_request_to_server.c \
    forward_response_to_client.c \
    find_client_for_response.c \
    get_sender_address.c \
    register_client_request.c 
broker_LDFLAGS = $(LIBLINK)



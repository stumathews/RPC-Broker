#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <stdio.h>
#include <msgpack.h>
#include <stulibc.h>

enum RequestType {REQUEST_SERVICE, REQUEST_REGISTRATION};

struct packet {
    uint32_t len;
    char* buffer;
};

// object to represent a service registration from server to broker
typedef struct ServiceRegistration
{
    char* address;
    char* port;  
    char** services;
    int num_services;
    char* service_name;
} ServiceReg;

typedef struct ProtocolHeader
{
    // Header Text
    char header[20];
    // Header Value type
    msgpack_object val;

    // internal list structure
    struct list_head list;
} ProtocolHeaders;


// object to represent the list of registered servers in the broker
struct ServiceRegistrationsList
{
    struct ServiceRegistration *service_registration;
    struct list_head list;

};

enum RequestType determine_request_type(struct packet* pkt);
int send_request(char* buffer, int bufsize,char* address, char* port);
int client(SOCKET s, struct sockaddr_in* peerp, char* buffer, int length);
void unpack_request_data(char const* buf, size_t len);
char* pack_client_request_data( msgpack_sbuffer* sbuf, char* op,char* fmt, ...);
void _return();
bool service_register_with_broker( char* broker_address, char *broker_port );
msgpack_object extract_header( msgpack_object* obj, char* header_buffer );
void pack_map_str( char* key, char* value, msgpack_packer* pk);
void pack_map_int(char* key, int ival,msgpack_packer* pk );
#endif

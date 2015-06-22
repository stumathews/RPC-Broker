#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <stdio.h>
#include <msgpack.h>
#include <stulibc.h>
#define MAX_HEADER_NAME_SIZE 20
#define MAX_PORT_CHARS 20

enum RequestType {REQUEST_SERVICE, REQUEST_SERVICE_RESPONSE, REQUEST_REGISTRATION};

struct packet {
    uint32_t len;
    char* buffer;
};

typedef struct DestinationEndpoint
{
    char* address;
    char* port;
} Destination;

// object to represent a service registration from server to broker
typedef struct ServiceRegistration
{
    char* address;
    char* port;  
    char** services;
    int num_services;
    char* service_name;
    struct list_head list;
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


enum RequestType determine_request_type(struct packet* pkt);
int send_request(char* buffer, int bufsize,char* address, char* port, bool verbose);
int client(SOCKET s, struct sockaddr_in* peerp, char* buffer, int length, bool verbose);
void unpack_data(char const* buf, size_t len, bool verbose);
char* pack_client_request_data( msgpack_sbuffer* sbuf, char* op,char* fmt, ...);
char* pack_client_response_data( msgpack_sbuffer* sbuf, char* op,char* fmt, ...);
void _return();
msgpack_object extract_header( msgpack_object* obj, char* header_buffer );
void pack_map_str( char* key, char* value, msgpack_packer* pk);
void pack_map_int(char* key, int ival,msgpack_packer* pk );
#endif

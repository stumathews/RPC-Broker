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

typedef struct ServiceRegistration
{
    char* address;
    char* port;  
} ServiceReg;

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
#endif

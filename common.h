#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <stdio.h>
#include <msgpack.h>
#include <stulibc.h>

struct packet {
    uint32_t len;
    char* buffer;
};


void send_request(char* buffer, int bufsize,char* address, char* port);
void client(SOCKET s, struct sockaddr_in* peerp, char* buffer, int length);
void unpack_request_data(char const* buf, size_t len);
char* pack_request_data( msgpack_sbuffer* sbuf, char* op,char* fmt, ...);
void _return();
#endif

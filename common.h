#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <stdio.h>

struct packet {
    uint32_t len;
    char* buffer;
};


void unpack(char const* buf, size_t len);
#endif

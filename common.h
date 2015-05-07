#ifndef COMMON_H
#define COMMON_H

    struct param {
        char name[80];
        char type[10];
        char value[256];
    } __attribute__((packed));

    struct return_result {
        char type[10];
        char result[256];
    } __attribute__((packed));

    struct broker_message {
        char functionname[50];
        struct param params[5];
        struct return_result result;
    } __attribute__((packed));
    
#endif

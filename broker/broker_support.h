#ifndef BROKER_SUPPORT_H
#define BROKER_SUPPORT_H
#include "common.h"
void register_service(char* buffer,int buflen, struct sockaddr_in* peerp);
struct ServiceRegistration* unpack_service_registration_buffer(char* buffer,int buflen);
void acknowledgement();
Destination *find_server(char* buffer, int len);
void find_client(char* buffer, int len, Destination *dest);
void forward_request(char* buffer, int len,struct sockaddr_in* peerp);
void forward_response(char* buffer, int len,struct sockaddr_in* peerp);
void setPortNumber(char* arg);
void setVerboseFlag(char* arg);
void setWaitIndefinitelyFlag(char* arg);
void print_service_repository();
#endif

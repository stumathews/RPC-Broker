#ifndef BROKER_SUPPORT_H
#define BROKER_SUPPORT_H
#include "common.h"
void register_service(char* buffer,int buflen);
void UnpackServiceRegistrationBuffer(char* buffer,int buflen, struct ServiceRegistration* unpacked);
void acknowledgement();
void find_server(char* buffer, int len, Destination *dest);
void find_client(char* buffer, int len, Destination *dest);
void forward_request(char* buffer, int len);
void forward_response(char* buffer, int len);
void setPortNumber(char* arg);
void setVerbose(char* arg);
void setWaitIndefinitely(char* arg);
void print_service_repository();
#endif

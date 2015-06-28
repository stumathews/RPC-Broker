#ifndef BROKER_SUPPORT_H
#define BROKER_SUPPORT_H
#include "common.h"

struct ServiceRegistration* unpack_service_registration_buffer(char* buffer,int buflen);

Destination* find_server_for_request(Packet packet);
Destination* find_client_for_response(Packet packet);
Destination* get_sender_address( Packet packet, struct sockaddr_in* peerp );
ClientReg*   register_client_request( char* op, Destination* src, int message_id );

void forward_request(Packet packet,struct sockaddr_in* peerp);
void acknowledgement();

void register_service(Packet packet, struct sockaddr_in* peerp);
void forward_response(Packet packet,struct sockaddr_in* peerp);

void print_service_repository();
void print_client_request_repository();

void setPortNumber(char* arg);
void setVerboseFlag(char* arg);
void setWaitIndefinitelyFlag(char* arg);

#endif

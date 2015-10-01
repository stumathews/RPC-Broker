#ifndef BROKER_SUPPORT_H
#define BROKER_SUPPORT_H
#include "common.h"

// Helper method to decode the protocol registration message
struct ServiceRegistration* unpack_service_registration_buffer(char* buffer,int buflen);

// Find a server that the client request is for (we're a broker aren't we!)
Destination* find_server_for_request(Packet* packet);

// Find a client who this response(brokered) from the server is for
Destination* find_client_for_response(Packet* packet);

// Decods the protocol message and gets the sends's address from it
Destination* get_sender_address( Packet* packet, struct sockaddr_in* peerp );

// Register the client in the client repository
ClientReg*   register_client_request( char* op, Destination* src, int message_id );

// Send the client's service requets (Packet) to the server that is known to be able to process it
void forward_request(Packet* packet, Destination* src);
void acknowledgement();

void register_service_request(Packet* packet);
void forward_response(Packet* packet);

// Print all servers registered with the broker's service repository 
void print_service_repository();

// Print all client service requests that the broker is scheduled to broker
void print_client_request_repository();

// Sets the broker's port
void setPortNumber(char* arg);

// Sets the broker's verbose flag
void setVerboseFlag(char* arg);

// Sets the wait indefinitely flag in the broker
void setWaitIndefinitelyFlag(char* arg);

#endif

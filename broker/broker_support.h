#ifndef BROKER_SUPPORT_H
#define BROKER_SUPPORT_H
#include "common.h"


/**
 * @brief Helper method to decode the protocol registration message
 * 
 * @param buffer The raw packet data containing the protocol registration data
 * @param buflen The size of the raw data
 * @return ServiceRegistration*
 */
struct ServiceRegistration* unpack_service_registration_buffer(char* buffer, int buflen, struct BrokerConfig* brokerConfig);


/**
 * @brief Find a server that the client request is for (we're a broker aren't we!)
 * 
 * @param packet The location of the packet's contents
 * @return Location* the location of the server that will process this packet
 */
Location* find_server_for_request(Packet* packet);


/**
 * @brief Find a client who this response(brokered) from the server is for
 * 
 * @param packet Address of the packet
 * @param dest client to send the reponse to
 * @return Location* the client who will get the response
 */
Location* find_client_for_response(Packet *packet, Location* dest, struct BrokerConfig *brokerConfig);


/**
 * @brief Decods the protocol message and gets the sends's address from it
 * 
 * @param packet the raw protocol data
 * @param peerp the peerp
 * @param Location* buffer for the address of the sender
 */
void get_sender_address( Packet* packet, struct sockaddr_in* peerp, Location* addr);


/**
 * @brief Register the client in the client repository
 * 
 * @param op the operation the client is requesting
 * @param src the location of the client
 * @param message_id the message id associated with the request
 * @return ClientReg* 
 */
struct ClientRequestRegistration *register_client_request(char* op, Location* src, int message_id, struct BrokerConfig *brokerConfig);


/**
 * @brief Send the client's service requets (Packet) to the server that is known to be able to process it
 * 
 * @param packet the address of the protocol data from client
 * @param src the client's address
 * @return void
 */
void forward_request_to_server(Packet* packet, Location* src, struct BrokerConfig *brokerConfig);
/**
 * @brief Future acknowledgement of recept of data from the broker
 * 
 * @return void
 */

void acknowledgement();

/**
 * @brief Tracks the client's request for a service
 * 
 * @param packet the clients revice request data
 * @return void
 */
void register_service_request(Packet* packet, struct BrokerConfig *brokerConfig);
/**
 * @brief Forwards a packet on
 * 
 * @param packet the packet to forward on
 * @return void
 */

void forward_response_to_client(Packet* response, struct BrokerConfig *brokerConfig);


/**
 * @brief Print all servers registered with the broker's service repository 
 * 
 * @return void
 */
void print_service_repository();


/**
 * @brief Print all client service requests that the broker is scheduled to broker
 * 
 * @return void
 */
void print_client_request_repository();


/**
 * @brief Sets the broker's port
 * 
 * @param arg Sets the broker's listening port
 * @return void
 */
void setPortNumber(char* arg);


/**
 * @brief Sets the broker's verbose flag
 * 
 * @param arg (ignored)
 * @return void
 */
void setVerboseFlag(char* arg);


/**
 * @brief Sets the wait indefinitely flag in the broker
 * 
 * @param arg (ignored)
 * @return void
 */
void setWaitIndefinitelyFlag(char* arg);
/**
 * @brief Sets the brokers address
 * 
 * @param arg address to bind the broker to
 * @return void
 */
void setOurAddress(char* arg);

#endif

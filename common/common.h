#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <stdio.h>
#include <msgpack.h>
#include <stulibc.h>
#include "protocol.h"

#define ON 1;
#define INI_PARSE_SUCCESS 0
#define _WAIT_TIMEOUT 0
#define WAIT_ERROR -1
#define FOREVER 1

#ifdef __linux__
#define THREADFUNC(func) void* func(void* params)
#else
#define THREADFUNC(func) unsigned long func(void* params)
#endif

/**
 * @brief A packet consisting of the length of it and then the trailing data
 *
 */
typedef struct Packet {
	uint32_t len;
	char* buffer;
} Packet;

/**
 * @brief Used to describe a address/port combination
 *
 */
typedef struct DestinationEndpoint {
	char* address;
	char* port;
} Location;

/**
 * @brief object to represent a service registration from server to broker
 *
 */
typedef struct ServiceRegistration {
	char* address;
	char* port;
	char** services;
	char* service_name;
	int num_services;
	struct list_head list;
} ServiceReg;

/**
 * @brief Represents a client registration
 *
 */
typedef struct ClientRequestRegistration {
	char* address;
	char* port;
	char* operation;
	int message_id;
	struct list_head list;
} ClientReg;

typedef struct Details {
	char port[MAX_PORT_CHARS];
	char address[MAX_ADDRESS_CHARS];
} Details;

typedef struct Config {
	bool verbose;
	bool waitIndef;
	List *svc_repo;
	List *clnt_req_repo;
} Config;

struct ServerArgs {
	struct Config* config;
	struct Details* details;
	SOCKET *socket;

};

struct SendArgs {
	Packet* packet;
	char* to_address;
	char* port;
	bool verbose;
	char* wait_response_port;
};

#ifdef __linux__
void* thread_send_request(void* params);
#else
unsigned __stdcall thread_send_request(void* params);
#endif

void copyString(int str_len, const msgpack_object_str* from, char* to);
enum RequestType get_req_type(struct Packet* pkt);
int send_req(Packet* packet, char* address, char* port, bool verbose);
int send_data(SOCKET s, struct sockaddr_in* peerp, Packet* packet, bool verbose);
int get_hdr_int(Packet* packet, char* look_header_name);
void unpack_data(Packet* packet, bool verbose);
void pack_map_str(char* key, char* value, msgpack_packer* pk);
void pack_map_int(char* key, int ival, msgpack_packer* pk);
char* pack_client_request_data(msgpack_sbuffer* sbuf, char* op, char* fmt, ...);
Packet pack_client_response_data(msgpack_sbuffer* sbuf, char* op,
		int message_id, char* fmt, ...);
char* get_hdr_str(Packet* packet, char* look_header_name);
char* get_op_name(Packet* packet);
msgpack_object extract_header(msgpack_object* obj, char* header_buffer);
struct Packet *send_and_receive(Packet* packet, char* address, char* port,
		bool verbose, char* wait_response_port);
void printSetting(Node* LinkedListNode);
void printKeyValuePair(Node* LinkedListNode);
void check_socket(SOCKET s1);
int THREAD_RESULT();
#endif

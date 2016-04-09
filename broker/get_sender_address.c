#include "broker_support.h"
#include "common.h"

void get_sender_address(Packet* packet, struct sockaddr_in* peerp,
		Location* addr) {
	char* reply_port;
	char* sender_address;

	reply_port = get_header_str_value(packet, REPLY_PORT_HDR);
	sender_address = get_header_str_value(packet, SENDER_ADDRESS_HDR);

	addr->address = sender_address;
	addr->port = reply_port;
}


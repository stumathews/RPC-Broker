#include <msgpack.h>
#include <stulibc.h>
#include "common.h"

extern bool wait_response_indef;
extern char client_address[MAX_ADDRESS_CHARS];

Packet* waitForResponse(_Bool verbose, char client_address[MAX_ADDRESS_CHARS], _Bool wait_response_indef, char* wait_response_port)
{
	struct sockaddr_in local;
	struct sockaddr_in peer;
	char* hname;
	char* sname;
	int peerlen;
	SOCKET s1;
	SOCKET s;
	fd_set readfds;
	FD_ZERO(&readfds);
	const int on = 1;
	Packet* result;
	struct timeval timeout = {.tv_sec = 60, .tv_usec = 0};
	NETINIT();
	if (verbose) {
		PRINT("client waiting for response...\n");
	}
	s = netTcpServer(client_address, wait_response_port);
	FD_SET(s, &readfds);
	if (verbose) {
		PRINT("About wait for read on port %s...\n", wait_response_port);
	}
	// wait/block on this listening socket...
	int res = 0;
	if (!wait_response_indef) {
		res = select(s + 1, &readfds, NULL, NULL, &timeout);
	} else {
		res = select(s + 1, &readfds, NULL, NULL, NULL);
	}
	if (res == 0) {
		LOG("timeout");
		netError(1, errno, "timeout!");
	} else if (res == -1) {
		LOG("Select error!");
		netError(1, errno, "select error!!");
	} else {
		peerlen = sizeof(peer);
		if (FD_ISSET(s, &readfds)) {
			s1 = accept(s, (struct sockaddr*) &peer, &peerlen);
			if (!isvalidsock(s1)) {
				netError(1, errno, "accept failed");
			}
			// do network functionality on this socket that now represents a connection with the peer (client)
			result = get_response(s1, verbose);
			NETCLOSE(s1);
		} else {
			DBG("not our socket. continuing");
		}
	}

	NETCLOSE (s);
	return result;
}

/**
 * @brief Sends a packet of data and waits for its response
 *
 * @param packet the data to send
 * @param address the address to send the data to
 * @param port the port to connect to on the address
 * @param verbose true if this function should log verbose messages
 * @param wait_response_port the port that we'll listen locally on for responses(we send this port to recipient)
 * @return Packet* the response from the receipient
 */
Packet *send_and_receive(Packet* packet, char* to_address, char* port, bool verbose, char* wait_response_port)
{
	struct SendArgs args =
	{
			.packet = packet,
			.to_address = to_address,
			.port = port,
			.wait_response_port = wait_response_port
	};

	THREAD_RunAndForget(thread_send_request, (void*) &args);

	return waitForResponse(verbose, client_address, wait_response_indef, wait_response_port); // dangling pointer - stack frame finishes and could invalidate this address
}


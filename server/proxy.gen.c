if (STR_Equals(operation, "getServerDate")) {
msgpack_sbuffer response;

Packet pkt = pack_client_response_data(&response, operation, message_id, "%s", getServerDate());
if (verbose)
	unpack_data(&pkt, verbose);

send_request(&pkt, broker_address, broker_port, verbose);
msgpack_sbuffer_destroy(&response);
}

if (STR_Equals(operation, "add")) {
msgpack_sbuffer response;
int one = *(int*) params[0];
int two = *(int*) params[1];

Packet pkt = pack_client_response_data(&response, operation, message_id, "%d", add(one,two));
if (verbose)
	unpack_data(&pkt, verbose);

send_request(&pkt, broker_address, broker_port, verbose);
msgpack_sbuffer_destroy(&response);
}

if (STR_Equals(operation, "echo")) {
msgpack_sbuffer response;
char* message = (char*) params[0];

Packet pkt = pack_client_response_data(&response, operation, message_id, "%s", echo(message));
if (verbose)
	unpack_data(&pkt, verbose);

send_request(&pkt, broker_address, broker_port, verbose);
msgpack_sbuffer_destroy(&response);
}

if (STR_Equals(operation, "getBrokerName")) {
msgpack_sbuffer response;

Packet pkt = pack_client_response_data(&response, operation, message_id, "%s", getBrokerName());
if (verbose)
	unpack_data(&pkt, verbose);

send_request(&pkt, broker_address, broker_port, verbose);
msgpack_sbuffer_destroy(&response);
}

if (STR_Equals(operation, "sayHello")) {
msgpack_sbuffer response;
int age = *(int*) params[0];
char* name = (char*) params[1];

Packet pkt = pack_client_response_data(&response, operation, message_id, "%s", sayHello(age,name));
if (verbose)
	unpack_data(&pkt, verbose);

send_request(&pkt, broker_address, broker_port, verbose);
msgpack_sbuffer_destroy(&response);
}


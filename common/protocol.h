/* -----------------------------
 * Broker Protocol specification
 * ------------------------------

 The format contains multiple headers packed one after the other. 
 The order and headers determine the type of message.

headers 
    Can be any string with an associated value.
    header name bust be at most 20 characters (char[20])
    the associated value can be either an integer or a string.
    Example two headers are "name" and "age" :  
        { "name" : "Stuart" }
        { "age" : 27 }


Types of protocol messages:
---------------------------

** ServiceRegistration **

    Sent by the service to advertise its services by name and register with the broker. 

    { "request-type" : 0 }
    { "sender-address" : "127.0.0.1" }
    { "reply-port" : 8090 }
    { "services-count" : 3 }
    { "services" : ["service1", "getServerDate", "service3", ..] }
    { "service-name" : "ServiceId" }

** ServiceRequest **
    
    Sent by the client to request a service, processed by the broker and sent to approproate server to handle

   { "request-type" :0 }
   { "message-id": 3456789 }
   { "sender-address" : "127.0.0.1" }
   { "reply-port": 8090 }
   { "op"=>"getServerDate" }
   { "params" => [ buffer, length, ... ] }

** ServiceRequestReply **
    
    Sent by the service in response to a client's ServiceRequest. This is sent to the broker, who sends it to the requesting client.    

    { type:0 }
    { message-id: 3456789 }
    { "op" => "getServerDate" }
    { "reply" => [data1, data2], ... }

*/

#ifndef PROTOCOL_H
#define PROTOCOL_H
    
#define MESSAGE_ID_HDR "message-id"
#define OPERATION_HDR "op"
#define REPLY_PORT_HDR "reply-port"
#define SENDER_ADDRESS_HDR "sender-address"
#define SERVICE_NAME_HDR "service-name"
#define SERVICES_COUNT_HDR "services-count"
#define REPLY_HDR "reply"
#define REQUEST_TYPE_HDR "request-type"
#define SERVICE_PARAMS_HDR "params"
#define SERVICES_HDR "services"
#define MAX_HEADER_NAME_SIZE 20
#define MAX_PORT_CHARS 20
#define MAX_ADDRESS_CHARS 29


/**
 * @brief The types of protocol messages that exist in the specification
 * 
 */
enum RequestType {REQUEST_SERVICE, REQUEST_SERVICE_RESPONSE, REQUEST_REGISTRATION};

#endif

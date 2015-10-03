#include "broker_support.h"
#include "common.h"

/**
 * @brief Broker port
 * 
 */
extern char port[MAX_PORT_CHARS ];
/**
 * @brief Broker address
 * 
 */
extern char our_address[MAX_ADDRESS_CHARS];
/**
 * @brief true if the broker should print messages
 * 
 */
extern bool verbose_flag;
/**
 * @brief true if the broker should wait indefinitely until data arrives on its listening port
 * 
 */
extern bool waitIndef_flag;
/**
 * @brief List of servers that have registered with the broker
 * 
 */
extern struct ServiceRegistration service_repository;
/**
 * @brief List of clients that have pending requests
 * 
 */
extern struct ClientRequestRegistration client_request_repository;

/**
 * @brief Prints the contents of the services registered with this broker
 * 
 * @return void
 */
void print_service_repository()
{
    PRINT("Service registrations:\n");

    struct list_head *pos;
    list_for_each( pos, &service_repository.list)
    {
        struct ServiceRegistration* sreg_entry = list_entry( pos, struct ServiceRegistration, list );
        if( sreg_entry  == NULL )
        {
            PRINT("Found a NULL(empty) service registration entry in service repository list. Not good. Exiting!\n");
            return;
        }

        PRINT("Service Registration:\n"
                "Service name:%s\n"
                "Address: %s\n"
                "Port: %s\n"
                "Number ofservices %d",sreg_entry->service_name,
                                       sreg_entry->address,
                                       sreg_entry->port,
                                       sreg_entry->num_services);
    }
}

/**
 * @brief Prints the list of clients with requests sent to the broker. Currently not purged of old, already serviced requests
 * 
 * @return void
 */
void print_client_request_repository()
{
    PRINT("Client request registrations:\n");

    struct list_head *pos;
    list_for_each( pos, &client_request_repository.list)
    {
        struct ClientRequestRegistration* crreg_entry = list_entry( pos, struct ClientRequestRegistration, list );
        if( crreg_entry  == NULL )
        {
            PRINT("Found a NULLi(empty) client request registration entry in client request repository list. Not good. Exiting!\n");
            return;
        }

        PRINT("Client request Registration:\n"
                "Address: %s\n"
                "Port: %s\n"
                "Operation: %s\n"
                "Message ID: %d\n",crreg_entry->address,
                                       crreg_entry->port,
                                       crreg_entry->operation,
                                       crreg_entry->message_id);
    }

}


/**
 * @brief Set the brokers port number
 * 
 * @param arg The port number as a string
 * @return void
 */
void setPortNumber(char* arg)
{
    CHECK_STRING(arg, IS_NOT_EMPTY);
    CHK_ExitIf( strlen(arg) > MAX_PORT_CHARS + 1, "The length of the port you specified is larger than MAX_PORT_CHARS ","setPortNumber" );

    strncpy( port, arg, strlen(arg));
}

/**
 * @brief Set the verbose flag
 * 
 * @param arg (ignored)
 * @return void
 */
void setVerboseFlag(char* arg)
{
    verbose_flag = true;
}

/**
 * @brief Sets the wait indefinitely flag
 * 
 * @param arg (ignored)
 * @return void
 */
void setWaitIndefinitelyFlag(char* arg)
{
    waitIndef_flag = true;
}


/**
 * @brief Future call to acknowledge recept of message from client or server
 * 
 * @return void
 */
void acknowledgement()
{
    // Send a message back to sender(client or server) with general ACK
}

/**
 * @brief Sets the address that the broker will bind to (its listening port)
 * 
 * @param arg the address
 * @return void
 */
void setOurAddress(char* arg)
{
    CHECK_STRING( arg, IS_NOT_EMPTY );
    strncpy( our_address, arg, strlen(arg) );
}

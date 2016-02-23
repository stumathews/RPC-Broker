#include "broker_support.h"
#include "common.h"
#include "../config.h"

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
 * @brief Future call to acknowledge recept of message from client or server
 * 
 * @return void
 */
void acknowledgement()
{
    // Send a message back to sender(client or server) with general ACK
}



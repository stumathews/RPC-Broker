#include "broker_support.h"
#include "common.h"

extern char port[MAX_PORT_CHARS ];
extern bool verbose_flag;
extern bool waitIndef_flag;
extern struct ServiceRegistration service_repository;
extern struct ClientRequestRegistration client_request_repository;


void print_service_repository()
{
    PRINT("Service registrations:\n");

    struct list_head *pos;
    list_for_each( pos, &service_repository.list)
    {
        struct ServiceRegistration* sreg_entry = list_entry( pos, struct ServiceRegistration, list );
        if( sreg_entry  == NULL )
        {
            PRINT("Found a NULL service registration entry in servic erepository list. Exiting.!\n");
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

void print_client_request_repository()
{
    PRINT("Client request registrations:\n");

    struct list_head *pos;
    list_for_each( pos, &client_request_repository.list)
    {
        struct ClientRequestRegistration* crreg_entry = list_entry( pos, struct ClientRequestRegistration, list );
        if( crreg_entry  == NULL )
        {
            PRINT("Found a NULL client request registration entry in client request repository list. Exiting.!\n");
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


void setPortNumber(char* arg)
{
    CHECK_STRING(arg, IS_NOT_EMPTY);
    CHK_ExitIf( strlen(arg) > MAX_PORT_CHARS + 1, "The length of the port you specified is larger than MAX_PORT_CHARS ","setPortNumber" );

    strncpy( port, arg, strlen(arg));
}

void setVerboseFlag(char* arg)
{
    verbose_flag = true;
}

void setWaitIndefinitelyFlag(char* arg)
{
    waitIndef_flag = true;
}

void update_repository()
{
    
}

void acknowledgement()
{
    // Send a message back to sender(client or server) with general ACK
}

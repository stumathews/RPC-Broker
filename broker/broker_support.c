#include "broker_support.h"
#include "common.h"

extern char port[20];
extern bool verbose;
extern bool waitIndef;
extern struct ServiceRegistration service_repository;

// ===============================
// Comand line processing routines
// ===============================

void setPortNumber(char* arg)
{
    CHECK_STRING(arg, IS_NOT_EMPTY);
    strncpy( port, arg, strlen(arg));
}

void setVerbose(char* arg)
{
    verbose = true;
}

void setWaitIndefinitely(char* arg)
{
    waitIndef = true;
}


void print_service_repository()
{
    printf("Service registrations:\n");

    struct list_head *pos, *q;
    struct ServiceRegistration* tmp = Alloc( sizeof( struct ServiceRegistration ));
    int count = 0;

    list_for_each( pos, &service_repository.list)
    {
        tmp = list_entry( pos, struct ServiceRegistration, list );
        if( tmp  == NULL )
        {
            PRINT("Null service!\n");
            return;
        }

        PRINT("Service Registration:\n"
                "Service name:%s\n"
                "Address: %s\n"
                "Port: %s\n"
                "Number ofservices %d",tmp->service_name,tmp->address, tmp->port,tmp->num_services);
    }
}



void update_repository()
{
    
}

void acknowledgement()
{
    // Send a message back to sender(client or server) with general ACK
}

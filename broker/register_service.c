#include "broker_support.h"
#include "common.h"

extern char port[20];
extern bool verbose;
extern bool waitIndef;
extern struct ServiceRegistration service_repository;

// Add the service registration request to the service repository
void register_service( char* buffer,int buflen)
{
    struct ServiceRegistration *service_registration = Alloc( sizeof( struct ServiceRegistration ));
    UnpackServiceRegistrationBuffer(buffer, buflen,service_registration ); // registration request will be put into service_registration 
    
    list_add( &(service_registration->list),&(service_repository.list)); // add service registration to the repository

// Diagnostics only:

    if(verbose)
        PRINT("Registering service '%s':\n",service_registration->service_name);

    if(verbose)
    {
        for( int i = 0 ; i < service_registration->num_services;i++)
        {
            PRINT("Service %s\n", service_registration->services[i]);
        }
    }


    if( verbose )
        print_service_repository();
}

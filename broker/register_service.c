#include "broker_support.h"
#include "common.h"

extern char port[MAX_PORT_CHARS];
extern bool verbose_flag;
extern struct ServiceRegistration service_repository;

// Add the service registration request to the service repository
void register_service( char* buffer,int buflen)
{
    struct ServiceRegistration *service_registration =  UnpackServiceRegistrationBuffer(buffer, buflen );
    
    list_add( &(service_registration->list),&(service_repository.list)); // add service registration to the repository

// Diagnostics only:

    if(verbose_flag)
        PRINT("Registering service '%s':\n",service_registration->service_name);

    if(verbose_flag)
    {
        for( int i = 0 ; i < service_registration->num_services;i++)
        {
            PRINT("Service %s\n", service_registration->services[i]);
        }
    }


    if( verbose_flag )
        print_service_repository();
}

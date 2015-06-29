#include "broker_support.h"
#include "common.h"

extern char port[MAX_PORT_CHARS];
extern bool verbose_flag;
extern struct ServiceRegistration service_repository;
static void perform_diagnostics(struct ServiceRegistration* service_registration,bool verbose_flag);

void register_service( char* buffer,int buflen)
{
    struct ServiceRegistration *service_registration =  unpack_service_registration_buffer(buffer, buflen );
    
    list_add( &(service_registration->list),&(service_repository.list)); 

    perform_diagnostics(service_registration,verbose_flag);

}

static void perform_diagnostics(struct ServiceRegistration* service_registration,bool verbose_flag)
{
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

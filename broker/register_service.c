#include "broker_support.h"
#include "common.h"

extern struct ServiceRegistration service_repository;
extern bool verbose;
static void perform_diagnostics(struct ServiceRegistration* service_registration,bool verbose_flag);

void register_service_request(Packet* packet, struct BrokerConfig *brokerConfig)
{
    struct ServiceRegistration *service_registration = unpack_service_registration_buffer(packet->buffer, packet->len, brokerConfig);
    
    list_add( &(service_registration->list),&(service_repository.list)); 

    if(brokerConfig->verbose) perform_diagnostics(service_registration, brokerConfig->verbose);

}

static void perform_diagnostics(struct ServiceRegistration* service_registration, bool verbose_flag)
{

    PRINT("Registering service '%s' from host '%s':\n",service_registration->service_name, service_registration->address);


	for( int i = 0 ; i < service_registration->num_services;i++)
	{
		PRINT("Registering operation '%s'\n", service_registration->services[i]);
	}

    print_service_repository();
}

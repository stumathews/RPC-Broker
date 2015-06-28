#include "broker_support.h"
#include "common.h"

extern char port[MAX_PORT_CHARS];
extern bool verbose_flag;
extern struct ServiceRegistration service_repository;

Destination* find_server_for_request(Packet packet)
{
    char* op_name = get_op_name( packet );

    Destination *dest = Alloc( sizeof(Destination) );
    dest->address = NULL;
    dest->port = NULL;


    struct list_head *pos, *q;

    if( list_empty( &service_repository.list ))
    {
        PRINT("No services registered in broker.\n");
        return dest;;
    }

    list_for_each( pos, &service_repository.list)
    {
        ServiceReg *sreg_entry  = list_entry( pos, struct ServiceRegistration, list );

        for( int i = 0 ; i < sreg_entry->num_services;i++)
        {
            if( STR_Equals( op_name, sreg_entry->services[i]))
            {
                dest->address = sreg_entry->address;
                dest->port = sreg_entry->port;
                if( verbose_flag) { PRINT("FOUND server for required service '%s' at location '%s:%s'\n",op_name, dest->address,dest->port); }
                return dest;
            }
        }
    }
    return dest;
}

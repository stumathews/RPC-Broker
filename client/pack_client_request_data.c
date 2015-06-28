#include <msgpack.h>
#include "common.h"

extern char wait_response_port[MAX_PORT_CHARS];

char* pack_client_request_data( msgpack_sbuffer* sbuf, char* op,char* fmt, ...)
{
    msgpack_sbuffer_init(sbuf);

    msgpack_packer pk;
    msgpack_packer_init(&pk, sbuf, msgpack_sbuffer_write);

    pack_map_int("request_type",REQUEST_SERVICE,&pk);
    pack_map_int("message-id",rand(),&pk);
    pack_map_str("sender-address","localhost",&pk);
    pack_map_str("reply-port",wait_response_port,&pk);

    pack_map_str("op",op,&pk);

    msgpack_pack_map(&pk,1);
    msgpack_pack_str(&pk, 6);
    msgpack_pack_str_body(&pk, "params", 6);
    
    va_list ap;
    va_start(ap,(const char*)fmt);
    char *p, *sval;
    int ival;
    int numargs = 0;

    for( p = fmt;*p;p++)
    {
        if(*p != '%') {
            continue;
        }
        numargs++;
    }
    
    msgpack_pack_array(&pk, numargs);

    for( p = fmt;*p;p++)
    {
        if(*p != '%') {
            continue;
        }
        switch(*++p)
        {
            case 'd':
                ival = va_arg(ap,int);
                msgpack_pack_int(&pk, ival);
                break;
            case 's':
                sval =  va_arg(ap, char *);
                int sval_len = strlen(sval);
                msgpack_pack_str(&pk,sval_len);
                msgpack_pack_str_body(&pk, sval, sval_len);
                break;
        }
    }
}

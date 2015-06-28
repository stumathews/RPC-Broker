#include <msgpack.h>
#include "common.h"

extern char wait_response_port[MAX_PORT_CHARS];

char* pack_client_request_data( msgpack_sbuffer* sbuf, char* op,char* fmt, ...)
{
    msgpack_sbuffer_init(sbuf);

    msgpack_packer pk;
    msgpack_packer_init(&pk, sbuf, msgpack_sbuffer_write);

    pack_map_int(REQUEST_TYPE_HDR,REQUEST_SERVICE,&pk);
    pack_map_int(MESSAGE_ID_HDR,rand(),&pk);
    pack_map_str(SENDER_ADDRESS_HDR,"localhost",&pk);
    pack_map_str(REPLY_PORT_HDR,wait_response_port,&pk);

    pack_map_str(OPERATION_HDR,op,&pk);

    msgpack_pack_map(&pk,1);
    msgpack_pack_str(&pk, 6);
    msgpack_pack_str_body(&pk, SERVICE_PARAMS_HDR, 6);
    
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

#include <msgpack.h>
#include "common.h"

Packet pack_client_response_data( msgpack_sbuffer* sbuf, char* op, int message_id, char* fmt, ...)
{
    msgpack_sbuffer_init(sbuf);
    
    msgpack_packer pk;
    msgpack_packer_init(&pk, sbuf, msgpack_sbuffer_write);
    
    pack_map_int("request_type",REQUEST_SERVICE_RESPONSE,&pk);
    pack_map_int("message-id",message_id,&pk);

    pack_map_str("op",op,&pk);

    msgpack_pack_map(&pk,1);
    msgpack_pack_str(&pk, 5);
    msgpack_pack_str_body(&pk, "reply", 5);
    
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
    
    for( p = fmt;*p;p++)
    {
        if(*p != '%') {
            putchar(*p);
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
    Packet ret;
    ret.buffer = sbuf->data;
    ret.len = sbuf->size;
    return ret;
}

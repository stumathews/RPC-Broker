#include <msgpack.h>
#include "common.h"
#include <stdio.h>
#include <stdarg.h>

extern char wait_response_port[MAX_PORT_CHARS];
extern char client_address[MAX_ADDRESS_CHARS];

/**
 * @brief Packs a client's request for a service into a protocol message
 * 
 * @param sbuf msgpack buffer
 * @param op operation the client wants
 * @param fmt the format of the operation call
 * @param  the parameters of the operatio call
 * @return char* the protocol service request message
 */
char* pack_client_request_data( msgpack_sbuffer* sbuf, char* op,char* fmt, ...)
{
    msgpack_sbuffer_init(sbuf);

    msgpack_packer pk;
    msgpack_packer_init(&pk, sbuf, msgpack_sbuffer_write);

    pack_map_int(REQUEST_TYPE_HDR, SERVICE_REQUEST,&pk);
    pack_map_int(MESSAGE_ID_HDR,rand(),&pk);
    pack_map_str(SENDER_ADDRESS_HDR,client_address,&pk);
    pack_map_str(REPLY_PORT_HDR,wait_response_port,&pk);

    pack_map_str(OPERATION_HDR,op,&pk);

    msgpack_pack_map(&pk,1);
    msgpack_pack_str(&pk, 6);
    msgpack_pack_str_body(&pk, SERVICE_PARAMS_HDR, 6);
    
    va_list ap;
    va_start(ap,(const char*)fmt);
    char *p;
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

	char *sval = NULL;
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
                sval = va_arg(ap, char*);
                int sval_len = strlen(sval);
                msgpack_pack_str(&pk,sval_len);
                msgpack_pack_str_body(&pk, sval, sval_len);
                sval = NULL;
                break;
        }
    }
    va_end(ap);
}

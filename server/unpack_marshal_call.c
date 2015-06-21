
#include <stulibc.h>
#include "common.h"
#include "server_interface.h"

extern bool verbose;
extern char broker_address[30];
extern char broker_port[20];
// unpack the service request's operation and parameters and call server function.
void unpack_marshal_call_send( char* buffer, int buflen )
{

    msgpack_unpacked result;
    msgpack_unpack_return ret;
    size_t off = 0;
    int i = 0;
    msgpack_unpacked_init(&result);
    void** params = 0;

    ret = msgpack_unpack_next(&result, buffer, buflen, &off);
    
    while (ret == MSGPACK_UNPACK_SUCCESS) 
    {
        msgpack_object obj = result.data;
        char header_name[20];  // stores the protocol header
        char* op_name;         // stores the service request operation name

        memset(header_name, '\0', 20);
        msgpack_object val = extract_header( &obj, header_name);

        if( STR_Equals( "op", header_name) && val.type == MSGPACK_OBJECT_STR )
        {
            msgpack_object_str string = val.via.str;

            int str_len = string.size;
            char* str = Alloc( str_len);
            op_name = str;
            memset( str, '\0', str_len);
            str[str_len] = '\0';
            strncpy(str, string.ptr,str_len); 

            PRINT("%s(",str);
        } 
        else if( val.type == MSGPACK_OBJECT_ARRAY )
        {
            msgpack_object_array array = val.via.array;

            params = Alloc( sizeof(void*) * array.size);

            for( int i = 0; i < array.size;i++)
            {
                PRINT("*",i);
                msgpack_object_type type = array.ptr[i].type;
                msgpack_object param = array.ptr[i];
                params[i] = NULL;

                if( type == MSGPACK_OBJECT_STR ) //param is a char*
                {
                    int str_len = param.via.str.size;
                    char* str = Alloc( str_len);

                    memset( str, '\0', str_len);
                    str[str_len] = '\0';
                    strncpy(str, param.via.str.ptr,str_len); 

                    PRINT("char* param%d(%s),",i,str);

                    params[i] = str;
                }
                else if(type == MSGPACK_OBJECT_POSITIVE_INTEGER) //param is an int
                {
                     int64_t ival = param.via.i64;
                     int64_t *pival = Alloc( sizeof(int) );
                     *pival = ival; 

                    params[i] = pival;
                    PRINT("int param%d(%d),",i,*(int*)(params[i]));
                }

            }
            PRINT(");");

            // Now arrange for the service call to be invoked and marshal the parmeters into the function call
            // Note: This should probably be generated but I'm unsure how to do this automatically.
            // Current research has pointed me to trying to use Macros or a macro-like manguge such as M4 to generate this:
            if( STR_Equals( op_name, "echo") )
            {
                char* param0 = (char*)params[0];
                echo(param0);
                msgpack_sbuffer sbuf;
                pack_client_response_data( &sbuf, op_name, "%s", param0);
                unpack_data( sbuf.data, sbuf.size, verbose);
                send_request( sbuf.data, sbuf.size, broker_address, broker_port,verbose );
                msgpack_sbuffer_destroy(&sbuf);
            }
            else if( STR_Equals( op_name,"getBrokerName"))
            {
                char* brokerName = getBrokerName();
                PRINT("getBrokername() results in '%s'\n", brokerName);
                msgpack_sbuffer sbuf;

                pack_client_response_data( &sbuf, op_name, "%s", brokerName);
                unpack_data( sbuf.data, sbuf.size, verbose);
                send_request( sbuf.data, sbuf.size, broker_address, broker_port,verbose );
                msgpack_sbuffer_destroy(&sbuf);
                
            }
            else if( STR_Equals( op_name,"getServerDate"))
            {
                PRINT("getServerDate() result is %s", getServerDate());
                msgpack_sbuffer sbuf;
                pack_client_response_data( &sbuf, op_name, "%s", getServerDate());
                unpack_data( sbuf.data, sbuf.size, verbose);
                send_request( sbuf.data, sbuf.size, broker_address, broker_port,verbose );
                msgpack_sbuffer_destroy(&sbuf);
            }
            else if( STR_Equals( op_name,"add"))
            {
                int param0 = *(int*)params[0];
                int param1 = *(int*)params[1];
                PRINT("two in values are %d and %d\n", param0, param1);
                PRINT("The result of add() is %d\n", add(param0, param1 ));
                msgpack_sbuffer sbuf;
                pack_client_response_data( &sbuf, op_name, "%d", add(param0,param1));
                unpack_data( sbuf.data, sbuf.size, verbose);
                send_request( sbuf.data, sbuf.size, broker_address, broker_port,verbose );
                msgpack_sbuffer_destroy(&sbuf);
            }
        }

        ret = msgpack_unpack_next(&result, buffer, buflen, &off);

    } // finished unpacking.

    msgpack_unpacked_destroy(&result);

    if (ret == MSGPACK_UNPACK_PARSE_ERROR) 
    {
        printf("The data in the buf is invalid format.\n");
    }
}


#include <stulibc.h>
#include "common.h"
#include "server_interface.h"

extern bool verbose;
extern char broker_address[MAX_ADDRESS_CHARS];
extern char broker_port[MAX_PORT_CHARS];

void unpack_marshal_call_send( char* buffer, int buflen )
{

    msgpack_unpacked unpacked_result;
    msgpack_unpack_return return_status;
    size_t off = 0;
    int i = 0;
    msgpack_unpacked_init(&unpacked_result);
    void** params = 0;

    int message_id;
    return_status = msgpack_unpack_next(&unpacked_result, buffer, buflen, &off);
    
    while (return_status == MSGPACK_UNPACK_SUCCESS) 
    {
        msgpack_object obj = unpacked_result.data;
        char header_name[MAX_HEADER_NAME_SIZE];  // stores the protocol header
        char* op_name;         // stores the service request operation name

        memset(header_name, '\0', MAX_HEADER_NAME_SIZE);
        msgpack_object val = extract_header( &obj, header_name);

        // Extract the operation name to call.
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
        } // below: Get list of parameters for the operation
        else if(STR_Equals( "message_id", header_name) && val.type == MSGPACK_OBJECT_POSITIVE_INTEGER) //param is an int
        {
            int64_t ival = val.via.i64;
            message_id = ival;
            PRINT("message_id: %d,",message_id);
        }
        else if( val.type == MSGPACK_OBJECT_ARRAY )
        {
            msgpack_object_array array = val.via.array;

            params = Alloc( sizeof(void*) * array.size);

            for( int i = 0; i < array.size;i++)
            {
                msgpack_object_type param_type = array.ptr[i].type;
                msgpack_object param = array.ptr[i];

                params[i] = NULL;

                if( param_type == MSGPACK_OBJECT_STR ) //param is a char*
                {
                    int str_len = param.via.str.size;
                    char* str = Alloc( str_len);

                    memset( str, '\0', str_len);
                    str[str_len] = '\0';
                    strncpy(str, param.via.str.ptr,str_len); 

                    PRINT("char* param%d(%s),",i,str);

                    params[i] = str;
                }
                else if(param_type == MSGPACK_OBJECT_POSITIVE_INTEGER) //param is an int
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
            
                msgpack_sbuffer response;
                
                pack_client_response_data( &response, op_name, message_id, "%s", param0);
                
                if( verbose ) unpack_data( response.data, response.size, verbose);

                send_request( response.data, response.size, broker_address, broker_port,verbose );

                msgpack_sbuffer_destroy(&response);
            }
            else if( STR_Equals( op_name,"getBrokerName"))
            {
                char* brokerName = getBrokerName();
                PRINT("getBrokername() results in '%s'\n", brokerName);
                msgpack_sbuffer response;

                pack_client_response_data( &response, op_name, message_id,"%s", brokerName);

                if( verbose ) unpack_data( response.data, response.size, verbose);
                
                send_request( response.data, response.size, broker_address, broker_port,verbose );
                
                msgpack_sbuffer_destroy(&response);
                
            }
            else if( STR_Equals( op_name,"getServerDate"))
            {
                msgpack_sbuffer response;
                pack_client_response_data( &response, op_name, message_id, "%s", getServerDate());
                
                if( verbose ) unpack_data( response.data, response.size, verbose);

                send_request( response.data, response.size, broker_address, broker_port,verbose );

                msgpack_sbuffer_destroy(&response);
            }
            else if( STR_Equals( op_name,"add"))
            {
                int param0 = *(int*)params[0];
                int param1 = *(int*)params[1];
                msgpack_sbuffer response;
                
                pack_client_response_data( &response, op_name, message_id, "%d", add(param0,param1));
                
                if( verbose ) unpack_data( response.data, response.size, verbose);
                
                send_request( response.data, response.size, broker_address, broker_port,verbose );
                
                msgpack_sbuffer_destroy(&response);
            }
        }

        return_status = msgpack_unpack_next(&unpacked_result, buffer, buflen, &off);

    } // finished unpacking.

    msgpack_unpacked_destroy(&unpacked_result);

    if (return_status == MSGPACK_UNPACK_PARSE_ERROR) 
    {
        printf("The data in the buf is invalid format.\n");
    }
}

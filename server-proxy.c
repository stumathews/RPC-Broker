/* server-proxy.c
 * Acts as the main service and calls the server functions.
 * 
 * Responsibility: Abstract communication with broker away from server functions.
 * 
 * */
#include <stulibc.h>
#include "common.h"
#include "server_interface.h"

static char port[20] = {0};
static char broker_port[20] = {0};
static char broker_address[30] = {0};
static bool waitIndef = false;
static bool verbose = false;
static bool registered_with_broker = false;
static bool service_register_with_broker( char *broker_address, char* broker_port );
static void unpack_marshal_call( char* buffer, int buflen );
static void setBrokerPort( char* arg);
static void setPortNumber(char* arg);
static void setBrokerAddress(char* arg);
static void setWaitIndef(char* arg);
static void setBeVerbose(char* arg);

// == Main Server processing routine ==
// called when the server gets a connection from the broker
//
static void server( SOCKET s, struct sockaddr_in *peerp )
{

    // Wait for connections from the broker.
    struct packet pkt;
    int n_rc = netReadn( s,(char*) &pkt.len, sizeof(uint32_t));
    pkt.len = ntohl(pkt.len);

    if( verbose)
        PRINT("received %d bytes and interpreted it as length of %u\n", n_rc,pkt.len );

    if( n_rc < 1 )
        netError(1, errno,"failed to receiver packet size\n");
    
    char* dbuf = (char*) malloc( sizeof(char) * pkt.len);
    int d_rc  = netReadn( s, dbuf, sizeof( char) * pkt.len);

    if( d_rc < 1 )
        netError(1, errno,"failed to receive message\n");
    
    if(verbose)
    {
        printf("read %d bytes of data\n",d_rc);
    }
    
    unpack_marshal_call( dbuf, pkt.len );

    // TODO: pack and send response back to broker.
}

// =====================
// Server start up code
// =====================
int main( int argc, char **argv )
{
    LIB_Init();

    struct Argument* portNumber = CMD_CreateNewArgument("port","port <number>","Set the port that the server will listen on", true, true, setPortNumber);
    struct Argument* brokerAddressCMD = CMD_CreateNewArgument("baddress","baddress <address>","Set the address of the broker", true, true, setBrokerAddress);
    struct Argument* brokerPortCMD = CMD_CreateNewArgument("bport","bport <port>","Set the port of the broker", true, true, setBrokerPort);
    struct Argument* waitIndefCMD = CMD_CreateNewArgument("waitindef","","wait indefinitely for connections", false, false, setWaitIndef);
    struct Argument* beVerboseCMD = CMD_CreateNewArgument("verbose","","be verbose wtih messages", false, false, setBeVerbose);

    CMD_AddArgument(portNumber);
    CMD_AddArgument(waitIndefCMD);
    CMD_AddArgument(brokerAddressCMD);
    CMD_AddArgument(brokerPortCMD);
    CMD_AddArgument(beVerboseCMD);

    struct sockaddr_in local;
    struct sockaddr_in peer;
    char *hname;
    char *sname;
    int peerlen;
    SOCKET s1;
    SOCKET s;
    fd_set readfds;
    FD_ZERO( &readfds);
    const int on = 1;

    struct timeval timeout = {.tv_sec = 60, .tv_usec=0}; 

    if( argc > 1 )
    {
        enum ParseResult result = CMD_Parse(argc,argv,true);
        if( result != PARSE_SUCCESS )
        {
            PRINT("There was a problem parsing: %d \n", result);
            return 1;
        }
    }
    else
    {
        CMD_ShowUsages("server <options>");
        exit(0);
    }

    INIT();
    
    if( verbose ) 
        PRINT("Server listening...\n");

    // Register with the broker on startup. Currently dont wit for an ACK
    if( service_register_with_broker( broker_address, broker_port ))
        registered_with_broker = true;

    // get a socket, bound to this address thats configured to listen.
    // NB: This is always ever non-blocking 
    s = netTcpServer("localhost",port);

    FD_SET(s, &readfds);
    if(verbose) PRINT("About wait for read on port %s...\n", port);
    do
    {
        // wait/block on this listening socket...
        int res = 0;
        if( !waitIndef )
            res = select( s+1, &readfds, NULL, NULL, &timeout);
        else
            res = select( s+1, &readfds, NULL, NULL, NULL);

        if( res == 0 )
        {
            LOG( "timeout");
            netError(1,errno,"timeout!");
        }
        else if( res == -1 )
        {
            LOG("Select error!");
            netError(1,errno,"select error!!");
        }
        else
        {
            peerlen = sizeof( peer );

            if( FD_ISSET(s,&readfds ))
            {
                s1 = accept( s, ( struct sockaddr * )&peer, &peerlen );

                if ( !isvalidsock( s1 ) )
                    netError( 1, errno, "accept failed" );

                // do network functionality on this socket that now represents a connection with the peer (client) 
                server( s1, &peer );
                CLOSE( s1 );
            }
            else
            {
                DBG("not our socket. continuing");
                continue;
            }
        }

    } while ( 1 );

    LIB_Uninit();
    EXIT( 0 );
}

// ===============================
// Command line handling routines
// ===============================
static void setBrokerPort( char* arg)
{
    CHECK_STRING(arg, IS_NOT_EMPTY);
    strncpy( broker_port, arg, strlen(arg));
}

static void setPortNumber(char* arg)
{
    CHECK_STRING(arg, IS_NOT_EMPTY);
    strncpy( port, arg, strlen(arg));
}

static void setBrokerAddress(char* arg)
{
    CHECK_STRING( arg, IS_NOT_EMPTY );
    strncpy( broker_address, arg, strlen(arg) );
}

static void setWaitIndef(char* arg)
{
    waitIndef = true;
}

static void setBeVerbose(char* arg)
{
    verbose = true;
}

// unpack the service request's operation and parameters and call server function.
void unpack_marshal_call( char* buffer, int buflen  )
{

    msgpack_unpacked result;
    msgpack_unpack_return ret;
    size_t off = 0;
    int i = 0;
    msgpack_unpacked_init(&result);
    void** params = 0;

    ret = msgpack_unpack_next(&result, buffer, buflen, &off);
    
    // Loop through protocol data and unpack and store it as we go:
    
    // The protcol specifies that the operation name will be followed by an array of parameters
    while (ret == MSGPACK_UNPACK_SUCCESS) 
    {
        msgpack_object obj = result.data;
        
        char header_name[20];  // stores the protocol header
        char* op_name;         // stores the service request operation name

        memset(header_name, '\0', 20);
        msgpack_object val = extract_header( &obj, header_name);

        // Extract Service request(operation) name:
        if( STR_Equals( "op", header_name) && val.type == MSGPACK_OBJECT_STR )
        {
            msgpack_object_str string = val.via.str;

            // Extract string:
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
            // Extract the service requsts's(operation's) parameters
            msgpack_object_array array = val.via.array;

            // we'll store the parameters (of variable type) in this void pointer array and we'll use this as our input for 
            // marshaling it into the actual service/operation C call
            params = Alloc( sizeof(void*) * array.size);

            for( int i = 0; i < array.size;i++)
            {
                msgpack_object_type type = array.ptr[i].type;
                msgpack_object param = array.ptr[i];

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
                    int ival = param.via.i64;
                    int *pival = Alloc( sizeof(int) );
                    
                    PRINT("int param%d(%d),",i,*pival);
                    
                    params[i] = pival;
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
            }
            else if( STR_Equals( op_name,"getBrokerName"))
            {
                char* brokerName = getBrokerName();
                PRINT("getBrokername() results in '%s'\n", brokerName);
            }
            else if( STR_Equals( op_name,"getServerDate"))
            {
                PRINT("Dont yet know how to marshal and call getServerDate() yet.\n");
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

// =====================
// SERVICE Registration
// =====================
// Craft a service registration message and send it of fto the broker.
bool service_register_with_broker( char *broker_address, char* broker_port )
{
    ServiceReg *sr = Alloc( sizeof( ServiceReg ) );
    sr->address = "localhost"; // TODO: Get our actual IP address
    sr->port = port;
    
    msgpack_sbuffer sbuf;
    msgpack_sbuffer_init(&sbuf);
    msgpack_packer pk;
    msgpack_packer_init(&pk, &sbuf, msgpack_sbuffer_write);

    pack_map_int("request_type",REQUEST_REGISTRATION,&pk);
    pack_map_str("sender-address",sr->address,&pk);
    pack_map_str("reply-port",sr->port,&pk);
    pack_map_str("service-name","theServiceName",&pk);

    // pull in the services defined in the server code: server.c
    extern char* services[];
    char* service = services[0];
    int i = 0;
    while( services[i] != NULL )
    {
        if(verbose)
            PRINT("Service %s.\n", services[i]);
        i++;
    }
    pack_map_int("services-count",i,&pk);
    msgpack_pack_map(&pk,1);
    msgpack_pack_str(&pk, 8);
    msgpack_pack_str_body(&pk, "services", 8);
    msgpack_pack_array(&pk, i);

    if( verbose )
        PRINT("num services %d\n",i);

    // pack the services into the protocol message
    while( i >= 0 )
    {
        if( !STR_IsNullOrEmpty(services[i] ))
        {
            if(verbose)
                PRINT("service packed is %s\n", services[i]);
            msgpack_pack_str(&pk, strlen(services[i]));
            msgpack_pack_str_body(&pk, services[i], strlen(services[i]));
        }
        i--;    
    }
    // send registration message to broker
    send_request( sbuf.data, sbuf.size, broker_address, broker_port,verbose);
    msgpack_sbuffer_destroy(&sbuf);
}

#include "server_interface.h"

int main( int argc, char* argv[])
{
    char strServerDate[80];
    // Call the client-proxy's getServerDate();
     getServerDate(strServerDate,80);
}

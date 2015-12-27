#!/usr/bin/perl -w

use strict;

my $beginning = << "END";
#include \"server_interface.h\"
#include \"common.h\"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <msgpack.h>
#include <stulibc.h>
#include <stdarg.h>

extern char broker_address[MAX_ADDRESS_CHARS];
extern char broker_port[MAX_PORT_CHARS];
extern bool verbose;
extern char wait_response_port[MAX_PORT_CHARS];
END
print $beginning;
open( interfaceHandle , 'server_interface.h');

while( <interfaceHandle>) {
	if (/^([\w*]+)\s+(\w+)\s*\((.*)\);$/){
		my $fnRet = $1;
		my $fnName = $2;
		my $fnParams = $3;			
		my $format = '';		
		my @params = split(' ',$fnParams);
				
		my @paramNames = '';
		my @paramTypes = '';
		my $paramCount = 0;
		
		for( my $i = 0; $i < scalar @params ; $i++ ) {				
			if( $i % 2 == 0 ) {				
				if( $params[$i] eq 'char*' ) {
					$format .= '%s';					
				}
				if ($params[$i] eq 'int') {
					$format .= '%d';					
				}
				$paramTypes[$paramCount] = $params[$i];	
				$paramCount++;				
			}else {						
				$paramNames[$paramCount] = $params[$i];		
				$paramNames[$paramCount] =~ s/,//g;		
			}
		}
		
		
		@paramNames = grep { $_ ne '' } @paramNames;
		my $retStatement = '';
		if( $fnRet eq "int" ){
			$retStatement = "return  get_header_int_value(result, REPLY_HDR);";
		}
		if( $fnRet eq "char*" ){
			$retStatement = "return  get_header_str_value(result, REPLY_HDR);";
		}
		
		
		
		my $paramGets = '';
		for( my $i = 0 ; $i < $paramCount; $i++ ){
			if(  $paramTypes[$i] eq "int" ){
				$paramGets .= "\t\t$paramTypes[$i] $paramNames[$i] = *(".$paramTypes[$i]."*) params[".$i."];\n";
			} else {
				$paramGets .= "\t\t$paramTypes[$i] $paramNames[$i] = (".$paramTypes[$i].") params[".$i."];\n";
			}
		}	
		
		my $commaParams = '';
		if( scalar @paramNames > 1) { 			
			$commaParams = ','.join(',',@paramNames); 
		} else {
			if (defined $paramNames[0]  ){
			$commaParams = ",$paramNames[0]";
			} else {
			$commaParams = "";
			} 
		}

		my $output = << "END";
$fnRet $fnName( $fnParams )
{

    msgpack_sbuffer sbuf;

    pack_client_request_data( &sbuf, (char*)__func__, \"$format\"$commaParams);

    Packet pkt; pkt.buffer = sbuf.data; pkt.len = sbuf.size;

    Packet *result = send_and_receive( &pkt, broker_address, broker_port, verbose, wait_response_port );

    msgpack_sbuffer_destroy(&sbuf);

    $retStatement
}
END
print "$output\n";

	}
}


close(interfaceHandle);


#!/usr/bin/perl -w

use strict;

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
		
		my $paramGets = '';
		for( my $i = 0 ; $i < $paramCount; $i++ ){
			if(  $paramTypes[$i] eq "int" ){
				$paramGets .= "$paramTypes[$i] $paramNames[$i] = *(".$paramTypes[$i]."*) params[".$i."];\n";
			} else {
				$paramGets .= "$paramTypes[$i] $paramNames[$i] = (".$paramTypes[$i].") params[".$i."];\n";
			}
		}	
		
		my $fnCall = "$fnName(".join(',',@paramNames).")";		
		my $output = << "END";
if (STR_Equals(operation, \"$fnName\")) {
msgpack_sbuffer response;
$paramGets
Packet pkt = pack_client_response_data(&response, operation, message_id, \"$format\", $fnCall);
if (verbose)
	unpack_data(&pkt, verbose);

send_request(&pkt, broker_address, broker_port, verbose);
msgpack_sbuffer_destroy(&response);
}
END
print "$output\n";
	}
}


close(interfaceHandle);


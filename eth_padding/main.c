#include "../files/eth.h"
#include "../files/util.h"
#include <rawnet.h>
#include <timerms.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>

int main() {

	bold("This sends and empty payload, length should be 64\n");
	printf("Opening eth0\n");

	eth_iface_t * iface = eth_open( "eth0" );

	printf("Sending empty payload (1 byte) to 00:11:22:33:44:55\n");
	mac_addr_t dst;
	mac_str_addr("00:11:22:33:44:55", dst);
	uint8_t byte = 1;

	
	printf("With type IPV4_OVER_ETH_TYPE\n");
	
	int bytes_sent = eth_send( iface, dst, 0x0800, &byte, 1 );

	printf(BOLDCYAN "bytes_sent = %d\n" RESET, bytes_sent); 



	bold("Now we try to send 128 bits, for exmple: \n");
	printf(" Now we try 128 bytes to 00:11:22:33:44:55\n");

	int i;
	unsigned char word[128];
	for( i=0; i < 128; i++ ) word[i] = i;
		
	bytes_sent = eth_send( iface, dst, 0x0800, word, 128 );

	printf(BOLDCYAN "bytes_sent = %d\n" RESET, bytes_sent); 

	return 0;
}
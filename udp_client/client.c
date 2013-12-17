#include "../files/ipv4.h"
#include "../files/udp.h"
#include "../files/ipv4_config.h"
#include "../files/ipv4_route_table.h"
#include "../files/eth.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <timerms.h>

int main(int argc , char *argv[]) {
	

	 /*------------------VARIABLE DECLARATION-----------------------------------*/

	  //Destination IPv4 address (when sending)
	ipv4_addr_t dst;
	ipv4_addr_t src;

        //Protocol
	int port;

          //Send error checking
	int send_flag;
	
	
	/*-------------------ARGUMENT MANIPULATION----------------------------------*/
	
	if(argc != 3) {

		printf("Usage: ip_client <port> <ip_address>\n");
		return -1;
	}
	
	//We convert the string enterned as an argument to an ipv4_addr_t type
	port = atoi(argv[1]);
	ipv4_str_addr(argv[2], dst);

	
	/*------------------------OPENING---------------------------------------------*/

	int flag = udp_open( port );
	
	if ( flag == -1 ) {

		print_alert("Unable to open port\n");
		return -1;

	} else {

		printf("Route table loaded, PORT= %d\n", flag);
	}	
	
	/*--------------------SENDING-------------------------------------*/
	
	//Message we want to send to the server
	char payload[140];
	bold( "\nWrite the message you want to send:" );
	printf("\n>");
	scanf("%s", payload);
	printf("(Sending \"%s\")\n", payload);
	
	//We send the message
	send_flag = udp_send(dst, port, (unsigned char *) payload, strlen(payload));
	
	if (send_flag == -1) {

		return -1;
	} 
	
	/*-------------------RECEIVING---------------------------------------*/
	//The message we sent is echoed back
	
	//Maximum time to wait when waiting for a packet to be recieved. Measured in miliseconds.
	long int timeout = 3000;

	//Buffer to store the payload of the received packet.
	unsigned char data [ETH_MTU - HED_LEN];
	
	//Bytes received from the received packet
	int bytes_received = 0;

	bold("Waiting for an answer...\n");
	//START WHILE LOOP: filters the unwanted packets
	while (bytes_received == 0){

	//We wait to receive a packet
		bytes_received = udp_recv(src, data, timeout);


	}
	////END WHILE LOOP

	if (bytes_received == 0){

	 print_alert ("\nmain: End of loop!\n");
	 return -1;
	}
	//else{
	////START PRINTING DATA);
	printf ("\n\n--------------------------------------------------------------\n\n");
	printf (">%s<", data);
	printf ("\n\n--------------------------------------------------------------\n\n");
	////END PRINTING DATA
	//}

	/*-----------------------CLOSING------------------------------------------*/

	udp_close();

	return 0;	
}

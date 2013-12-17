#include "../files/ipv4.h"
#include "../files/udp.h"
#include "../files/ipv4_config.h"
#include "../files/ipv4_route_table.h"
#include "../files/eth.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <timerms.h>
#include <signal.h>

int RUN = 1;

void interrupt() {
	RUN = 0;
	udp_close();
	printf("\nCaptured CTRL-C. Going down NOW!\n");
	exit(0);
}

int main(int argc, char *argv[]) {

	/*------------------VARIABLE DECLARATION-----------------------------------*/

	if(argc != 2) {

		printf("Usage: server <port>\n");
		return -1;
	}

	int port = atoi(argv[1]);

	/*------------------------OPENING---------------------------------------------*/
	
	int flag = udp_open( port );
	
	if ( flag == -1 ) {

		print_alert("Unable to open the port\n");
		return -1;

	} else {

		bold("Se ha cargado la tabla de rutas\n");
	}	
	
	/*-------------------RECEIVING---------------------------------------*/

	/*
	* Esta sentancia captura CTRL+C
	*/
	signal(SIGINT, interrupt);

	bold("Server started\n");
	
	/* 
	*	START WHILE LOOP:
	*   filters the unwanted packets
	*/

	while ( RUN ) {

		//Source IPv4 address (the address of the client from whom we recevied a packet)
		ipv4_addr_t src;

		//Array where we store the string that represents the source IPv4 address.
		char ipv4_str_src [IPv4_STR_SIZE];

		//Buffer to store the contents of the received packet.
		unsigned char data [ETH_MTU - HED_LEN];

		//We wait to receive a packet  
		int bytes_received = udp_recv( src, data, -1);
		print_success("We received something\n");

		if( bytes_received == -1 ) {

			print_alert("Error while trying to read a packet\n");
			printf("main:\n\t while(run) => bytes_received == -1 \n");

		} else if ( bytes_received == 0 ) {

			/* Se ha producido un timeout 
			* No hacemos nada (el servidor sigue funcionando)
			*/
			

		} else if( bytes_received > 0 ) {

		//We convert the source IPv4 address into a string for comparing purposes
			ipv4_addr_str (src, ipv4_str_src);

		////START PRINTING DATA
			printf ("\n\n--------------------------------------------------------------\n\n");
			printf (">recieved %d BYTES:/> %s FROM: %s", bytes_received, data, ipv4_str_src);
			printf ("\n\n--------------------------------------------------------------\n\n");
		////END PRINTING DATA

		/*--------------------ECO-------------------------------------*/
			int bytes_sent = udp_send( src, port, data, bytes_received );
			if( bytes_sent == -1 ) {

				print_alert( "Ha habido un error haciendo eco.\n");
				return -1;

			} else {

				printf("Se han enviado %d bytes al origen.\n", bytes_sent);
			}
		}

	} /* end of while loop */
		
	udp_close();
	return 0;
}

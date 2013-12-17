#include "udp.h"

uint16_t port = -1; //puerto cerrado

int udp_open( uint16_t p ) {

	port = p;
	//printf ("PORT: %d\n", p);
	int flag = ipv4_open();
	if(flag == -1) 
		return -1;
	else 
		return port;
}

void udp_close( ) {

	port = -1;	//puerto cerrado
	ipv4_close();
	return;
}

/*
* Devuelve num de bytes, 0 si timeout, -1 si error
*/
int udp_recv( ipv4_addr_t src, unsigned char * buffer, long int timeout, int * src_port) {

	unsigned char temp [ETH_MTU];
	udp_datagram_t data;

	/* variables timer */
	timerms_t timer;
	timerms_reset(&timer, timeout);
	long int countdown;

	/* guardian, a 0 para salir del bucle */
	int not_received = 1;

	while ( not_received ) {
		//vemos el tiempo que queda
		countdown = timerms_left(&timer); 

		int bytes_recv = ipv4_recv(src, UDP_OVER_IP_PROT, temp, countdown);
		
		if( bytes_recv > 0) {
		
  			//copiamos a buffer tempporal
			memcpy(&data, temp, bytes_recv);
			
			
			if(ntohs(data.header.port_dst) == port) {
  			//si es el mismo puerto lo devolvemos al nivel superior
				not_received = 0; //se ha recibido
				memcpy(buffer, temp + UDP_HDR_LEN, bytes_recv - UDP_HDR_LEN);
				*src_port = ntohs(data.header.port_src);
				return bytes_recv - UDP_HDR_LEN;

			} else {
				//el puerto esta mal, bytes_recv a 0 de nuevo
				
				bytes_recv = 0;
			}

		} else if( bytes_recv == 0 ) {
  			//ha habido timeout
  			//volvemos a iterar
			return 0;

		} else if( bytes_recv < 0 ){
  			//ha habido error
			print_alert("udp.c: udp_rcv() => bytes_recv < 0\n");
			return -1;
		}
	}

	//si sigue iterando hasta que acabe el tiempo es que no ha recibido nada
	//y devolvemos 0
	return 0;
}

int udp_send( ipv4_addr_t dst, uint16_t dst_port, unsigned char * payload, int len ) {

	//primero creamos el paquete udp
	//datagrama estatico
	udp_datagram_t udp_pkt;
	udp_pkt.header.port_src = htons(port);
	udp_pkt.header.port_dst = htons(dst_port);
	udp_pkt.header.udp_len = htons (len + UDP_HDR_LEN);
	udp_pkt.header.checksum = 0;
	udp_pkt.header.checksum =  htons( udp_checksum( payload, len ));
	
	/*
	REMINDER: cabecera UDP
	struct udp_header_t {
		uint16_t port_src;
		uint16_t port_dst;
		uint16_t udp_len;
		uint16_t checksum;
	};
	*/ 

	unsigned char data[ ntohs (udp_pkt.header.udp_len) ];
	
	
	//copiamos la info a enviar en un array
	memcpy( data, &udp_pkt, UDP_HDR_LEN );
	memcpy( data + UDP_HDR_LEN, payload, len);

	int bytes_sent = ipv4_send( dst, UDP_OVER_IP_PROT, data, ntohs (udp_pkt.header.udp_len));
	return bytes_sent;
}

 uint16_t udp_checksum ( unsigned char * data, int len )
 {

  int i;
  uint16_t word16;
  unsigned int sum = 0;

  /*
  * Make 16 bit words out of every two adjacent 
  * 8 bit words in the packet
  * and add them up 
  */
  for (i=0; i<len; i=i+2) {
    word16 = ((data[i] << 8) & 0xFF00) + (data[i+1] & 0x00FF);
    sum = sum + (unsigned int) word16;	
  }

  /* Take only 16 bits out of the 32 bit sum and add up the carries */
  while (sum >> 16) {
    sum = (sum & 0xFFFF) + (sum >> 16);
  }

  /* One's complement the result */
  sum = ~sum;

  return (uint16_t) sum;
}



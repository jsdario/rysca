#include "ipv4.h"
#include "ipv4_config.h"
#include "ipv4_route_table.h"
#include "eth.h"
#include "arp.h"
#include "util.h"

#include <timerms.h>
#include <stdio.h>
#include <stdlib.h>

#define CONFIG_TXT "../files/ipv4_config.txt"
#define ROUTES_TXT "../files/ipv4_routes.txt"
#define ETH_MTU 1500

/* ip multicast estructura y lista */
int multicast_entries = 0;

//array de 50 elementos
multicast_resource multicast_list[_CACHE_SIZE];


/* Global scope variable */

ipv4_route_table_t * route_table_ptr; //Root of route table
eth_iface_t * iface_handler;          //Handles the interface
ipv4_addr_t ipv4_hostaddr;            //Host IPv4 address
ipv4_addr_t ipv4_netmask;             //Host IPv4 mask

/* int ipv4_open
* ( )
* 
* 
* DESCRIPTION: 
*   This function opens the interface specified in the config file. 
*   It also initializes the route table pointer.  
*
*
* PARAMETERS:
*   'route_table_txt': Name of the file in which the route table is stored.
*        'config_txt': Name of the config file.
*
*
* RETURNED VALUE:
*   Returns 1 if everything has beed loaded correctly
* 
* ERROR:
*   Returns -1 if there's an error
*   
*/
int ipv4_open() {

  if(multicast_entries == 0) {
    print_notice("Opened multicast list for subscribers \n");
    memset(multicast_list, 0, sizeof(multicast_resource)*_CACHE_SIZE);
  }

  //We create an empty table
  route_table_ptr = ipv4_route_table_create ();

   //We initialize the route table pointer (declared as a global variable) by reading the file
   //in which the route table is stored
  int routes_read = ipv4_route_table_read ( ROUTES_TXT , route_table_ptr );

   ////START ROUTE TABLE CHECK: checking if the route table we have just read is empty or non-existant
  if( routes_read <= 0  || route_table_ptr == NULL ) {

    print_alert("Could not load the route table.\n");
    return -1;
  } /*else {

    printf("%d route/s read\n", routes_read );
    
    ipv4_route_table_print ( route_table_ptr );
    
  }*/
  ////END ROUTE TABLE CHECK

  //Array in which the interface name is stored
    char ifname[IFACE_NAME_LENGTH]; 

  //We read the config file
    int flag =  ipv4_config_read ( CONFIG_TXT, ifname, ipv4_hostaddr, ipv4_netmask );

    if( flag == -1 ) {

      print_alert("ipv4_open() @ ipv4.c : Could not read the config file\n");
      return -1;
    }


  //We initialize the interface handler (set as a global variable)
    iface_handler = eth_open( ifname );
    if ( iface_handler == NULL ) {

      print_alert("Could not open the interface\n");
      printf("\t ipv4.c: ipv4_open(): iface_handler = NULL \n");
      return -1;
    }

    return 1;

  }

/* int ipv4_send
 * ( ipv4_addr_t dst, uint8_t prot, unsigned char * payload, int payload_len  )
 * 
 * 
 * DESCRIPTION: 
 *   This function sends a packet over Ethernet given a destination IPv4 address  
 *
 *
 * PARAMETERS:
 *           'dst': Destination IPv4 address.
 *          'prot': Type of protocol.
 *       'payload': The data we wish to send.
 *   'payload_len': Length of the data we wish to send.
 *
 *
 * RETURNED VALUE:
 *   Returns -1 if there's an error
 * 
 * ERROR:
 *   Returns -1 if there's an error
 *   
 */
 int ipv4_send( ipv4_addr_t dst, uint16_t prot, unsigned char * payload, int payload_len  ) {

  //Destination MAC address
  mac_addr_t mac_addr_dst; 
  
  //Next hop destination IPv4 address
  ipv4_addr_t next_jump;
  
  char dest_addr_str [IPv4_STR_SIZE]; //cadena ipv4 del destino
  char next_addr_str [IPv4_STR_SIZE]; //cadena ipv4 del destino
  ipv4_addr_str (dst, dest_addr_str);
  
  //Array to contain the destination address in a string format
  char ipv4_str_dst [IPv4_STR_SIZE];
  
   //Convert to string (destination)
  ipv4_addr_str (dst, ipv4_str_dst);
  
  int is_multicast = 0;
  //WE CHECK IF DESTINATION IS MULTICAST
  if (strcmp (ipv4_str_dst, "224.0.0.9") == 0) is_multicast = 1;
  
   //Pointer to individual routes from the route table
  ipv4_route_t * route_ptr = NULL;
  //We check the route table for the best route to reach the specified destination IPv4 address
  if (!is_multicast){
    route_ptr = ipv4_route_table_lookup( route_table_ptr, dst ); 
    //underline("Destination: "); underline( dest_addr_str ); printf(" -> ");
    ipv4_addr_str (route_ptr->gateway_addr, next_addr_str);
    //underline("Next jump "); underline( next_addr_str ); printf("\n");

    if (route_ptr == NULL){

      print_alert ("\nNo routes found\n");
      printf ("\tipv4_route_table.c : ipv4_route_table_lookup() = null \n");
      return -1;
    }

  //We set next hop address as the gateway address
    memcpy( next_jump, route_ptr->gateway_addr, IPv4_ADDR_SIZE ); 
  }

  ////START CHECKING NEXT HOP: checking if the gateway address is "0.0.0.0"
  //Array to contain the address "0.0.0.0" in a string format
  char ipv4_str_empty [IPv4_STR_SIZE];

  //Array to contain the gateway address in a string format
  char ipv4_str_gw [IPv4_STR_SIZE];

  //Convert to string ("0.0.0.0")
  ipv4_addr_str (IPv4_ZERO_ADDR, ipv4_str_empty);

  //Convert to string (gateway)
  ipv4_addr_str (next_jump, ipv4_str_gw);

  //Compare both strings to see if the gateway address is "0.0.0.0"
  if (strcmp (ipv4_str_empty, ipv4_str_gw) == 0)
   memcpy( next_jump, dst, IPv4_ADDR_SIZE ); 
  ////END CHECKING NEXT HOP

  ////START CHECKING OWN IP: checking if we are sending to our own IP. If not, do arp resolve

  //Array to contain the host address in a string format
 char ipv4_str_host [IPv4_STR_SIZE];

  //Convert to string (host)
 ipv4_addr_str (ipv4_hostaddr, ipv4_str_host);

  //Variable to check if there has been an error when calling the arp_resolve function
 int flag = 0;

 /*
 * START COMPARE: Compare both strings. If they are the same, 
 * get the mac address of our own interface. If 
 * they are not the same, do arp resolve, or if multicast!
 */

 if ( ip_subscribed_to( dst ) ){ 

  if (is_multicast){

    mac_str_addr("01:00:5E:00:00:09", mac_addr_dst);
    //printf ("It is a multicast address\n"); //this should be only for verbose

  } else {

    eth_getaddr (iface_handler, mac_addr_dst);
    //printf ("It is our own ip address\n"); //this should be only for verbose
  }

} else {

  flag = arp_resolve( iface_handler, ipv4_hostaddr, next_jump, mac_addr_dst );
}

if( flag == -1 ){
  print_alert("ipv4.c : ipv4_send(): Could not resolve MAC adress\n");
  return -1;
}

  /* This is team six and this is jackass */

  ////START INITIALIZE HEADER: we initialize ip_header with standard values and values received as parameters
  //New header
ip_header header;

  //Header fields
  header.ver_and_ihl = 0x45; //IPv4 AND HEADER LENGTH 5
  header.ip_tos = 0; 
  header.ip_pkg_len = htons(HED_LEN + payload_len); //HEADER + PAYLOAD
  header.ip_pkg_id = htons(5038); //ANY
  header.ip_offset = 0; 
  header.ip_ttl = 64; // # of hopes
  if(is_multicast) header.ip_ttl = 1; //if multicast f** this bastard ASAP
  header.ip_prot = prot; //icmp control
  
  //We set the source IPv4 address field
  memcpy(header.ip_src, ipv4_hostaddr, IPv4_ADDR_SIZE );
  
  //We set the destination IPv4 address field
  memcpy(header.ip_dst, dst, IPv4_ADDR_SIZE );
  
  //First 0, then we do ipv4_checksum and then we set checksum field with new value
  header.ip_checksum = 0; 
  
  ////START HOST TO NETWORK FUNCTIONALITY (for understanding purposes)
  //##### 0xABCD to 0xCDAB (for example) #####//
  uint16_t checksum_calc = ipv4_checksum ((unsigned char *) (&header), HED_LEN);
  header.ip_checksum = htons(checksum_calc);
  
  //Bytes sent over Ethernet
  int bytes_sent = 0;
  
  //Array to store header data
  unsigned char eth_header [HED_LEN];
  
  //We copy the data to the eth_header array
  memcpy(eth_header, &header, HED_LEN);
  
  //Array to store payload
  unsigned char eth_payload [HED_LEN+payload_len]; //SET MACRO FOR HEADER LENGTH!!!!

  //We store the eth_header data into the eth_payload
  memcpy(eth_payload, eth_header, HED_LEN);
  
  //We store the payload data into the eth_payload
  memcpy (eth_payload+HED_LEN, payload, payload_len);

  //We send the data
  bytes_sent = eth_send (iface_handler, mac_addr_dst, IPV4_OVER_ETH_TYPE, eth_payload, HED_LEN+payload_len);
  
  if( bytes_sent < 0 ) {

    print_alert ("Error while sending packet\n");
    printf("\tarp.c: ipv4_send() => bytes_sent < 0 \n");
    return -1;

  } else if ( bytes_sent == 0 ) {

    print_alert ("Unable to send ipv4 packet \n");
    printf("\tipv4.c: ipv4_send() => bytes_sent = 0 \n");
    return 0;
  }

  return bytes_sent;
}

/* int ipv4_recv
 * ( ipv4_addr_t src, uint8_t prot, unsigned char buffer[], long int timeout )
 * 
 * 
 * DESCRIPTION: 
 *   This function sends a packet over Ethernet given a destination IPv4 address  
 *
 *
 * PARAMETERS:
 *           'src': OUTPUT PARAMETER: stores the source IPv4 address.
 *          'prot': Type of protocol.
 *       'payload': OUTPUT PARAMETER: stores the data we have received (without header).
 *   	 'timeout': Maximum time to wait when waiting for a packet to be recieved. Measured in miliseconds.
 *
 *
 * RETURNED VALUE:
 *   Returns bytes recieved WITHOUT header bytes (Payload bytes ONLY).
 * 
 * ERROR:
 *   Returns -1 if there's an error
 *   
 */
 int ipv4_recv( ipv4_addr_t src, uint16_t prot, unsigned char payload[], long int timeout ) {


  timerms_t timer;
  timerms_reset(&timer, timeout);

  //Source MAC address
  mac_addr_t mac_addr_src;
  
  //We create an empty ip datagram
  ip_datagram_ptr datagram_ptr = (ip_datagram_ptr) malloc (ETH_MTU);
  
  //Bytes received from the received packet
  int bytes_received = 0;


  unsigned char eth_buffer [ETH_MTU];
  
  //Array to contain the host address in a string format
  char ipv4_str_host [IPv4_STR_SIZE];
  
  //Convert to string (host)
  ipv4_addr_str (ipv4_hostaddr, ipv4_str_host);

  long int countdown;

  while (bytes_received == 0){	

   countdown = timerms_left(&timer);

    //We wait to receive a packet
   bytes_received = eth_recv (iface_handler, mac_addr_src, IPV4_OVER_ETH_TYPE, eth_buffer, countdown);
   
   
   if (bytes_received <= -1) {
   /* Error leyendo paquetes, dejo que imprima el nivel superior */ 
    print_alert("ipv4.c: ipv4_rcv() => bytes_recv < 0\n"); 
    free (datagram_ptr); 
    return -1;

  } else if (bytes_received == 0)  { 

   free (datagram_ptr); 
   return 0; 
 }

 memcpy(datagram_ptr, eth_buffer, bytes_received);


/* Comparamos si somos destino, si el checksum esta bien y si el protocolo es correcto */
 if ( !ip_subscribed_to( datagram_ptr->header.ip_dst )
  ||  datagram_ptr->header.ip_prot != prot
  ||  ipv4_checksum ((unsigned char*) datagram_ptr, HED_LEN) != 0){
	/* queremos iterar otra vez */
  bytes_received = 0;

}
}

  // We copy the contents of the received data to the ip datagram pointed by datagram_ptr
memcpy (payload, datagram_ptr->data, ETH_MTU - HED_LEN);

  // We copy the source IPv4 address from the ip datagram to src, an output parameter.
memcpy (src, datagram_ptr->header.ip_src, IPv4_ADDR_SIZE);

//print_ip_header(datagram_ptr->header);

free (datagram_ptr);

return ntohs(datagram_ptr->header.ip_pkg_len) - HED_LEN;

}

/* void ipv4_close(void)
 *  
 * DESCRIPTION: 
 *   This function closes the interface 
 * @return nothing
 *   
 */
 void ipv4_close() {

  //We close the interface
  eth_close( iface_handler );
  
  //we also unsubscribe for all multicast ip in this RAM 
  int i;
  for( i = 0 ; i < _CACHE_SIZE; i++) {
    leave_multicast( &multicast_list[i] );
  }

  //We free the memory allocated for the route table
  ipv4_route_table_free( route_table_ptr );
}

/*
 * uint16_t ipv4_checksum ( unsigned char * data, int len )
 *
 * DESCRIPCIÓN:
 *   Esta función calcula el checksum IP de los datos especificados.
 *
 * PARÁMETROS:
 *   'data': Puntero a los datos sobre los que se calcula el checksum.
 *    'len': Longitud en bytes de los datos.
 *
 * VALOR DEVUELTO:
 *   El valor del checksum calculado.
 */

 uint16_t ipv4_checksum ( unsigned char * data, int len ) {

  int i;
  uint16_t word16;
  unsigned int sum = 0;

  /* Make 16 bit words out of every two adjacent 8 bit words in the packet
   * and add them up */
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
  uint16_t test = (uint16_t) sum;
  return test;
}


/* void ipv4_addr_str ( ipv4_addr_t addr, char* str );
 *
 * DESCRIPCIÓN:
 *   Esta función genera una cadena de texto que representa la dirección IPv4
 *   indicada.
 *
 * PARÁMETROS:
 *   'addr': La dirección IP que se quiere representar textualente.
 *    'str': Memoria donde se desea almacenar la cadena de texto generada.
 *           Deben reservarse al menos 'IPv4_STR_SIZE' bytes.
 */
 void ipv4_addr_str ( ipv4_addr_t addr, char* str )
 {
  if (str != NULL) {
    sprintf(str, "%d.%d.%d.%d",
      addr[0], addr[1], addr[2], addr[3]);
  }
}

/* int ipv4_str_addr ( char* str, ipv4_addr_t addr );
 *
 * DESCRIPCIÓN:
 *   Esta función analiza una cadena de texto en busca de una dirección IPv4.
 *
 * PARÁMETROS:
 *    'str': La cadena de texto que se desea procesar.
 *   'addr': Memoria donde se almacena la dirección IPv4 encontrada.
 *
 * VALOR DEVUELTO:
 *   Se devuelve 0 si la cadena de texto representaba una dirección IPv4.
 *
 * ERRORES:
 *   La función devuelve -1 si la cadena de texto no representaba una
 *   dirección IPv4.
 */
 int ipv4_str_addr ( char* str, ipv4_addr_t addr )
 {
  int err = -1;

  if (str != NULL) {
    unsigned int addr_int[IPv4_ADDR_SIZE];
    int len = sscanf(str, "%d.%d.%d.%d", 
     &addr_int[0], &addr_int[1], 
     &addr_int[2], &addr_int[3]);

    if (len == IPv4_ADDR_SIZE) {
      int i;
      for (i=0; i<IPv4_ADDR_SIZE; i++) {
        addr[i] = (unsigned char) addr_int[i];
      }
      
      err = 0;
    }
  }
  
  return err;
}

/* void print_ip_header( ip_header header );
 *
 * DESCRIPCIÓN:
 *   Imprime en la consola el header del paquete formateado.
 *
 * PARÁMETROS:
 *    'header' : El header que se va a imprimir 
 *
 * VALOR DEVUELTO:
 *   Ninguno.
 *
 * ERRORES:
 *   Ninguno.
 */
 void print_ip_header( ip_header header ) {

  underline("\t\tIPv4 Header\t\t\n");
  printf("-----------------------------------------\n");
  printf("  %u \t | \t", header.ver_and_ihl );
  printf("%u \t | \t", header.ip_tos );
  printf("%d  ", header.ip_pkg_len );
  printf("\n ----------------------------------------\n");
  printf("  %d \t | \t", header.ip_pkg_id );
  printf("%d  \t", header.ip_offset );
  printf("\n ----------------------------------------\n");
  printf("  %u \t | \t", header.ip_ttl );
  printf("%u \t | \t", header.ip_prot );
  printf("%d  ", header.ip_checksum );
  printf("\n ----------------------------------------\n");
  char src[IPv4_STR_SIZE];
  ipv4_addr_str( header.ip_src, src );
  printf("\t    %s\n", src);
  printf("-----------------------------------------\n");
  char dst[IPv4_STR_SIZE];
  ipv4_addr_str( header.ip_dst, dst );
  printf("\t    %s\n", dst);
  printf("-----------------------------------------\n");

  return;
}

/*
* Devuelve TRUE si estamos suscritos 
* a esa ip_addr o si es nuestra ip
* @Return 1 si exito, 0 si fracaso
*/
int ip_subscribed_to( ipv4_addr_t addr ) {

  //si es nuestra ip estamos suscritos de sobra!
  if( memcmp(addr, ipv4_hostaddr, IPv4_ADDR_SIZE) == 0 )
   return 1;

 char ip_str[IPv4_STR_SIZE];
 ipv4_addr_str(addr, ip_str);

  int i;
  for(i = 0; i < _CACHE_SIZE; i++) {
    if( strcmp(multicast_list[i].ip, ip_str) == 0 ) 
    {
      printf("We are subscribed to %s\n", ip_str);
      return 1;
    } 
  }

  return 0; /* default */
}


/* Adds an element to the multicast list, and returns a pointer to it */
multicast_handler join_multicast( char * ip_str ) {

  multicast_handler handler = NULL;
  char mac_str[MAC_STR_LENGTH];
  mac_addr_t mac_addr;
  ipv4_addr_t ip_addr;
  ipv4_str_addr( ip_str, ip_addr);

  /* Now we map it: we take de 23 lower bits of the ip */
  /* And prepend a single 0 */
  //ip_addr[2] = ip_addr[2] & 254;

  /* Prepend Prepend 01:00:5e */
  //mac_addr = { 0x01, 0x00, 0x5e, ip_addr[2], ip_addr[3], ip_addr[4] };
  //mac_addr_str(mac_addr, mac_str);

  int i = 0;
  for(i = 0; i < _CACHE_SIZE; i++) {
    if( strcmp(multicast_list[i].mac, multicast_list[i].ip) == 0 ) {
            //si el tiempo de una entrada se ha acabado
            //o la string mac == ip (es decir, vacia)
      strcpy(multicast_list[i].mac, mac_str);
      strcpy(multicast_list[i].ip, ip_str);
      multicast_entries++;
      printf("Joined to multicast ip %s \n", ip_str);
      return &multicast_list[i];
    }
  }

  return handler;
}

int leave_multicast( multicast_handler handler ) {

  if(handler!=NULL) {

    memset(handler, 0, sizeof(multicast_resource));
    return 1;
  }

  return 0;
}
  #include "arp.h"

  //We define the structure containing the ARP fields
  struct arp_message {

	uint16_t l2_type;     /* Tipo de protocolo de nv2*/
	uint16_t l3_type;     /* Tipo de protocolo de red*/

	uint8_t mac_addr_len;    /* Longitud mac adress en bits*/
	uint8_t ip_addr_len;     /* Longitud ip address en bits*/

	uint16_t op_code;    /* Codigo opcional */ 

	mac_addr_t mac_src;    /* Dirección MAC origen */
	ipv4_addr_t ip_src;    /* Dirección IP origen*/

	mac_addr_t mac_target;    /* Dirección MAC destino*/
	ipv4_addr_t ip_target;    /* Dirección IP destino*/
  };

  struct cache_resource {

    

    char mac [MAC_STR_LENGTH]; // Mac
    char ip [IPv4_STR_SIZE]; // Ip

    timerms_t timer; // tiempo en el que se creo
  };

  // Cache
  struct cache_resource cache [_CACHE_SIZE];
  int cache_entries;
  typedef struct arp_message arp_msg;
  typedef struct arp_message * arp_msg_ptr;

  arp_msg arp_request;
  arp_msg_ptr arp_request_ptr = &arp_request;

  arp_msg arp_reply;
  arp_msg_ptr arp_reply_ptr = &arp_reply;

  //devuelve 1 si todo ha ido bien, -1 si no ha recibido nada

  int arp_resolve (eth_iface_t * iface, ipv4_addr_t host_addr, ipv4_addr_t target_addr, mac_addr_t mac_addr) {

    if(cache_entries == 0) {
      print_notice("Opened cache arp\n");
      memset(cache, 0, sizeof(struct cache_resource)*_CACHE_SIZE);
    }

    // Comprobamos si esta en la cache
    char target_str [IPv4_STR_SIZE];
    ipv4_addr_str(target_addr, target_str);

    int i;
    for(i = 0; i < _CACHE_SIZE; i++) {
      if( strcmp(cache[i].ip, target_str) == 0 ) 
      {

	print_success("Already in cache\n");
	if(timerms_left(&cache[i].timer) > 0) {
	  mac_str_addr(cache[i].mac, mac_addr);
	  return 1;
	}
	bold("...but timer has run out\n");
      } 
    }

    /* VARIABLES */
    /* Inicializamos flood request */
    arp_request_ptr->l2_type = htons (LVL2_TYPE);
    arp_request_ptr->l3_type = htons (LVL3_TYPE);
    arp_request_ptr->ip_addr_len = IPv4_ADDR_SIZE;
    arp_request_ptr->mac_addr_len = MAC_ADDR_SIZE;
    arp_request_ptr->op_code = htons (OP_CODE_ARP_REQ);
    memcpy (arp_request_ptr->ip_src, host_addr, IPv4_ADDR_SIZE); // IP LOCAL
    eth_getaddr (iface, arp_request_ptr->mac_src); //MAC LOCAL
    mac_str_addr (MAC_ANY_ADDR, arp_request_ptr->mac_target);
    memcpy (arp_request_ptr->ip_target, target_addr, IPv4_ADDR_SIZE);

    /* resto de variables */
    /* Para el buffer de datos hay que esperar la longitud maxima */

    int bytes_sent = 0;
    int bytes_recv = 0;                 //suponemos fracaso
    char ip_str_recv[IPv4_STR_SIZE]; //Ip que recibimos
    char mac_str_recv[MAC_STR_LENGTH]; //cadena de la mac recibida

    /* Temporizador y guardian de intentos */

    int n = 0;                      //numero de intentos
    timerms_t timer;                //cronometro 
    long int timeout = 16000;        //2 segundos
    timerms_reset(&timer, timeout); //empezamos el contador

    while (n < 2) {
	  //mientras no se haya recibido una MAC con la ip deseada
	  //mientras quede tiempo 
	  //2 intentos

	  /* ENVIO */
      bold("Sending an ARP request:\n");
      bytes_sent = eth_send (iface, 
	MAC_BCAST_ADDR,
	ETH_ARP_TYPE,
	(unsigned char *) arp_request_ptr,
	sizeof (arp_request));

      if( bytes_sent < 0 ) {

	print_alert ("Error during arp flood req\n");
	printf("\tarp.c: arp_resolve() => bytes_sent = -1 \n");
	return -1;

      } else if ( bytes_sent == 0 ) {

	print_alert ("Unable to send arp message \n");
	printf("\tarp.c: arp_resolve() => bytes_sent = 0 \n");
	return -1;
      }

	  /* RECEPCION */
	  
      bytes_recv = -1;
      arp_reply_ptr->op_code = 0;
      
      while (ntohs(arp_reply_ptr->op_code) != 2  && 
	timerms_left (&timer))
      {
	
	
      bytes_recv = eth_recv (iface,
	mac_addr, 
	ETH_ARP_TYPE,
	(unsigned char *) arp_reply_ptr,
      timerms_left (&timer) );
      
       }
       
     
      ipv4_addr_str ( arp_reply_ptr->ip_src, ip_str_recv );
      mac_addr_str ( arp_reply_ptr->mac_src, mac_str_recv );
     

      //comparamos los resultados recibidos con los deseados
      //si se reciben bytes y es un reply:
      if( bytes_recv > 0 && ntohs(arp_reply_ptr->op_code) == 2 ) {
      
	printf("We are looking for ipv4: %s; \n"
	  "Received from %s, with MAC: %s\n", 
	  target_str, ip_str_recv, mac_str_recv );

	if ( !strcmp(target_str, ip_str_recv) ) {
	//comprobamos si es la ip que buscamos
	  memcpy (mac_addr, arp_reply_ptr->mac_src, 
	    sizeof(arp_reply_ptr->mac_src));

	// Guardamos la mac en la cache
	  int i = 0;
	  for(i = 0; i < _CACHE_SIZE; i++) {
	    if(
	      timerms_left(&cache[i].timer) == 0 || 
	      strcmp(cache[i].mac, cache[i].ip) == 0
	      ) {
	      //si el tiempo de una entrada se ha acabado
	      //o la string mac == ip (es decir, vacia)
	      timerms_reset(&cache[i].timer, _CACHE_TIME);
	      strcpy(cache[i].mac, mac_str_recv);
	      strcpy(cache[i].ip, ip_str_recv);
	      cache_entries++;
	      print_success("Saved into ARP cache\n");
	      return 1;
	    }

	    
	  }
	  print_warning ("ARP cache full\n");
	  return 1;
	}

      } else if( bytes_recv < 0 ) {

	print_alert("Catched error on reception\n");
	printf("\t arp_resolve: bytes_recv = -1\n\n");
	return -1; 

      } else if (timerms_left (&timer) == 0 ) {
	
        //intentos
        if (n==1){
	print_alert("Nobody answered. \n");
	printf("\tarp_resolve: time expired #%d essay\n", n);
	return 0;
	}
	 n++;
	
      }

    }

    return 1;
  }

  void print_cache () 
  {
    int i;
    underline("Arp cache:\n");
    for(i = 0; i < 5; i++) 
    {
      printf("ipv4: %s |", cache[i].ip);
      printf(" MAC: %s |", cache[i].mac);
      printf(" %ld s left\n", timerms_left(&(cache[i].timer))/1000 );
    }
  }
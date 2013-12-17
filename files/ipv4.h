#ifndef _IPv4_H
#define _IPv4_H

#include <stdint.h>
#include <stdio.h>
#include "util.h"
#include <timerms.h>
#include <string.h>
#include <rawnet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include "eth.h"

#define IP_ANY_ADDR "0.0.0.0"
#define IPV4_OVER_ETH_TYPE 0x0800
#define IPv4_ADDR_SIZE 4
#define IPv4_STR_SIZE  16
#define ETH_MTU 1500
#define HED_LEN 20

typedef unsigned char ipv4_addr_t [IPv4_ADDR_SIZE];

//HEADER STRUCTURE
typedef struct {

uint8_t   ver_and_ihl; 	//ip_version and header length;
uint8_t   ip_tos; 		//type of service;
uint16_t  ip_pkg_len; 	//hdr + body length 
uint16_t  ip_pkg_id; 	//identificador de paquete
uint16_t  ip_offset; 	//para indicar que continua un paquete fragmentado
uint8_t   ip_ttl; 		//time to live
uint8_t   ip_prot; 		//protocolo de nivel superior
uint16_t  ip_checksum;
ipv4_addr_t  ip_src;
ipv4_addr_t  ip_dst;

} ip_header, * ip_header_ptr;


//DATAGRAM STRUCTURE
typedef struct {
  
  ip_header header;
  unsigned char data [ETH_MTU - 20];
  
} ip_datagram, * ip_datagram_ptr;

//para subscribirse a ip/mac multicast
typedef struct multicast_list_element {

  char mac [MAC_STR_LENGTH]; // Mac
  char ip [IPv4_STR_SIZE]; // Ip

} multicast_resource, *multicast_handler;

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
	/* Funciones diseniadas para la implementacion del protocolo */

	/*
	Esta funcion iniciaiza la interfaz Ethernet asi como las tablas de rutas y 
	provee el contexto necesario para el envio y la recepcion de los paquetes
	Devuelve -1 si algo ha ido mal
	*/
int ipv4_open(); 

//Envia un paquete ip
int ipv4_send( ipv4_addr_t dst, uint16_t prot, unsigned char * payload, int payload_len );

//Recibe un paquete ip
int ipv4_recv( ipv4_addr_t src, uint16_t prot, unsigned char buffer[], long int timeout );

//Cierra el contexto ip, la lectura del fichero
void ipv4_close(); 

//Funcion que suscribe a la ip multicast y mac multicast deseadas
//devuelve un puntero a la misma o NULL si no ha sido posible
multicast_handler join_multicast( char * ip_str );

//Pone a 0 el elemento de la lista multicast apuntado por handler
//@Return 1 si exito, 0 si fracaso
int leave_multicast( multicast_handler handler );

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

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
 uint16_t ipv4_checksum ( unsigned char * data, int len );


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
 void ipv4_addr_str ( ipv4_addr_t addr, char* str );


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
 int ipv4_str_addr ( char* str, ipv4_addr_t addr );


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
 void print_ip_header( ip_header header );

/* Devuelve TRUE si estamos suscritos 
a esa ip_addr o si es nuestra ip*/
int ip_subscribed_to( ipv4_addr_t addr );

#endif

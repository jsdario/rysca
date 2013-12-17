#include "ipv4.h"

#define ETH_ARP_TYPE 0x0806

#define LVL2_TYPE 1  	  	//para arp mac
#define LVL3_TYPE 0x0800 	//para arp ip
#define OP_CODE_ARP_REQ 1 
#define OP_CODE_ARP_RPLY 2
#define _CACHE_SIZE 50
#define _CACHE_TIME 60000

/*
* Tiene cache
* Resuelve direcciones pac
* recibe por parametros ip buscadae ip propia
*/
int arp_resolve (eth_iface_t * iface, ipv4_addr_t src,
 ipv4_addr_t ip_host_addr, mac_addr_t mac_addr);
void print_cache();

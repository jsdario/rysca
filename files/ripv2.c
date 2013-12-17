#include "ripv2.h"

/* This way we can recover the iface we are handling*/
/* Name of the iface, just for printing @ the table */
extern eth_iface_t * iface_handler; 
extern ipv4_addr_t ipv4_hostaddr;

/*
*  Crea un elemento ruta rip para enlazar
*  en la tabla de rutas
*  @return &elemento
*/
rip_route_t * rip_route_create ( ipv4_addr_t subnet, 
  ipv4_addr_t mask, char * iface_name, ipv4_addr_t next_hop, uint32_t metric ) {

  rip_route_t * route = (rip_route_t *) malloc(sizeof(rip_route_t));

  if (route != NULL) {
    memcpy(route->ipv4_addr, subnet, IPv4_ADDR_SIZE);
    memcpy(route->ipv4_mask, mask, IPv4_ADDR_SIZE);
    memcpy(route->iface, iface_name, IFACE_NAME_LENGTH);
    memcpy(route->ipv4_next, next_hop, IPv4_ADDR_SIZE);
    route->metric = htonl (metric);
  }

  return route;
}

/*
*  Crea una lista enlazada de rutas rip 
*  @Return &raiz
*/
rip_table_t * rip_table_create()
{
  rip_table_t * table;

  table = (rip_table_t *) malloc(sizeof(rip_table_t));
  if (table != NULL) {
    int i;
    for (i=0; i<RIP_ROUTE_TABLE_SIZE; i++) {
      table->routes[i] = NULL;
    }
  }
   
  table->num_entries = 0;
  return table;
}

/* 
*  Funcion para abrir el puerto rip 
*  Aquí hay que suscribirse a multicast 
*  en caso de implementar
*/

int initialize_rip (rip_table_t * pointer, int port){

  int flag = udp_open (port);

  if (flag == -1) {
    print_alert ("Unable to open port\n");
    printf("\tripv2.c: initialize_rip(): udp_open = -1\n");
  } else 

  join_multicast( RIP_MULTICAST_IPv4 );

  pointer->num_entries = 0;

  return flag;
}


/*
*  Funcion para aniadir un elemento a la lista
*  de rutas rip
*  @Param &tabla, &ruta
*/
int rip_route_table_add ( rip_table_t * table, rip_route_t * route )
{

  int route_index = -1;

  if (table != NULL) {
    /* Find an empty place in the route table */
    int i;
    for (i=0; i<RIP_ROUTE_TABLE_SIZE; i++) {
      if (table->routes[i] == NULL) {
        table->routes[i] = route;
        route_index = i;
        break;
      }
    }
  }

  table->num_entries++;
  table->routes[route_index]->garbage_flag = 0;

  return route_index;
}

/*
*  Funcion para celiminar un elemento de la lista
*  de rutas RIP.
*  @Param &tabla, indice
*/
rip_route_t * rip_route_table_remove ( rip_table_t * table, int index )
{
  rip_route_t * removed_route = NULL;

  if ((table != NULL) && (index >= 0) && (index < RIP_ROUTE_TABLE_SIZE)) {
    removed_route = table->routes[index];
    table->routes[index] = NULL;
  }
  table->num_entries--;
  return removed_route;
}

/*
*  Funcion para coger un elemento de la lista
*  de rutas RIP. 
*  @Param &tabla, indice
*/
rip_route_t * rip_route_table_get ( rip_table_t * table, int index ) {

  rip_route_t * route = NULL;

  if ((table != NULL) && (index >= 0) && (index < RIP_ROUTE_TABLE_SIZE)) {
    route = table->routes[index];
  }

  return route;
}

int rip_route_table_find
( rip_table_t * table, ipv4_addr_t subnet, ipv4_addr_t mask ) {

  int route_index = -2;

  if (table != NULL) {
    route_index = -1;
    int i;
    for (i=0; i<RIP_ROUTE_TABLE_SIZE; i++){
      rip_route_t * route_i = table->routes[i];
      if (route_i != NULL) {
        int same_subnet = 
        (memcmp(route_i->ipv4_mask, subnet, IPv4_ADDR_SIZE) == 0);
        int same_mask = 
        (memcmp(route_i->ipv4_mask, mask, IPv4_ADDR_SIZE) == 0);
        if (same_subnet && same_mask) {
          route_index = i;
          break;
        }
      }
    }
  }

  return route_index;
}


int rip_route_table_read ( char * filename, rip_table_t * table )
{
  int read_routes = 0;

  FILE * routes_file = fopen(filename, "r");
  if (routes_file == NULL) {
    fprintf(stderr, "Error opening input IPv4 Routes file \"%s\": %s.\n",
      filename, strerror(errno));
    return -1;
  }

  int linenum = 0;
  char line_buf[1024];
  char subnet_str[256];
  char mask_str[256];
  char iface_name[256];
  char gw_str[256];
  char metric_str[256];
  char timeout_str[256];

  int err = 0;


  while ((! feof(routes_file)) && (err==0)) {

    linenum++;

    /* Read next line of file */
    char* line = fgets(line_buf, 1024, routes_file);
    if (line == NULL) {
      break;
    }

    /* If this line is empty or a comment, just ignore it */
    if ((line_buf[0] == '\n') || (line_buf[0] == '#')) {
      err = 0;
      continue;
    }

    /* Parse line: Format "<subnet> <mask> <iface> <gw>\n" */
    err = sscanf(line, "%s %s %s %s %s %s\n", 
     subnet_str, mask_str, iface_name, gw_str, metric_str, timeout_str);
    if (err != 6) {
      fprintf(stderr, "%s:%d: Invalid IPv4 Route format: \"%s\" (%d items)\n",
        filename, linenum, line, err);
      fprintf(stderr, 
        "%s:%d: Format must be: <subnet> <mask> <iface> <gw>\n",
        filename, linenum);
      err = -1;

    } else {

      /* Parse IPv4 route subnet address */
      ipv4_addr_t subnet;
      err = ipv4_str_addr(subnet_str, subnet);
      if (err == -1) {
        fprintf(stderr, "%s:%d: Invalid <subnet> value: \"%s\"\n", 
          filename, linenum, subnet_str);
        break;
      }

      /* Parse IPv4 route subnet mask */
      ipv4_addr_t mask;
      err = ipv4_str_addr(mask_str, mask);
      if (err == -1) {
        fprintf(stderr, "%s:%d: Invalid <mask> value: \"%s\"\n",
          filename, linenum, mask_str);
        break;
      }

      /* Parse IPv4 route gateway */
      ipv4_addr_t gateway;
      err = ipv4_str_addr(gw_str, gateway);
      if (err == -1) {
        fprintf(stderr, "%s:%d: Invalid <gw> value: \"%s\"\n",
          filename, linenum, gw_str);
        break;
      }
 
      int metric = atoi (metric_str);
      //int timeout = atoi (timeout_str);

      /* Create new route & add it to Route Table */
      rip_route_t * new_route = 
      rip_route_create(subnet, mask, iface_name, gateway, ntohl(metric));

      if (table != NULL) {
        err = rip_route_table_add(table, new_route);
        if (err >= 0) {
          err = 0;
          read_routes++;
        }
      }
    }
  } /* while() */

    if (err == -1) {
      read_routes = -1;
    }

  /* Close IP Route Table file */
    fclose(routes_file);

    table->num_entries = read_routes;
    return read_routes;
  }


  int rip_route_table_output ( FILE * out, rip_table_t * table, int dontknow)
  {
    int err;

    if (dontknow == 0) {
      err = fprintf
      (out, "# SubnetAddr  \tSubnetMask    \tIface  \tNextHop\n");
    } else {
      err = fprintf
      (out, "# SubnetAddr  \tSubnetMask    \tIface  \tNextHop       \tMetric \tTimeout\n");

    }
    if (err < 0) {
      return -1;
    }

    char subnet_str[IPv4_STR_SIZE];
    char mask_str[IPv4_STR_SIZE];
    char* ifname = NULL;
    char gw_str[IPv4_STR_SIZE];
    int metric;
    int timeout;

    int i;
    for (i=0; i<RIP_ROUTE_TABLE_SIZE; i++) {
      rip_route_t * route_i = rip_route_table_get(table, i);
      if (route_i != NULL) {
        ipv4_addr_str(route_i->ipv4_addr, subnet_str);
        ipv4_addr_str(route_i->ipv4_mask, mask_str);
        ifname = route_i->iface;
        ipv4_addr_str(route_i->ipv4_next, gw_str);
        metric = route_i->metric;

        timeout = timerms_left (&route_i->time);
        /*you like comments don't you? guess this =>*/
        if (timeout == -2) timeout = -1;

        if (metric == 0) {
         err = fprintf(out, "%-15s\t%-15s\t%s\t%-15s\t%s\n",
          subnet_str, mask_str, ifname, gw_str, "STATIC");
       } else {
         err = fprintf(out, "%-15s\t%-15s\t%s\t%-15s\t%d\t%d\n",
          subnet_str, mask_str, ifname, gw_str, metric, timeout);
       }
       if (err < 0) {
        return -1;
      }
    }
  }

  printf("\n");
  return 0;
}


int rip_route_table_write ( rip_table_t * table, char * filename )
{
  int num_routes = 0;

  FILE * routes_file = fopen(filename, "w");
  if (routes_file == NULL) {
    fprintf(stderr, "Error opening output IPv4 Routes file \"%s\": %s.\n",
      filename, strerror(errno));
    return -1;
  }

  fprintf(routes_file, "# %s\n", filename);
  fprintf(routes_file, "#\n");

  if (table != NULL) {
    num_routes = rip_route_table_output (routes_file, table, 1);
    if (num_routes == -1) {
      fprintf(stderr, "Error writing IPv4 Routes file \"%s\": %s.\n",
        filename, strerror(errno));
      return -1;
    }
  }

  fclose(routes_file);

  return num_routes;
}


/*
* Another AWESOME alias, API for our team 
*/
void rip_route_table_print ( rip_table_t * table )
{
  if (table != NULL) {
    rip_route_table_output (stdout, table, 1);
  }
}

/* El equipo 6 manda */
void print_entry (rip_entry_ptr pointer){

  printf ("{\n family_id = %d ; route_tag %d ; metric %d;\n", 
   ntohs (pointer->family_id), 
   ntohs (pointer->route_tag),
   ntohl (pointer->metric));

  char ip_addr [32];
  char ip_mask [32];
  char ip_next [32];

  ipv4_addr_str (pointer->ip_addr, ip_addr);
  ipv4_addr_str (pointer->ip_mask, ip_mask);
  ipv4_addr_str (pointer->ip_next, ip_next);


  printf (" ip_addr = %s ; netmask = %s ; next_hop = %s\n}\n",
   ip_addr,
   ip_mask,
   ip_next);
}

void rip_route_free ( rip_route_t * route )
{
  if (route != NULL) {
    free(route);
  }
}

void rip_route_table_free ( rip_table_t * table )
{
  if (table != NULL) {
    int i;
    for (i=0; i<RIP_ROUTE_TABLE_SIZE; i++) {
      rip_route_t * route_i = table->routes[i];
      if (route_i != NULL) {
        table->routes[i] = NULL;
        rip_route_free(route_i);
      }
    }
    free(table);
  }
}

void print_packet (rip_header_ptr pointer, int num_entries){

  underline ("# RIPv2 packet:");

  if (pointer->command == 1)
    printf (" request message with %d entries\n", num_entries);
  else 
    printf (" response message with %d entries\n", num_entries);

  if (num_entries > 0){

    int i;
    for (i=0; i<num_entries; i++){
      printf(BOLD "Entry #%d" RESET, i+1);
      print_entry (&(pointer->entry[i]));
    }
  }
}


/* Another mind-blower function */
int rip_message_size (int num_entries){

 return RIP_HEADER + (num_entries * RIP_ENTRY);
}

/* Another mind-blower function */

int rip_number_entries (int message_size){

  return (message_size - RIP_HEADER)/RIP_ENTRY;  
}


int rip_send (ipv4_addr_t dst, rip_header_ptr pointer, int num_entries, int dst_port){

  int size_of_message = rip_message_size (num_entries);
  unsigned char payload [size_of_message];
  memcpy (payload, pointer, size_of_message);
  int bytes_sent = udp_send (dst, dst_port, payload, size_of_message);
  
  return bytes_sent;
}


int rip_recv (ipv4_addr_t src, rip_header_ptr pointer, int timeout, int * src_port){

  unsigned char buffer [RIP_MAX];
  int bytes_received = udp_recv (src, buffer, timeout, src_port);
  memcpy (pointer, buffer, bytes_received);
  
  return bytes_received;
}  

/*
* Funcion que coge un mensaje rip, le da formato y lo añade a la tabla.
*/

void add_entries_table (rip_table_t * table, rip_header_ptr pointer, int num_entries){
   

  char ifname[IFACE_NAME_LENGTH];
  strcpy( ifname, eth_getname(iface_handler));

  int index = 0;

  int i;

  for (i=0; i<num_entries; i++){ 

     rip_route_t * route = rip_route_create 
    (
      pointer->entry [i].ip_addr,
      pointer->entry [i].ip_mask,
      ifname, 
      pointer->entry [i].ip_next,
      pointer->entry [i].metric
      );
   index = rip_route_table_add (table, route);
    
    timerms_reset (&table->routes[index]->time, TIMEOUT);
  }
      
}


rip_table_t * convert_message_table (rip_header_ptr pointer, int num_entries){

  rip_table_t * table = rip_table_create ();

  /* 
  * Name of the iface, just for printing @ the table 
  * Recovered from eth.c by an extern directive
  */
  char ifname[IFACE_NAME_LENGTH];
  strcpy( ifname, eth_getname(iface_handler));
  
  int index = 0;

  int i;
  
  for (i=0; i<num_entries; i++){

    rip_route_t * route = rip_route_create 
    (
      pointer->entry [i].ip_addr,
      pointer->entry [i].ip_mask,
      ifname, 
      pointer->entry [i].ip_next,
      pointer->entry [i].metric
      );
   index = rip_route_table_add (table, route);
    
    timerms_reset (&table->routes[index]->time, TIMEOUT);
    
  }
  
  table->num_entries = num_entries;
  
  return table;
}


/*
* In case we receive a better route than one
* of our own, we shall destroy use the one better.
* Otherwise we would be pretty dumb, don't we?
*
* @Params: &table, &route2remove, &route2write,
* AND the index of route2remove
*/
int rip_replace_entry (rip_table_t * table, rip_route_t * route, rip_route_t * route_aux, int index){

  int time_left = timerms_left (&route->time);
  route = rip_route_table_remove (table, index);
  rip_route_free (route);
  index = rip_route_table_add (table, route_aux);
  timerms_reset (&table->routes[index]->time, time_left);
  return index;
  
}

/*
* ANOTHER ALIAS BY JAY-JAY! This time, it JUST compares an IP
* He is our miracle.
* Eh, it is useful
* @Returns true when entry is the same
*/
int same_entry (ipv4_addr_t addr, ipv4_addr_t addr_aux){

  if (memcmp (addr, addr_aux, IPv4_ADDR_SIZE)!=0)
    return 0;

  else
    return 1;
}

/*
* ANOTHER ALIAS BY JAY-JAY! This time, it JUST compares a number!
* He is our miracle. YES, I have done this so many times this is
* copy-paste
*/
int compare_metrics (int metric, int metric_aux){

  if (metric < metric_aux) return -1;
  if (metric == metric_aux) return 0;
  if (metric > metric_aux) return 1;
  
  else 
    return -2;
}

/*
* ANOTHER ALIAS BY JAY-JAY! This time, it JUST says if is 16!
* He is our miracle. YES, I have done this so many times this is
* copy-paste.
*
* BTW, I changed its name to be MOAR pro, i hop ya liek it
*/
int metric_is_inf (int metric){

  if (metric == 16)
    return 1;
  else
   return 0;  
}

/*
* ANOTHER ALIAS BY JAY-JAY! This time, it JUST collapses a timer
* He is our miracle. YES, I have done this so many times this is
* copy-paste
*/
void set_delete_flag (rip_route_t * route){

  //int time_left = timerms_left (&route->time);
  //printf ("time_left: %d\n", time_left);
  timerms_reset (&route->time, TIMER_ZERO);
  
}


void reset_entry_timer (rip_route_t * route){

  timerms_reset(&route->time, TIMEOUT);
  route->garbage_flag = 0;
}

int timer_halfway(rip_route_t * route){

  if (timerms_left (&route->time) <= TIMEOUT/2) return 1;
  else return 0;
  
}

//RETURN 0 WHEN NO TRIGGER UPDATE IS NEEDED (equal entries)
//RETURN 1 WHEN TRIGGER UPDATE IS NEEDED (not equal entries)
// and replace when better cost
int compare_entries (rip_table_t * table, 
  rip_route_t * route_aux,
  ipv4_addr_t src ) {

  int index;
  int i;
  
  for (i=0; i<RIP_ROUTE_TABLE_SIZE; i++){

    if (table->routes[i] != NULL){
    //IS THERE AN EXISTENT ROUTE??
    //WE COMPARE DESTINATION ADDRESS

      if (same_entry (table->routes[i]->ipv4_addr, route_aux->ipv4_addr)){
    //THERE IS AN EXISTENT ROUTE

      //COMPARE METRICS
        int cmp_metrics = compare_metrics (table->routes[i]->metric, route_aux->metric);

      //WE CHECK IF THIS DATAGRAM IS FROM THE SAME ROUTER AS THE EXISTING ROUTE
        if (same_entry (table->routes[i]->ipv4_next, src)){
      //THIS DATAGRAM IS FROM THE SAME ROUTER AS THE EXISTING ROUTE

      //WE REINITIALIZE TIMEOUT
      //reset_entry_timer (table->routes[i]);

      //WE CHECK IF METRICS ARE DIFFERENT
          if (cmp_metrics != 0){
      //METRICS ARE DIFFERENT

	//CHECK IF METRIC IS 16
           if (metric_is_inf (route_aux->metric)){
	//METRIC IS 16

	  //SUBSTITUTE ROUTE AND SET DELETE FLAG
             set_delete_flag (table->routes[i]);
             index = rip_replace_entry (table, table->routes[i], route_aux, i); 
             memcpy (table->routes[index]->ipv4_next, src, IPv4_ADDR_SIZE);

             return 1;

           } else {
	//METRIC IS LOWER THAN 16 

	  //SUBSTITUTE ROUTE AND RESET TIMER
             index = rip_replace_entry (table, table->routes[i], route_aux, i); 
             memcpy (table->routes[index]->ipv4_next, src, IPv4_ADDR_SIZE);
             reset_entry_timer (table->routes[index]);
             return 1;

           }

         }else{
      //METRICS ARE THE SAME

          if (!metric_is_inf (route_aux->metric)){
      //METRIC IS NOT 16

      //RESET TIMER
            reset_entry_timer (table->routes[i]);
            return 0;
          }else{
      //METRIC IS 16
           set_delete_flag (table->routes[i]);
           return 0;
         }

       }
     }else{
      //THIS DATAGRAM IS NOT FROM THE SAME ROUTER AS THE EXISTING ROUTE

      //WE CHECK IF METRICS ARE DIFFERENT
      if (cmp_metrics != 0){
      //METRICS ARE DIFFERENT

      //WE CHECK IF THE NEW METRIC IS LOWER THAN THE ACTUAL ONE

        if (cmp_metrics < 0)
      //THE ACTUAL METRIC IS A BETTER OPTION, DO NOTHING
          return 0;

        else{
      //THE NEW METRIC IS A BETTER OPTION, WE ADOPT THE NEW ROUTE
          index = rip_replace_entry (table, table->routes[i], route_aux, i); 
          memcpy (table->routes[index]->ipv4_next, src, IPv4_ADDR_SIZE); 
          reset_entry_timer (table->routes[index]);
          return 1;

        }

      }else{
      //METRICS ARE THE SAME

      //WE CHECK IF THE TIMER FOR THE ACTUAL ENTRY IS HALFWAY OR LOWER
        if (timer_halfway(table->routes[i])){
      //TIMER HALFWAY OR LOWER, SUBSTITUTE ROUTE

          index = rip_replace_entry (table, table->routes[i], route_aux, i); 
          memcpy (table->routes[index]->ipv4_next, src, IPv4_ADDR_SIZE); 
          reset_entry_timer (table->routes[index]);
          return 0;
        }else
      //TIMER IS FINE, DO NOTHING
        return 0;

      }
    }
  }
}
}
   //THERE IS NOT AN EXISTENT ROUTE

if (route_aux->metric < 16){

  index = rip_route_table_add (table, route_aux);
  memcpy (table->routes[index]->ipv4_next, src, IPv4_ADDR_SIZE); 
  reset_entry_timer (table->routes[index]);
  return 1;

}

else 
  return 0;
}

void update_metrics (rip_table_t * pointer, int num_entries){

  int i;
  
  for (i=0; i<num_entries; i++){

    if (pointer->routes[i]->metric<16)
      pointer->routes[i]->metric++;
    else 
      pointer->routes[i]->metric = 16;
    
  }
}

/*
* @Returns > 0 if there has changed an entry 
*  or zero if tables are the same
*/
int compare_tables (rip_table_t * table, rip_table_t * table_aux, int num_entries, ipv4_addr_t src){

  //UPDATE METRICS FOR INCOMING TABLE
  //lo que solo tiene sentido si el mensaje viene
  //de una maquina que no es la nuestra
  if( memcmp(ipv4_hostaddr, src, sizeof(ipv4_addr_t)) != 0) {
    update_metrics (table_aux, num_entries);
  }

  //variable used for checking if we have modified the table
  int changes = 0;

  int i;

  for (i=0; i<num_entries; i++){

    changes += compare_entries (table, table_aux->routes[i], src);  
  }

  return changes;
}



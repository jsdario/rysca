#include "ripv2_server_files.h"
#include "../files/ripv2.h"

extern int is_verbose;

/* Alias para timerms_reset */
timerms_t timer_start (long int countdown){

  timerms_t timer;
  timerms_reset(&timer, countdown);
  return timer;  
}

/* 
* Alias para timerms_left
* @Return true si el contador se ha acabado
*  y false en caso contrario
*/
int timer_ended (timerms_t timer){

 if (timerms_left (&timer) == 0)
   return 1;
 else 
   return 0;

}

/*
* Sends an update NO MATTER HOW!
* doesnt return anything, but anyways
* it DEMANDS a pointer to the header to be sent
* AND the number of entries
*/

void send_update (rip_header_ptr pointer, int num_entries, int timer_ended){

  if (timer_ended)
    print_notice ("Update timer ended, sending a periodic update\n");
  else
    print_notice ("A cost has changed, sending a triggered update\n");

  ipv4_addr_t ip_multi;
  ipv4_str_addr (RIP_MULTICAST_IPv4, ip_multi);
  if( is_verbose )
    print_packet( pointer, num_entries );
  rip_send (ip_multi, pointer, num_entries, RIP_PORT);
}


/*
* Another single-line function, this time we also print a cool message 
* Jay-Jay strikes again
*/
void send_response (ipv4_addr_t dst, rip_header_ptr pointer, int num_entries, int dst_port){

  printf ("Sending response\n\n");
  //print_packet (pointer, num_entries);
  rip_send (dst, pointer, num_entries, dst_port);  
}


/*
* This is good stuff 
* creates an ordinary rip response message
* takes whole table and a pointer to the message
* to be sent
*/
void create_rip_message (rip_header_ptr pointer, rip_table_t * table){

  pointer->command = 2; //Indicates response
  pointer->version = 2;

  int i;
  int j = 0;

  for (i=0; i < RIP_ROUTE_TABLE_SIZE ; i++){

   if (table->routes[i] != NULL){
     pointer->entry[j].family_id = htons (2);
     memcpy (pointer->entry[j].ip_addr, table->routes[i]->ipv4_addr, IPv4_ADDR_SIZE);
     memcpy (pointer->entry[j].ip_mask, table->routes[i]->ipv4_mask, IPv4_ADDR_SIZE);
     memcpy (pointer->entry[j].ip_next, table->routes[i]->ipv4_next, IPv4_ADDR_SIZE);
     pointer->entry[j].metric = htonl(table->routes[i]->metric);
     j++;
   }
 } 
}

void create_two_rip_message (rip_header_ptr pointer, rip_header_ptr pointer_two, rip_table_t * table){

  pointer->command = 2; //Indicates response
  pointer->version = 2;

  int i;
  int j = 0;
  int k = 0;

  for (i=0; i < RIP_ROUTE_TABLE_SIZE ; i++){

   if (table->routes[i] != NULL){
     if (j>24){
     pointer_two->entry[k].family_id = htons (2);
     memcpy (pointer_two->entry[k].ip_addr, table->routes[i]->ipv4_addr, IPv4_ADDR_SIZE);
     memcpy (pointer_two->entry[k].ip_mask, table->routes[i]->ipv4_mask, IPv4_ADDR_SIZE);
     memcpy (pointer_two->entry[k].ip_next, table->routes[i]->ipv4_next, IPv4_ADDR_SIZE);
     pointer_two->entry[k].metric = htonl(table->routes[i]->metric);
     k++;
     }else{
     pointer->entry[j].family_id = htons (2);
     memcpy (pointer->entry[j].ip_addr, table->routes[i]->ipv4_addr, IPv4_ADDR_SIZE);
     memcpy (pointer->entry[j].ip_mask, table->routes[i]->ipv4_mask, IPv4_ADDR_SIZE);
     memcpy (pointer->entry[j].ip_next, table->routes[i]->ipv4_next, IPv4_ADDR_SIZE);
     pointer->entry[j].metric = htonl(table->routes[i]->metric);
     j++;
     }
     
     
   }
 } 
}



void initialize_garbage_collector_timers (timerms_t * timer){

  int i;
  
  for (i=0; i<RIP_ROUTE_TABLE_SIZE; i++){

    timerms_reset (&timer[i], INFINITE_TIMER);
  }
}

/*
* Starts guardian for garbage collection,
* Ana Botella would need also this function.
*
* @Return 1 if on, and 0 otherwise.
*/

int garbage_collector_start (rip_table_t * table, timerms_t * timer){

  int is_garbage_on = 0;
  int i;
  
  for (i=0; i<RIP_ROUTE_TABLE_SIZE; i++){

   if (table->routes[i] != NULL && timerms_left (&table->routes[i]->time) == 0 
     && table->routes[i]->garbage_flag == 0){

     timerms_reset (&timer[i], GARBAGE_COLLECTOR);
   table->routes[i]->garbage_flag = 1;
   is_garbage_on = 1;
 }
}  

return is_garbage_on;
}

int garbage_collector (rip_table_t * table, timerms_t * timer){

  int i;
  
  int garbage_collected = 0;
  
  for (i=0; i<RIP_ROUTE_TABLE_SIZE; i++){

   if (timerms_left (&timer[i])==0 && table->routes[i] != NULL && timerms_left (&table->routes[i]->time)==0){

     garbage_collected = 1;
     rip_route_t * route = rip_route_table_remove (table, i);
     rip_route_free (route);
     timerms_reset (&timer[i], INFINITE_TIMER);

   }else if (table->routes[i] != NULL && timerms_left (&table->routes[i]->time)>0){

     timerms_reset (&timer[i], INFINITE_TIMER);
     table->routes[i]->garbage_flag = 0;

   }
 }

 return garbage_collected;
}

/*
* Funcion para un request selectivo, recoge desde tabla auxiliar 
* la peticion selectiva, crea una tabla con la informacion requerida 
* que devuelve por parametros
*/

rip_table_t * table_to_send (rip_table_t * table, rip_table_t * table_aux){

  int i;
  int j;

  rip_table_t * table_send = rip_table_create ();
   
  for (i=0; i<RIP_ROUTE_TABLE_SIZE; i++){

    if (table->routes[i] != NULL){

     for (j=0; j<RIP_ROUTE_TABLE_SIZE; j++){

       if (table_aux->routes[j] != NULL){
        /* If the IP's are the same, we proceed */
        if (same_entry (table->routes[i]->ipv4_addr, table_aux->routes[j]->ipv4_addr)){
          rip_route_table_add (table_send, table->routes[i]);

        }
      }
    }
  }
}

return table_send;
}





#include "ripv2_server_files.h"
#include "../files/ripv2.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <timerms.h>
#include <signal.h>

#define MIN_TIMER 1000
#define RIP_TABLE_TXT "../files/rip_table.txt"

rip_header message;
rip_header_ptr message_ptr = &message;
rip_header message_two;
rip_header_ptr message_two_ptr = &message_two;
rip_entry  message_entry;
rip_entry_ptr message_entry_ptr = &message_entry;

int is_verbose = 0; /* Not changed by the moment */

rip_table_t * table;

timerms_t garbage_collector_timers [RIP_ROUTE_TABLE_SIZE];

void interrupt() {

  //rip_close();
  printf("\nCaptured CTRL-C\n");
  //print_success("Writing routes on to file.\n");
  //rip_route_table_write (table, RIP_TABLE_TXT);
  printf("bye\n");
  exit(0);
}

int main(int argc, char * argv[]) {

  system("clear");

  signal(SIGINT, interrupt);

  ipv4_addr_t multiaddress;

  ipv4_str_addr (RIP_MULTICAST_IPv4, multiaddress);

  if(argc == 2) {
    if( !strcmp(argv[1], "--verbose")) {
      is_verbose = 1;
      print_warning("(Debug mode ON) \n");
    }
  }
  else 
    printf("(Run with --verbose to print more info)\n");

  bold ("Starting RIP Server... \t\t\t\t\t"); print_success("[ OK ]\n");

  table = rip_table_create ();
  if (initialize_rip (table, RIP_PORT) == -1) {
    /* Already printed advert inside function */
    return -1;
  }
  int K = rip_route_table_read ( RIP_TABLE_TXT, table );

  /* set inf timer to routes read from file */
  int k;
  for( k = 0; k < K; k++ ) {

    timerms_reset(&table->routes[k]->time, INFINITE_TIMER);
  }

  rip_table_t * table_aux;

  ipv4_addr_t src;
  long int r = random_number(-15, 15)*1000;
//SEND UNSOLICITED RESPONSE MESSAGES EVERY 30 SECONDS +- 15s
  timerms_t update_timer = timer_start ( UPDATE_TIME + r );

//SEND REQUEST TO FILL ROUTE TABLE 

  rip_route_table_print ( table );

  for ( ;; ) {

    if (timer_ended (update_timer)) { 

    /* Si se ha acabado el update timer */
     send_rip_response (multiaddress, message_ptr, table, RIP_PORT); 
     r = random_number(-15, 15)*1000;
     if ( is_verbose ) printf("(update_time set to %ld)\n", r +UPDATE_TIME);
     update_timer = timer_start (UPDATE_TIME + r);

     bold ("\nCurrent table:\n");
     rip_route_table_print ( table );

   }
   int src_port;
   
   int bytes_received = rip_recv (src, message_ptr, MIN_TIMER, &src_port);

//WE RECEIVE A MESSAGE
   if (bytes_received>0){

   //WE CONVERT THE MESSAGE TO A ROUTE TABLE FORMAT
    table_aux = convert_message_table (message_ptr, rip_number_entries(bytes_received));

    if ( is_verbose ) {

      print_notice ("\nReceived packet\n");
      print_packet (message_ptr, rip_number_entries(bytes_received));
    }

//IF THE MESSAGE IS A RESPONSE...
    if (message_ptr->command == 2) {

  //VALIDATE (HAY QUE IMPLEMENTARLO)
  //number of entries in the received message
      int num_entries = rip_number_entries (bytes_received);

      int trig_update_flag = 0; //by default, when receiving, do not send update

  //AND THIS IS WHERE THE MAGIC HAPPENS, WE PROCESS THE RESPONSE MESSAGE
      trig_update_flag = compare_tables (table, table_aux, num_entries, src);
      
      
      if (trig_update_flag){
        
       send_rip_response (multiaddress, message_ptr, table, RIP_PORT);

      }
      bold ("\nCurrent table:\n");
      rip_route_table_print ( table );
    }

    if (message_ptr->command == 1 && 
      metric_is_inf(ntohl(message_ptr->entry[0].metric))) {
      //IF REQUEST FOR WHOLE TABLE, RESPOND WITH WHOLE TABLE

    print_warning("Received a request for single entry, sending whole table\n");

      send_rip_response (src, message_ptr, table, src_port);
  


  }  else if (message_ptr->command == 1) {
//IF REQUEST FOR SPECIFIC ENTRIES, RESPOND WITH SPECIFIC ENTRIES IF FOUND

    print_warning("Received a request for specific entries\n");
    rip_table_t * table_send = table_to_send (table, table_aux);

    send_rip_response (src, message_ptr, table_send, src_port);


    
    
  }
}


int is_garbage_on = garbage_collector_start (table, garbage_collector_timers);
int garbage_collected = garbage_collector (table, garbage_collector_timers);

if (garbage_collected || is_garbage_on) {

  if(garbage_collected)  print_notice("Garbage Collected \n");
  else if(is_garbage_on) print_notice("Garbage countdown ON\n");
  if( is_verbose ) rip_route_table_print(table);
}
}

return 0;	
}

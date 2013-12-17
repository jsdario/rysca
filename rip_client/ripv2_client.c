#include "../files/ripv2.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <timerms.h>

rip_header message;
rip_header_ptr message_ptr = &message;
rip_entry message_entry;
rip_entry_ptr message_entry_ptr = &message_entry;

void create_request ( ) {

    message_ptr->command = 1;
    message_ptr->version = 2;
    message_ptr->entry[0].family_id = 0;
    message_ptr->entry[0].route_tag = 0;
    message_ptr->entry[0].metric = htonl (16);
}


void create_specific_request (char ** entry_array, int num_entries){
  
  int i;
  
  ipv4_addr_t aux;
  
  message_ptr->command = 1;
  message_ptr->version = 2;
  
  
  
  for (i=0; i<num_entries; i++){
    
     message_ptr->entry[i].family_id =  htons(2);
     message_ptr->entry[i].route_tag = 0;
     message_ptr->entry[i].metric = 0;
     ipv4_str_addr (entry_array[i], aux);
     
     memcpy (message_ptr->entry[i].ip_addr, aux, IPv4_ADDR_SIZE);
  
     free (entry_array[i]);
    
  }
  

}

void create_response( int option ) {

    if (option == 2){

        ipv4_addr_t addr;

        message_ptr->command = 2;
        message_ptr->version = 2;
        message_ptr->entry[0].family_id = htons(2);
        message_ptr->entry[0].route_tag = 0;
        message_ptr->entry[0].metric = htonl (12);
        ipv4_str_addr ("10.0.2.15", addr);
        memcpy (message_ptr->entry[0].ip_addr, addr, IPv4_ADDR_SIZE);
        ipv4_str_addr ("255.255.255.0", addr);
        memcpy (message_ptr->entry[0].ip_mask, addr, IPv4_ADDR_SIZE);
        ipv4_str_addr ("0.0.0.0", addr);
        memcpy (message_ptr->entry[0].ip_next, addr, IPv4_ADDR_SIZE);
        message_ptr->entry[1].family_id = htons(2);
        message_ptr->entry[1].route_tag = 0;
        message_ptr->entry[1].metric = htonl (3);
        ipv4_str_addr ("10.0.3.15", addr);
        memcpy (message_ptr->entry[1].ip_addr, addr, IPv4_ADDR_SIZE);
        ipv4_str_addr ("255.255.255.255", addr);
        memcpy (message_ptr->entry[1].ip_mask, addr, IPv4_ADDR_SIZE);
        ipv4_str_addr ("10.0.6.2", addr);
        memcpy (message_ptr->entry[1].ip_next, addr, IPv4_ADDR_SIZE);
        message_ptr->entry[2].family_id = htons(2);
        message_ptr->entry[2].route_tag = 0;
        message_ptr->entry[2].metric = htonl (4);
        ipv4_str_addr ("10.0.0.6", addr);
        memcpy (message_ptr->entry[2].ip_addr, addr, IPv4_ADDR_SIZE);
        ipv4_str_addr ("255.0.0.0", addr);
        memcpy (message_ptr->entry[2].ip_mask, addr, IPv4_ADDR_SIZE);
        ipv4_str_addr ("10.0.5.15", addr);
        memcpy (message_ptr->entry[2].ip_next, addr, IPv4_ADDR_SIZE);
        message_ptr->entry[3].family_id = htons(2);
        message_ptr->entry[3].route_tag = 0;
        message_ptr->entry[3].metric = htonl (8);
        ipv4_str_addr ("10.0.2.7", addr);
        memcpy (message_ptr->entry[3].ip_addr, addr, IPv4_ADDR_SIZE);
        ipv4_str_addr ("255.255.0.0", addr);
        memcpy (message_ptr->entry[3].ip_mask, addr, IPv4_ADDR_SIZE);
        ipv4_str_addr ("10.0.6.15", addr);
        memcpy (message_ptr->entry[3].ip_next, addr, IPv4_ADDR_SIZE);

    } else if (option == 3) {


        ipv4_addr_t addr;

        message_ptr->command = 2;
        message_ptr->version = 2;
        message_ptr->entry[0].family_id = htons(2);
        message_ptr->entry[0].route_tag = 0;
        message_ptr->entry[0].metric = htonl (2);
        ipv4_str_addr ("10.0.2.15", addr);
        memcpy (message_ptr->entry[0].ip_addr, addr, IPv4_ADDR_SIZE);
        ipv4_str_addr ("255.255.255.0", addr);
        memcpy (message_ptr->entry[0].ip_mask, addr, IPv4_ADDR_SIZE);
        ipv4_str_addr ("0.0.0.0", addr);
        memcpy (message_ptr->entry[0].ip_next, addr, IPv4_ADDR_SIZE);
        message_ptr->entry[1].family_id = htons(2);
        message_ptr->entry[1].route_tag = 0;
        message_ptr->entry[1].metric = htonl (2);
        ipv4_str_addr ("10.0.3.15", addr);
        memcpy (message_ptr->entry[1].ip_addr, addr, IPv4_ADDR_SIZE);
        ipv4_str_addr ("255.255.255.255", addr);
        memcpy (message_ptr->entry[1].ip_mask, addr, IPv4_ADDR_SIZE);
        ipv4_str_addr ("10.0.6.2", addr);
        memcpy (message_ptr->entry[1].ip_next, addr, IPv4_ADDR_SIZE);
        message_ptr->entry[2].family_id = htons(2);
        message_ptr->entry[2].route_tag = 0;
        message_ptr->entry[2].metric = htonl (2);
        ipv4_str_addr ("10.0.0.6", addr);
        memcpy (message_ptr->entry[2].ip_addr, addr, IPv4_ADDR_SIZE);
        ipv4_str_addr ("255.0.0.0", addr);
        memcpy (message_ptr->entry[2].ip_mask, addr, IPv4_ADDR_SIZE);
        ipv4_str_addr ("10.0.5.15", addr);
        memcpy (message_ptr->entry[2].ip_next, addr, IPv4_ADDR_SIZE);
        message_ptr->entry[3].family_id = htons(2);
        message_ptr->entry[3].route_tag = 0;
        message_ptr->entry[3].metric = htonl (2);
        ipv4_str_addr ("10.0.2.7", addr);
        memcpy (message_ptr->entry[3].ip_addr, addr, IPv4_ADDR_SIZE);
        ipv4_str_addr ("255.255.0.0", addr);
        memcpy (message_ptr->entry[3].ip_mask, addr, IPv4_ADDR_SIZE);
        ipv4_str_addr ("10.0.6.15", addr);
        memcpy (message_ptr->entry[3].ip_next, addr, IPv4_ADDR_SIZE);


    } else if (option == 4){

        ipv4_addr_t addr;

        message_ptr->command = 2;
        message_ptr->version = 2;
        message_ptr->entry[0].family_id = htons(2);
        message_ptr->entry[0].route_tag = 0;
        message_ptr->entry[0].metric = htonl (3);
        ipv4_str_addr ("1.1.1.1", addr);
        memcpy (message_ptr->entry[0].ip_addr, addr, IPv4_ADDR_SIZE);
        ipv4_str_addr ("255.255.255.0", addr);
        memcpy (message_ptr->entry[0].ip_mask, addr, IPv4_ADDR_SIZE);
        ipv4_str_addr ("0.0.0.0", addr);
        memcpy (message_ptr->entry[0].ip_next, addr, IPv4_ADDR_SIZE);
        message_ptr->entry[1].family_id = htons(2);
        message_ptr->entry[1].route_tag = 0;
        message_ptr->entry[1].metric = htonl (16);
        ipv4_str_addr ("2.2.2.2", addr);
        memcpy (message_ptr->entry[1].ip_addr, addr, IPv4_ADDR_SIZE);
        ipv4_str_addr ("255.255.255.255", addr);
        memcpy (message_ptr->entry[1].ip_mask, addr, IPv4_ADDR_SIZE);
        ipv4_str_addr ("3.3.3.3", addr);
        memcpy (message_ptr->entry[1].ip_next, addr, IPv4_ADDR_SIZE);
        message_ptr->entry[2].family_id = htons(2);
        message_ptr->entry[2].route_tag = 0;
        message_ptr->entry[2].metric = htonl (15);
        ipv4_str_addr ("4.5.5.5", addr);
        memcpy (message_ptr->entry[2].ip_addr, addr, IPv4_ADDR_SIZE);
        ipv4_str_addr ("255.0.0.0", addr);
        memcpy (message_ptr->entry[2].ip_mask, addr, IPv4_ADDR_SIZE);
        ipv4_str_addr ("10.0.5.15", addr);
        memcpy (message_ptr->entry[2].ip_next, addr, IPv4_ADDR_SIZE);
        message_ptr->entry[3].family_id = htons(2);
        message_ptr->entry[3].route_tag = 0;
        message_ptr->entry[3].metric = htonl (2);
        ipv4_str_addr ("10.0.2.7", addr);
        memcpy (message_ptr->entry[3].ip_addr, addr, IPv4_ADDR_SIZE);
        ipv4_str_addr ("255.255.0.0", addr);
        memcpy (message_ptr->entry[3].ip_mask, addr, IPv4_ADDR_SIZE);
        ipv4_str_addr ("10.0.6.15", addr);
        memcpy (message_ptr->entry[3].ip_next, addr, IPv4_ADDR_SIZE);
    }
}

/*
* Previous functions are auxiliars to initialize messages to
* try the right behaviour of the program
*/

int main(int argc , char *argv[]) {


    int option = 0;
    int user_num_entries = 0;
    
    char entry_buffer [120];

    int random_port = random_number(1024,  65535);
    
    char * entry_array [RIP_ROUTE_TABLE_SIZE];
    ipv4_addr_t dst;
    ipv4_addr_t src;
    int bytes_sent;
    int bytes_recv;
    int src_port;
    
    ipv4_addr_t aux;

    if(argc != 2 && argc != 3) {

    printf("Usage: rip_client <ip_address>\n");
	printf("Specific entries usage: rip_client <ip_address> -X\n");
        return -1;
    }

    ipv4_str_addr(argv[1], dst);

    rip_table_t * table = rip_table_create();

    if (initialize_rip (table, random_port) == -1) {

        return -1;
    }

    for ( ;; ) {
        
        if (argc == 2){
      
        bold("Please enter an option:\n");
        printf("1. Send a request\n");
        printf("2. Send a response with 4 entries\n");
        printf("3. Send same response with other costs\n");
        printf("4. Send anooother response with 4 items\n");
        printf(">");
        scanf("%u",&option);

        if (option != 1 && option != 2 && option != 3 && option != 4){
            print_alert ("Insert option 1, 2, 3 or 4!!\n");
            option = 0;
        }
        
	}else if (argc == 3 && !strcmp ("-X", argv [2]) && ipv4_str_addr (argv [1], aux) != -1){
	  
	  	bold("Please insert number of entries to ask for\n");  
		printf(">");
                scanf("%u",&user_num_entries);
		
		int k;
		for (k=0; k<user_num_entries; k++){
		  
		  bzero (entry_buffer, 120);
		  printf("Entry number %d: ", k+1);
		  scanf("%s", entry_buffer);
		
		  
		  if (ipv4_str_addr (entry_buffer, aux) == -1){
		  print_warning ("Wrong IPv4 address inserted\n");
		  k = k - 1;
		  }else
		  entry_array [k] = (char *) malloc (strlen (entry_buffer));
		  strcpy (entry_array [k], entry_buffer);
		  //REMEMBER TO FREEEEEEEEEE
		}
        printf("\n");
	    //FOR SPECIFIC ENTRIES
            option = 5;
	}else{
	   printf("Usage: rip_client <ip_address>\n");
	   printf("Specific entries usage: rip_client <ip_address> -X\n");
           return -1;
	  
	}
        

        if (option == 1) {
            
            create_request ( );
            print_notice("Sending a request...");
            bytes_sent = rip_send(dst, message_ptr, 1, RIP_PORT);

            bytes_recv = rip_recv( src, message_ptr, INFINITE_TIMER, &src_port);

            if ( bytes_recv < 0) {
                print_alert("bytes_recv -1 \n");
                return -1;
            } else if ( bytes_recv == 0 ) {
                print_warning("Nobody answered\n");
            }else{
                table = convert_message_table ( message_ptr, 
                rip_number_entries(bytes_recv));
                rip_route_table_print ( table );
            }

        } else if( option > 1 && option <= 4) {
            create_response ( option );
            bytes_sent = rip_send(dst, message_ptr, 4, RIP_PORT);
        } else if ( option == 5){
	    
         bold ("Sending a request...\n");

         if (user_num_entries <= 25){
             create_specific_request (entry_array, user_num_entries);
             bytes_sent = rip_send(dst, message_ptr, 1, RIP_PORT);
             bytes_recv = rip_recv( src, message_ptr, INFINITE_TIMER, &src_port);
             table = convert_message_table ( message_ptr, 
                rip_number_entries(bytes_recv));

         }else{

             create_specific_request (entry_array, 25);
             bytes_sent = rip_send(dst, message_ptr, 1, RIP_PORT);
             bytes_recv = rip_recv( src, message_ptr, INFINITE_TIMER, &src_port);
             table = convert_message_table ( message_ptr, 
                rip_number_entries(bytes_recv));
             create_specific_request (entry_array+25, user_num_entries - 25);
             bytes_sent = bytes_sent + rip_send(dst, message_ptr, 1, RIP_PORT);
             bytes_recv = rip_recv( src, message_ptr, INFINITE_TIMER, &src_port);
             add_entries_table (table, message_ptr, 
                rip_number_entries(bytes_recv));
  
         }
            if ( bytes_recv < 0) {
                print_alert("bytes_recv -1 \n");
                return -1;
            } else if ( bytes_recv == 0 ) {
                print_warning("Nobody answered\n");
            }else{
                table = convert_message_table ( message_ptr, 
                rip_number_entries(bytes_recv));
                rip_route_table_print ( table );
            }
	  
	}

        option = 0; //so we set to 0 option and avoid other acts
    }

    return 1;
}
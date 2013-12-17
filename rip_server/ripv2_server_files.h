#ifndef _RIPV2_SERVER_FILES_H
#define _RIPV2_SERVER_FILES_H

#include "../files/ripv2.h"

/* alias timerms_reset */
timerms_t timer_start (long int countdown);

/* alias timerms_left */
int timer_ended (timerms_t timer);
void send_update (rip_header_ptr pointer, int num_entries, int timer_ended);
void create_rip_message (rip_header_ptr pointer, rip_table_t * table);
void create_two_rip_message (rip_header_ptr pointer, rip_header_ptr pointer_two, rip_table_t * table);
void send_response (ipv4_addr_t dst, rip_header_ptr pointer, int num_entries, int dst_port);
void initialize_garbage_collector_timers (timerms_t * timer);
int garbage_collector_start (rip_table_t * table, timerms_t * timer);
int garbage_collector (rip_table_t * table, timerms_t * timer);

rip_table_t * table_to_send (rip_table_t * table, rip_table_t * table_aux);

#endif /* _RIPV2_SERVER_FILES_H */
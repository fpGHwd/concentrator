/*
 * th_downcomm.c
 *
 *  Created on: 2015-8-11
 *      Author: Johnnyzhang
 */

#include "threads.h"
#include <stdbool.h>
#include "main.h"

void *th_downcomm(void * arg) {
	while (!g_terminated) {
		sleep(1);
	}
	return NULL;
}

/*
 #define AFFAIR_QUEUE_MAX_COUNT 100;

 struct affairs_queue_s{
 int current_size_of_queue;
 message_t message[AFFAIR_QUEUE_MAX_COUNT];
 }

 // ---------------------------------
 static bool affairs_ready = false;
 struct affairs_queue_s affairs_queue;
 void *th_downcomm(void * arg)
 {
 down_communication_initiate();

 while(!g_terminated){
 if(affairs_ready){
 affairs_handdle();
 }
 sleep(1);
 }
 return NULL;
 }

 void read_a_meter(char *meter_id){
 /// meter_id;
 /// pack the read meter message
 /// send message
 /// get the message
 /// display the result of reading
 }

 void affairs_handle(void)
 {
 /// check the affairs queue
 }

 void down_communication_initiate(void){
 /// create the queue

 /// initiate the queue
 }
 */
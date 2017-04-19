/*
 * threads.h
 *
 *  Created on: 2015-8-12
 *      Author: Johnnyzhang
 */

#ifndef THREADS_H_
#define THREADS_H_

#include "typedef.h"

void *th_upgprscdma(void * arg);
void *th_upeth(void * arg);
void *th_downcomm(void * arg);
void *th_hmisys(void * arg);
void *th_gasmeter(void * arg);
void *th_gasmeter_event(void * arg);
void *th_alarm(void * arg);

void threads_create(void);
void threads_join(void);
int which_thread(void);

void init_watchdog(void);
int kill_watchdog(void);
void notify_watchdog(void);
const char *get_thread_name(void);
void print_thread_info(void);

/* add by wd*/
bool check_thread(char *th_name);

#endif /* THREADS_H_ */

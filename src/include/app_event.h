/*
 * app_event.h
 *
 *  Created on: 2015-8-28
 *      Author: Johnnyzhang
 */

#ifndef APP_EVENT_H_
#define APP_EVENT_H_

#include "typedef.h"

typedef struct {
	pthread_mutex_t mutex; /// mutex
	pthread_cond_t cond; /// pthread condition
	UINT32 event; /// 4bytes event
} app_event_t; ///

void app_event_init(app_event_t *pctrl);
void app_event_wait(app_event_t *pctrl, INT32 bwait, UINT32 waitmask,
		UINT32 *pevent);
void app_event_send(app_event_t *pctrl, UINT32 event);

#define app_mutex_t 		pthread_mutex_t
#define app_mutex_init(p)	pthread_mutex_init(p, NULL)
#define app_mutex_lock		pthread_mutex_lock
#define app_mutex_unlock	pthread_mutex_unlock

#define app_sleep(ticks)    usleep((UINT32)(ticks)*10000)

#endif /* APP_EVENT_H_ */

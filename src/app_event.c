/*
 * app_event.c
 *
 *  Created on: 2015-8-28
 *      Author: Johnnyzhang
 */

#include "app_event.h"
#include "main.h"

void app_event_init(app_event_t *pctrl) {
	pthread_mutex_init(&pctrl->mutex, NULL);
	pthread_cond_init(&pctrl->cond, NULL);
	pctrl->event = 0;
}

void app_event_wait(app_event_t *pctrl, INT32 bwait, UINT32 waitmask,
		UINT32 *pevent) {
	UINT32 ul;

	pthread_mutex_lock(&pctrl->mutex); //// system call

	ul = pctrl->event;

	if (!bwait) {
		if (ul & waitmask) {
			ul &= waitmask;
			pctrl->event &= ~waitmask; //// 
		} else
			ul = 0;
	} else {
		while (!g_terminated) {
			if (ul & waitmask) {
				ul &= waitmask;
				pctrl->event &= ~waitmask;
				break;
			}

			pthread_cond_wait(&pctrl->cond, &pctrl->mutex);
			ul = pctrl->event;
		}
	}

	pthread_mutex_unlock(&pctrl->mutex);

	*pevent = ul;
}

void app_event_send(app_event_t *pctrl, UINT32 event) {
	pthread_mutex_lock(&pctrl->mutex);
	pctrl->event |= event;
	pthread_cond_signal(&pctrl->cond);
	pthread_mutex_unlock(&pctrl->mutex);
}

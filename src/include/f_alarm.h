/*
 * f_alarm.h
 *
 *  Created on: 2015-9-4
 *      Author: Johnnyzhang
 */

#ifndef INCLUDE_F_ALARM_H_
#define INCLUDE_F_ALARM_H_

#include "typedef.h"
#include "spont_alarm.h"

typedef enum {
	ALARM_SPONT_STATUS_UNSEND, ALARM_SPONT_STATUS_SENT,
} ALARM_SPONT_STATUS;

typedef struct {
	long alarm_tt;
	WORD type;
	ALARM_SPONT_STATUS status[MAX_SPONT_CHANNEL_CNT];
} ALARM_T;

#endif /* INCLUDE_F_ALARM_H_ */

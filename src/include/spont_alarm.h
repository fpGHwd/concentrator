/*
 * spont_alarm.h
 *
 *  Created on: 2015-9-5
 *      Author: Johnnyzhang
 */

#ifndef INCLUDE_SPONT_ALARM_H_
#define INCLUDE_SPONT_ALARM_H_

#include "typedef.h"

#define MAX_SPONT_CHANNEL_CNT 5

extern int g_spont_cnt;

typedef enum {
	e_up_spont_unknown,
	e_up_spont_conalm,
	e_up_spont_gasmeteralm,
	e_up_spont_device_alarm, /// added by wd, devices or modules alarm when need
} E_SPONT_TYPE;

typedef struct {
	E_SPONT_TYPE type;
	int alarmidx;
} SPONT_INFO;

int spontalarm_register_channel(const char *name);
int spontalarm_get_data(BYTE *buf, int maxlen, int chnidx, WORD *pfn);
void spontalarm_reset_info(int chnidx);
void spontalarm_set_response(WORD fn, int chnidx, BYTE *data, int len);

#endif /* INCLUDE_SPONT_ALARM_H_ */

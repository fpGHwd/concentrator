/*
 * f_con_alarm.h
 *
 *  Created on: 2015-8-11
 *      Author: Johnnyzhang
 */

#ifndef F_CON_ALARM_H_
#define F_CON_ALARM_H_

#include "typedef.h"
#include "f_alarm.h"

//#define F_CON_ALARM_NAME "f_conalarm.dat"
#define F_CON_ALARM_NAME "../data/f_conalarm.dat"

#define MAX_CON_ALARM_CNT 1000

typedef enum {
	CON_ALARM_TYPE_FORCE_VALVAOFF = 1001, /// valve_off
	CON_ALARM_TYPE_GAS_LEAK = 1002,
	CON_ALARM_TYPE_REED_BAD = 1003,
	CON_ALARM_TYPE_STORAGE_BAD = 1004,
	CON_ALARM_TYPE_MAGNETIC_DISTURB = 1005,
	CON_ALARM_TYPE_DIE_METER = 1006,
	CON_ALARM_TYPE_VALVA_BAD = 1007,
	CON_ALARM_TYPE_CANCEL_FORCEOFF = 1008,
	CON_ALARM_TYPE_BATTERY_BAD = 1009,
} CON_ALARM_TYPE;

typedef struct {
	BOOL valid;
	BYTE address[5];
	BYTE collector[5];
	ALARM_T data;
} CON_ALARM_T;

void fconalm_open(void);
void fconalm_close(void);
int fconalm_changed(void);
int fconalm_add(const CON_ALARM_T *palarm);
BOOL fconalm_get_data(int alarmidx, CON_ALARM_T *palarm, int chnidx);

#endif /* F_CON_ALARM_H_ */

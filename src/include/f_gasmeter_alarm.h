/*
 * f_gasmeter_alarm.h
 *
 *  Created on: 2015-8-11
 *      Author: Johnnyzhang
 */

#ifndef F_GASMETER_ALARM_H_
#define F_GASMETER_ALARM_H_

#include "typedef.h"
#include "f_alarm.h"

//#define F_GASMETER_ALARM_NAME "f_gasmeter_alarm.dat"
#define F_GASMETER_ALARM_NAME "/opt/concentrator/data/f_gasmeter_alarm.dat"

#define MAX_GASMETER_ALARM_CNT 5000

typedef enum {
	GASMETER_ALARM_TYPE_FORCE_VALVAOFF = 1001,
	GASMETER_ALARM_TYPE_GAS_LEAK = 1002,
	GASMETER_ALARM_TYPE_REED_BAD = 1003,
	GASMETER_ALARM_TYPE_STORAGE_BAD = 1004,
	GASMETER_ALARM_TYPE_MAGNETIC_DISTURB = 1005,
	GASMETER_ALARM_TYPE_DIE_METER = 1006,
	GASMETER_ALARM_TYPE_VALVA_BAD = 1007,
	GASMETER_ALARM_TYPE_CANCEL_FORCEOFF = 1008,
	GASMETER_ALARM_TYPE_BATTERY_BAD = 1009,
} GASMETER_ALARM_TYPE;

typedef struct {
	BOOL valid;
	BYTE address[5];
	BYTE collector[5];
	BYTE meterid[7];
	ALARM_T data;
} GASMETER_ALARM_T;

void fgasmeteralm_open(void);
void fgasmeteralm_close(void);
int fgasmeteralm_changed(void);
int fgasmeteralm_add(const GASMETER_ALARM_T *palarm);
BOOL fgasmeteralm_get_data(int alarmidx, GASMETER_ALARM_T *palarm, int chnidx);

// add by wd 
int reset_fgasmeteralm_data(void);

#endif /* F_GASMETER_ALARM_H_ */

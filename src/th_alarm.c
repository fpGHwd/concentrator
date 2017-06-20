/*
 * th_alarm.c
 *
 *  Created on: 2015-8-11
 *      Author: Johnnyzhang
 */

#include "threads.h"
#include "common.h"
#include "main.h"
#include "f_param.h"
#include "f_gasmeter.h"
#include "f_con_alarm.h"
#include "f_gasmeter_alarm.h"
#include "protocol_cjt188_data.h"
#include "f_current.h"
#include "lcd.h"

#define INTERVAL_TIME_MS (100u)

static void conalm_check_battery_bad(void)
{
	CON_ALARM_T alarm;
	BYTE address[7];
	int i;

	if (!check_rtc()) {
		fparam_get_value(FPARAMID_CON_ADDRESS, address, sizeof(address));
		memcpy(alarm.address, &address[3], 5);
		memset(alarm.address, 0, 5);
		time(&alarm.data.alarm_tt);
		alarm.data.type = CON_ALARM_TYPE_BATTERY_BAD;
		for (i = 0; i < MAX_SPONT_CHANNEL_CNT; i++) {
			alarm.data.status[i] = ALARM_SPONT_STATUS_UNSEND;
		}
		fconalm_add(&alarm);
	}
}

static void gasmeteralm_check_gas_leak(int mtidx)
{
	GASMETER_ALARM_T alarm;
	GASMETER_CJT188_901F di_data;
	BYTE st[2];
	BYTE con_addr[7], collector[5], meterid[7];
	int i;

	if (!fcurrent_get_data(mtidx, 0x901F, &di_data))
		return;
	memcpy(st, di_data.di_data.st, 2);
	if (st[0] & (1 << 3)) { /// 0000 0000 & 0000 1000
		fparam_get_value(FPARAMID_CON_ADDRESS, con_addr, sizeof(con_addr));
		memcpy(alarm.address, &con_addr[3], 5);
		fgasmeter_getgasmeter(mtidx, meterid, collector);
		memcpy(alarm.collector, collector, 5);
		memcpy(alarm.meterid, meterid, 7);
		time(&alarm.data.alarm_tt);
		alarm.data.type = GASMETER_ALARM_TYPE_GAS_LEAK;
		for (i = 0; i < MAX_SPONT_CHANNEL_CNT; i++) {
			alarm.data.status[i] = ALARM_SPONT_STATUS_UNSEND;
		} /// get the alarm
		fgasmeteralm_add(&alarm);
	}
}

static void gasmeteralm_check_reed_bad(int mtidx) {
	GASMETER_ALARM_T alarm;
	GASMETER_CJT188_901F di_data;
	BYTE st[2];
	BYTE con_addr[7], collector[5], meterid[7];
	int i;

	if (!fcurrent_get_data(mtidx, 0x901F, &di_data))
		return;
	memcpy(st, di_data.di_data.st, 2);
	if (st[0] & (1 << 7)) {
		fparam_get_value(FPARAMID_CON_ADDRESS, con_addr, sizeof(con_addr));
		memcpy(alarm.address, &con_addr[3], 5);
		fgasmeter_getgasmeter(mtidx, meterid, collector);
		memcpy(alarm.collector, collector, 5);
		memcpy(alarm.meterid, meterid, 7);
		time(&alarm.data.alarm_tt);
		alarm.data.type = GASMETER_ALARM_TYPE_REED_BAD;
		for (i = 0; i < MAX_SPONT_CHANNEL_CNT; i++) {
			alarm.data.status[i] = ALARM_SPONT_STATUS_UNSEND;
		}
		fgasmeteralm_add(&alarm);
	}
}

static void gasmeteralm_check_magnetic_disturb(int mtidx)
{
	GASMETER_ALARM_T alarm;
	GASMETER_CJT188_901F di_data;
	BYTE st[2];
	BYTE con_addr[7], collector[5], meterid[7];
	int i;

	if (!fcurrent_get_data(mtidx, 0x901F, &di_data))
		return;
	memcpy(st, di_data.di_data.st, 2);
	if (st[0] & (1 << 6)) {
		fparam_get_value(FPARAMID_CON_ADDRESS, con_addr, sizeof(con_addr));
		memcpy(alarm.address, &con_addr[3], 5);
		fgasmeter_getgasmeter(mtidx, meterid, collector);
		memcpy(alarm.collector, collector, 5);
		memcpy(alarm.meterid, meterid, 7);
		time(&alarm.data.alarm_tt);
		alarm.data.type = GASMETER_ALARM_TYPE_MAGNETIC_DISTURB;
		for (i = 0; i < MAX_SPONT_CHANNEL_CNT; i++) {
			alarm.data.status[i] = ALARM_SPONT_STATUS_UNSEND;
		}
		fgasmeteralm_add(&alarm);
	}
}

static void gasmeteralm_check_valve_bad(int mtidx)
{
	GASMETER_ALARM_T alarm;
	GASMETER_CJT188_901F di_data;
	BYTE st[2];
	BYTE con_addr[7], collector[5], meterid[7];
	int i;

	if (!fcurrent_get_data(mtidx, 0x901F, &di_data))
		return;
	memcpy(st, di_data.di_data.st, 2);
	if (((st[0] >> 6) & 0x03) == 0x03) { // fixme: (st[0] > 6) & 0x03 == 0x03 informal value status
		fparam_get_value(FPARAMID_CON_ADDRESS, con_addr, sizeof(con_addr));
		memcpy(alarm.address, &con_addr[3], 5);
		fgasmeter_getgasmeter(mtidx, meterid, collector);
		memcpy(alarm.collector, collector, 5);
		memcpy(alarm.meterid, meterid, 7);
		time(&alarm.data.alarm_tt);
		alarm.data.type = GASMETER_ALARM_TYPE_VALVE_BAD;
		for (i = 0; i < MAX_SPONT_CHANNEL_CNT; i++) {
			alarm.data.status[i] = ALARM_SPONT_STATUS_UNSEND;
		}
		fgasmeteralm_add(&alarm);
	}
}

static void gasmeteralm_check_battery_bad(int mtidx)
{
	GASMETER_ALARM_T alarm;
	GASMETER_CJT188_901F di_data;
	BYTE st[2];
	BYTE con_addr[7], collector[5], meterid[7];
	int i;

	if (!fcurrent_get_data(mtidx, 0x901F, &di_data))
		return;
	memcpy(st, di_data.di_data.st, 2);
	if (st[0] & (0x11 << 4)) {
		fparam_get_value(FPARAMID_CON_ADDRESS, con_addr, sizeof(con_addr));
		memcpy(alarm.address, &con_addr[3], 5);
		fgasmeter_getgasmeter(mtidx, meterid, collector);
		memcpy(alarm.collector, collector, 5);
		memcpy(alarm.meterid, meterid, 7);
		time(&alarm.data.alarm_tt);
		alarm.data.type = GASMETER_ALARM_TYPE_BATTERY_BAD;
		for (i = 0; i < MAX_SPONT_CHANNEL_CNT; i++) {
			alarm.data.status[i] = ALARM_SPONT_STATUS_UNSEND;
		}
		fgasmeteralm_add(&alarm);
	}
}

typedef struct {
	long last_tt;
	long interval_tt;
	void (*check_fn)(void);
	CON_ALARM_TYPE type;
} CON_ALARM_CONFIG;

typedef struct {
	long last_tt[MAX_GASMETER_ALARM_CNT];
	long interval_tt;
	void (*check_fn)(int);
	GASMETER_ALARM_TYPE type;
} GASMETER_ALARM_CONFIG;

static CON_ALARM_CONFIG con_alm_config[] = {
		{ 0, 24 * 3600, NULL, CON_ALARM_TYPE_FORCE_VALVAOFF },
		{ 0, 24 * 3600, NULL, CON_ALARM_TYPE_GAS_LEAK },
		{ 0, 24 * 3600, NULL, CON_ALARM_TYPE_REED_BAD },
		{ 0, 24 * 3600, NULL, CON_ALARM_TYPE_STORAGE_BAD },
		{ 0, 24 * 3600, NULL, CON_ALARM_TYPE_MAGNETIC_DISTURB },
		{ 0, 24 * 3600, NULL, CON_ALARM_TYPE_DIE_METER },
		{ 0, 24 * 3600, NULL, CON_ALARM_TYPE_VALVA_BAD },
		{ 0, 24 * 3600, NULL, CON_ALARM_TYPE_CANCEL_FORCEOFF },
		{ 0, 24 * 3600, conalm_check_battery_bad, CON_ALARM_TYPE_BATTERY_BAD },
};

static GASMETER_ALARM_CONFIG gasmeter_alm_config[] = {
		{ { 0 }, 24 * 3600, NULL, GASMETER_ALARM_TYPE_FORCE_VALVAOFF },
		{ { 0 }, 24 * 3600, gasmeteralm_check_gas_leak,GASMETER_ALARM_TYPE_GAS_LEAK },
		{ { 0 }, 24 * 3600, gasmeteralm_check_reed_bad, GASMETER_ALARM_TYPE_REED_BAD },
		{ { 0 }, 24 * 3600, NULL, GASMETER_ALARM_TYPE_STORAGE_BAD },
		{ { 0 }, 24 * 3600, gasmeteralm_check_magnetic_disturb, GASMETER_ALARM_TYPE_MAGNETIC_DISTURB },
		{ { 0 }, 24 * 3600, NULL, GASMETER_ALARM_TYPE_DIE_METER },
		{ { 0 }, 24 * 3600, gasmeteralm_check_valve_bad, GASMETER_ALARM_TYPE_VALVE_BAD },
		{ { 0 }, 24 * 3600, NULL, GASMETER_ALARM_TYPE_CANCEL_FORCEOFF },
		{ { 0 }, 24 * 3600, gasmeteralm_check_battery_bad, GASMETER_ALARM_TYPE_BATTERY_BAD }
};

static void conalm_init(void)
{
	CON_ALARM_T alarm;
	int i, j;
	CON_ALARM_CONFIG *pcfg;

	for (i = 0; i < MAX_CON_ALARM_CNT; i++) {
		if (!fconalm_get_data(i, &alarm, -1))
			continue;
		for (j = 0; j < ARRAY_SIZE(con_alm_config); j++) {
			pcfg = &con_alm_config[j];
			if (!pcfg->check_fn && alarm.valid && alarm.data.type == pcfg->type
			&& alarm.data.alarm_tt > pcfg->last_tt) {
				pcfg->last_tt = alarm.data.alarm_tt;
			}
		}
	}
}

static void conalm_check(void) {
	int i;
	long tt;
	CON_ALARM_CONFIG *pcfg;

	time(&tt);
	for (i = 0; i < ARRAY_SIZE(con_alm_config); i++) {
		pcfg = &con_alm_config[i];
		if (tt > pcfg->last_tt && tt - pcfg->last_tt > pcfg->interval_tt
		&& pcfg->check_fn) {
			pcfg->check_fn();
			pcfg->last_tt = time(NULL);
		}
	}
}

static void gasmeteralm_init(void) {
	GASMETER_ALARM_T alarm;
	int i, j;
	GASMETER_ALARM_CONFIG *pcfg;
	int mtidx;

	for (i = 0; i < MAX_CON_ALARM_CNT; i++) {
		if (!fgasmeteralm_get_data(i, &alarm, -1))
			continue;
		for (j = 0; j < ARRAY_SIZE(con_alm_config); j++) {
			pcfg = &gasmeter_alm_config[j];
			if (!alarm.valid)
				continue;
			mtidx = fgasmeter_getidx_by_gasmeter(alarm.address);
			if (mtidx < 0)
				continue;
			if (alarm.data.type == pcfg->type
					&& alarm.data.alarm_tt > pcfg->last_tt[mtidx]) {
				pcfg->last_tt[mtidx] = alarm.data.alarm_tt;
			}
		}
	}
}

static void gasmeteralm_check(void) {
	int i, mtidx;
	long tt;
	GASMETER_ALARM_CONFIG *pcfg;

	time(&tt);
	for (i = 0; i < ARRAY_SIZE(con_alm_config); i++) {
		pcfg = &gasmeter_alm_config[i];
		for (mtidx = 0; mtidx < MAX_GASMETER_NUMBER; mtidx++) {
			if (!fgasmeter_getgasmeter(mtidx, NULL, NULL))
				continue;
			if (tt > pcfg->last_tt[mtidx]
					&& tt - pcfg->last_tt[mtidx] > pcfg->interval_tt
					&& pcfg->check_fn) {
				pcfg->check_fn(mtidx);
				pcfg->last_tt[mtidx] = time(NULL);
			}
		}
	}
}

void *th_alarm(void * arg)
{
	print_thread_info();
	conalm_init();
	gasmeteralm_init();
	while (!g_terminated) {
		notify_watchdog();
		conalm_check();
		gasmeteralm_check();
		lcd_update_head_info();

		msleep(INTERVAL_TIME_MS);
	}
	return NULL;
}

/*
 * spont_alarm.c
 *
 *  Created on: 2015-9-5
 *      Author: Johnnyzhang
 */

#include "spont_alarm.h"
#include "common.h"
#include "f_con_alarm.h"
#include "f_gasmeter_alarm.h"
#include "protocol_gasup.h"

int g_spont_cnt = 0;

typedef struct {
	BYTE used;
	char describe[64];
	SPONT_INFO spont_info;
} SPONT_CHANNEL_INFO;

static SPONT_CHANNEL_INFO spont_channel_info[MAX_SPONT_CHANNEL_CNT];

int spontalarm_register_channel(const char *name)
{
	int i, size;
	SPONT_CHANNEL_INFO *pinfo;

	if (name == NULL)
		return -1;
	for (i = 0; i < MAX_SPONT_CHANNEL_CNT; i++) {
		pinfo = &spont_channel_info[i];
		if (pinfo->used)
			continue;
		if (strcmp(pinfo->describe, name) == 0) {
			PRINTF("ERROR: The spont channel %s is used\n", name);
			return -1;
		}
		size = min(sizeof(pinfo->describe) - 1, strlen(name));
		memcpy(pinfo->describe, name, size);
		pinfo->describe[size] = 0;
		pinfo->used = TRUE;
		PRINTF("Register SPONT channel, index: %d, name: %s\n", i, name);
		g_spont_cnt++;
		return i;
	}
	return -1;
}

int spontalarm_get_data(BYTE *buf, int maxlen, int chnidx, WORD *pfn) {
	int i;
	CON_ALARM_T con_alarm;
	GASMETER_ALARM_T gasmeter_alarm;
	int len;
	SPONT_CHANNEL_INFO *pinfo;

	if (buf == NULL || maxlen <= 0 || pfn == NULL)
		return 0;
	if (chnidx < 0 || chnidx >= g_spont_cnt)
		return 0;
	for (i = 0; i < MAX_CON_ALARM_CNT; i++) {
		if (!fgasmeteralm_get_data(i, &gasmeter_alarm, -1))
			continue;
		len = ptl_gasup_pack_gasmeteralarm_data(buf, maxlen, pfn,
				&gasmeter_alarm);
		if (len > 0) {
			pinfo = &spont_channel_info[chnidx];
			pinfo->spont_info.type = e_up_spont_gasmeteralm;
			pinfo->spont_info.alarmidx = i;
		}
		return len;
	}
	for (i = 0; i < MAX_CON_ALARM_CNT; i++) {
		if (!fconalm_get_data(i, &con_alarm, -1))
			continue;
		len = ptl_gasup_pack_conalarm_data(buf, maxlen, pfn, &con_alarm);
		if (len > 0) {
			if (len > 0) {
				pinfo = &spont_channel_info[chnidx];
				pinfo->spont_info.type = e_up_spont_conalm;
				pinfo->spont_info.alarmidx = i;
			}
		}
		return len;
	}
	return 0;
}

void spontalarm_reset_info(int chnidx) {
	SPONT_CHANNEL_INFO *pinfo;

	if (chnidx < 0 || chnidx >= g_spont_cnt)
		return;
	pinfo = &spont_channel_info[chnidx];
	pinfo->spont_info.type = e_up_spont_unknown;
}

void spontalarm_set_response(WORD fn, int chnidx, BYTE *data, int len) {
	SPONT_CHANNEL_INFO *pinfo;

	if (chnidx < 0 || chnidx >= g_spont_cnt)
		return;
	pinfo = &spont_channel_info[chnidx];
	if (pinfo->spont_info.type == e_up_spont_unknown)
		return;
	switch (fn) {
	case PTL_GASUP_FN_ALM_METER:
		fgasmeteralm_get_data(pinfo->spont_info.alarmidx, NULL, chnidx);
		return;
	case PTL_GASUP_FN_ALM_CON:
		fconalm_get_data(pinfo->spont_info.alarmidx, NULL, chnidx);
		break;
	default:
		return;
	}
}

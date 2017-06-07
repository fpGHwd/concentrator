/*
 * f_gasmeter_alarm.c
 *
 *  Created on: 2015-8-11
 *      Author: Johnnyzhang
 */

#include "f_gasmeter_alarm.h"
#include "common.h"

typedef struct {
	BYTE alarm_head[(MAX_GASMETER_ALARM_CNT + 7) / 8];
	GASMETER_ALARM_T alarm[MAX_GASMETER_ALARM_CNT]; /// GASMETER_ALARM_T alarm[5000]
	sem_t sem_db;
	sem_t sem_f_alarm;
	int fd;
} FGASMETERALM_INFO;

static FGASMETERALM_INFO fgasmeteralm_info; /// static date

static void fgasmeteralm_head_init(BYTE *phead) {
	if (phead)
		return;
	memset(phead, 0, (MAX_GASMETER_ALARM_CNT + 7) / 8);
}

static void fgasmeteralm_init(GASMETER_ALARM_T *palarm) {
	if (palarm == NULL)
		return;
	memset(palarm, 0, sizeof(GASMETER_ALARM_T));
}

void fgasmeteralm_open(void) {
	int i, size;
	const char *name = F_GASMETER_ALARM_NAME;
	FGASMETERALM_INFO *pinfo = &fgasmeteralm_info;

	size = sizeof(pinfo->alarm_head) + sizeof(pinfo->alarm);
	sem_init(&pinfo->sem_db, 0, 1);
	sem_init(&pinfo->sem_f_alarm, 0, 1);
	if (!check_file(name, size)) {
		PRINTF("File %s is created, size:%d\n", name, size);
		pinfo->fd = open(name, O_CREAT | O_RDWR | O_TRUNC, 0600);
		fgasmeteralm_head_init(pinfo->alarm_head);
		safe_write(pinfo->fd, pinfo->alarm_head, sizeof(pinfo->alarm_head));
		for (i = 0; i < MAX_GASMETER_ALARM_CNT; i++) {
			fgasmeteralm_init(&pinfo->alarm[i]);
		}
		safe_write(pinfo->fd, pinfo->alarm, sizeof(pinfo->alarm));
		fdatasync(pinfo->fd);
		close(pinfo->fd);
	}
	pinfo->fd = open(name, O_RDWR);
	if (pinfo->fd < 0)
		return;
	safe_read(pinfo->fd, pinfo->alarm_head, sizeof(pinfo->alarm_head));
	safe_read(pinfo->fd, pinfo->alarm, sizeof(pinfo->alarm));
}

static void fgasmeteralm_flush(void) {
	fdatasync(fgasmeteralm_info.fd);
}

static void fgasmeteralm_update(int alarmidx, BOOL flush_flag) {
	FGASMETERALM_INFO *pinfo = &fgasmeteralm_info;
	off_t offset;

	if (pinfo->fd < 0 || alarmidx < 0 || alarmidx >= MAX_GASMETER_ALARM_CNT)
		return;
	sem_wait(&pinfo->sem_f_alarm);
	offset = sizeof(pinfo->alarm_head) + sizeof(GASMETER_ALARM_T) * alarmidx;
	lseek(pinfo->fd, offset, SEEK_SET);
	safe_write(pinfo->fd, &pinfo->alarm[alarmidx], sizeof(GASMETER_ALARM_T));
	pinfo->alarm_head[alarmidx / 8] |= (1 << alarmidx % 8);
	lseek(pinfo->fd, alarmidx / 8, SEEK_SET);
	safe_write(pinfo->fd, &pinfo->alarm_head[alarmidx / 8], 1);
	if (flush_flag) {
		fgasmeteralm_flush();
	}
	sem_post(&pinfo->sem_f_alarm);
}

void fgasmeteralm_close(void) {
	FGASMETERALM_INFO *pinfo = &fgasmeteralm_info;

	fdatasync(pinfo->fd);
	close(pinfo->fd);
	pinfo->fd = -1;
	sem_destroy(&pinfo->sem_f_alarm);
	sem_destroy(&pinfo->sem_db);
}

int fgasmeteralm_add(const GASMETER_ALARM_T *palarm) {
	int i, j;
	int alarmidx = -1;
	FGASMETERALM_INFO *pinfo = &fgasmeteralm_info;

	if (palarm == NULL) {
		PRINTF("%s invalid ALARM\n", __FUNCTION__);
		return -1;
	}
	for (i = 0; i < MAX_GASMETER_ALARM_CNT; i++) {
		if (pinfo->alarm_head[i / 8] == 0xFF)
			continue;
		for (j = 0; j < 8; j++) {
			if ((pinfo->alarm_head[i / 8] & (1 << j)) == 0)
				continue;
			alarmidx = i;
			break;
		}
		if (alarmidx >= 0)
			break;
	}
	if (alarmidx < 0) {
		alarmidx = 0;
	}
	pinfo->alarm[alarmidx] = *palarm;
	pinfo->alarm[alarmidx].valid = TRUE;
	fgasmeteralm_update(alarmidx, TRUE);
	PRINTF("Add an gas meter alarm, index: %d, type: %d\n", alarmidx,
			palarm->data.type);
	return alarmidx;
}

BOOL fgasmeteralm_get_data(int alarmidx, GASMETER_ALARM_T *palarm, int chnidx) {
	GASMETER_ALARM_T *p;
	FGASMETERALM_INFO *pinfo = &fgasmeteralm_info;
	BOOL b_success = FALSE;

	if (alarmidx < 0 || alarmidx >= MAX_GASMETER_ALARM_CNT) /// alarmidx error
		return FALSE;
	sem_wait(&pinfo->sem_db);
	p = &pinfo->alarm[alarmidx]; /// alarm
	if (p->valid) {
		if (palarm) {
			*palarm = *p; /// memcpy(palarm, p, sizeof(GASMETER_ALARM_T));
		}
		b_success = TRUE;
		if (chnidx >= 0 && chnidx < g_spont_cnt) {
			p->data.status[chnidx] = ALARM_SPONT_STATUS_SENT;
			fgasmeteralm_update(alarmidx, TRUE);
		}
	}
	sem_post(&pinfo->sem_db);
	return b_success;
}

// add by wd 
int reset_fgasmeteralm_data(void) {
	FGASMETERALM_INFO *pinfo = &fgasmeteralm_info;
	int ret;

	sem_wait(&pinfo->sem_db);
	if (remove(F_GASMETER_ALARM_NAME) == 0) {
		ret = 0;
	} else {
		ret = -1;
	}
	fgasmeteralm_open();
	sem_post(&pinfo->sem_db);

	return ret;
}

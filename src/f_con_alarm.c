/*
 * f_con_alarm.c
 *
 *  Created on: 2015-8-11
 *      Author: Johnnyzhang
 */

#include "f_con_alarm.h"
#include "common.h"
#include "spont_alarm.h"

typedef struct {
	BYTE alarm_head[(MAX_CON_ALARM_CNT + 7) / 8];
	CON_ALARM_T alarm[MAX_CON_ALARM_CNT];
	sem_t sem_db;
	sem_t sem_f_alarm;
	int fd;
} FCONALM_INFO;

static FCONALM_INFO fconalm_info;

static void fconalm_head_init(BYTE *phead) {
	if (phead) /// if head is not NULL, return
		return;
	memset(phead, 0, (MAX_CON_ALARM_CNT + 7) / 8);
}

static void fconalm_init(CON_ALARM_T *palarm) /// init a concentrator alarm data structure
{
	if (palarm == NULL)
		return;
	memset(palarm, 0, sizeof(CON_ALARM_T));
}

void fconalm_open(void) {
	int i, size;
	const char *name = F_CON_ALARM_NAME;
	FCONALM_INFO *pinfo = &fconalm_info;

	size = sizeof(pinfo->alarm_head) + sizeof(pinfo->alarm);
	sem_init(&pinfo->sem_db, 0, 1);
	sem_init(&pinfo->sem_f_alarm, 0, 1);
	if (!check_file(name, size)) {
		PRINTF("File %s is created, size:%d\n", name, size);
		pinfo->fd = open(name, O_CREAT | O_RDWR | O_TRUNC, 0600);
		fconalm_head_init(pinfo->alarm_head);
		safe_write(pinfo->fd, pinfo->alarm_head, sizeof(pinfo->alarm_head));
		for (i = 0; i < MAX_CON_ALARM_CNT; i++) {
			fconalm_init(&pinfo->alarm[i]);
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

static void fconalm_flush(void) {
	fdatasync(fconalm_info.fd);
}

static void fconalm_update(int alarmidx, BOOL flush_flag) {
	FCONALM_INFO *pinfo = &fconalm_info;
	off_t offset;

	if (pinfo->fd < 0 || alarmidx < 0 || alarmidx >= MAX_CON_ALARM_CNT)
		return;
	sem_wait(&pinfo->sem_f_alarm);
	offset = sizeof(pinfo->alarm_head) + sizeof(CON_ALARM_T) * alarmidx;
	lseek(pinfo->fd, offset, SEEK_SET);
	safe_write(pinfo->fd, &pinfo->alarm[alarmidx], sizeof(CON_ALARM_T));
	pinfo->alarm_head[alarmidx / 8] |= (1 << alarmidx % 8);
	lseek(pinfo->fd, alarmidx / 8, SEEK_SET);
	safe_write(pinfo->fd, &pinfo->alarm_head[alarmidx / 8], 1);
	if (flush_flag) {
		fconalm_flush();
	}
	sem_post(&pinfo->sem_f_alarm);
}

void fconalm_close(void) {
	FCONALM_INFO *pinfo = &fconalm_info;

	fdatasync(pinfo->fd);
	close(pinfo->fd);
	pinfo->fd = -1;
	sem_destroy(&pinfo->sem_f_alarm);
	sem_destroy(&pinfo->sem_db);
}

int fconalm_changed(void) {
	FCONALM_INFO *pinfo = &fconalm_info;
	CON_ALARM_T *p;
	BOOL changed = FALSE;
	int i, j;

	for (i = 0; i < MAX_CON_ALARM_CNT; i++) {
		p = &pinfo->alarm[i];
		if (!p->valid)
			continue;
		for (j = 0; j < g_spont_cnt; j++) {
			if (p->data.status[j] == ALARM_SPONT_STATUS_UNSEND) {
				changed = TRUE;
				break;
			}
		}
		if (changed)
			break;
	}
	return changed;
}

int fconalm_add(const CON_ALARM_T *palarm) {
	int i, j;
	int alarmidx = -1;
	FCONALM_INFO *pinfo = &fconalm_info;

	if (palarm == NULL) {
		PRINTF("%s invalid ALARM\n", __FUNCTION__);
		return -1;
	}
	for (i = 0; i < MAX_CON_ALARM_CNT; i++) {
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
	fconalm_update(alarmidx, TRUE);
	PRINTF("Add an concentrator alarm, index: %d, type: %d\n", alarmidx,
			palarm->data.type);
	return alarmidx;
}

BOOL fconalm_get_data(int alarmidx, CON_ALARM_T *palarm, int chnidx) {
	CON_ALARM_T *p;
	FCONALM_INFO *pinfo = &fconalm_info;
	BOOL b_success = FALSE;

	if (palarm == NULL || alarmidx < 0 || alarmidx >= MAX_CON_ALARM_CNT)
		return FALSE;
	sem_wait(&pinfo->sem_db);
	p = &pinfo->alarm[alarmidx];
	if (p->valid) {
		*palarm = *p;
		b_success = TRUE;
		if (chnidx >= 0 && chnidx < g_spont_cnt) {
			p->data.status[chnidx] = ALARM_SPONT_STATUS_SENT;
			fconalm_update(alarmidx, TRUE);
		}
	}
	sem_post(&pinfo->sem_db);
	return b_success;
}

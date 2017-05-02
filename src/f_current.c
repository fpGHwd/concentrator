/*
 * f_current.c
 *
 *  Created on: 2015-8-11
 *      Author: Johnnyzhang
 */

#include "f_current.h"
#include "common.h"

typedef struct {
	FCURRENT_DATA data[MAX_GASMETER_NUMBER];
	sem_t sem_db;
	sem_t sem_f_current;
	int fd;
} FCURRENT_INFO;

static FCURRENT_INFO fcurrent_info;

static void fcurrent_data_init(FCURRENT_DATA *pdata) {
	if (pdata == NULL)
		return;
	pdata->valid = FALSE;
	ptl_cjt188_data_init(0x901F, &pdata->u.data.data_901f);
}

void fcurrent_open(void) {
	int size;
	const char *name = F_CURRENT_NAME;
	FCURRENT_INFO *pinfo = &fcurrent_info;

	size = sizeof(FCURRENT_DATA) * MAX_GASMETER_NUMBER;
	sem_init(&pinfo->sem_db, 0, 1); /// what's mean'
	sem_init(&pinfo->sem_f_current, 0, 1);
	if (!check_file(name, size)) {
		///printf("f_current_open can find the file\n");
		PRINTF("File %s is created, size:%d\n", name, size);
		pinfo->fd = open(name, O_CREAT | O_RDWR | O_TRUNC, 0600);
		fcurrent_data_init(pinfo->data); /// init fcurrent_info
		safe_write(pinfo->fd, pinfo->data, size); /// write into file
		fdatasync(pinfo->fd); /// sync writing
		close(pinfo->fd); /// close file
	}
	pinfo->fd = open(name, O_RDWR);
	if (pinfo->fd < 0)
		return;
	safe_read(pinfo->fd, pinfo->data, size); /// close?
}

static void fcurrent_flush(void) {
	fdatasync(fcurrent_info.fd); /// flush fdatasync
}

static void fcurrent_update(int mtidx, BOOL flush_flag) {
	FCURRENT_INFO *pinfo = &fcurrent_info;
	off_t offset;

	sem_wait(&pinfo->sem_f_current);
	offset = mtidx * sizeof(FCURRENT_DATA); /// 
	lseek(pinfo->fd, offset, SEEK_SET);
	safe_write(pinfo->fd, &pinfo->data[mtidx], sizeof(FCURRENT_DATA)); /// write into file
	if (flush_flag) {
		///printf("current data update and flush\n");
		fcurrent_flush();
	}
	sem_post(&pinfo->sem_f_current);
}

void fcurrent_close(void) /// close file
{
	FCURRENT_INFO *pinfo = &fcurrent_info; /// from pinfo into the structure

	fdatasync(pinfo->fd);
	close(pinfo->fd);
	sem_destroy(&pinfo->sem_f_current);
	sem_destroy(&pinfo->sem_db);
}

BOOL fcurrent_set_data(int mtidx, WORD di, const void *di_data)
{
	FCURRENT_DATA *p;
	FCURRENT_INFO *pinfo = &fcurrent_info;
	BYTE address[7];
	BOOL b_success = FALSE;

	if (mtidx < 0 || mtidx >= MAX_GASMETER_NUMBER)
		return FALSE;
	if (!fgasmeter_getgasmeter(mtidx, address, NULL))
		return FALSE;
	sem_wait(&pinfo->sem_db);
	p = &pinfo->data[mtidx];
	p->valid = TRUE;
	memcpy(p->address, address, 7);
	switch (di) {
	case 0x901F:
		if (di_data) {
			memcpy(&p->u.data.data_901f, di_data, sizeof(GASMETER_CJT188_901F));
			p->u.data.data_901f.status = GASMETER_READ_STATUS_NORMAL;
		}
		fcurrent_update(mtidx, TRUE);
		b_success = TRUE;
		break;
	default:
		break;
	}
	sem_post(&pinfo->sem_db);
	return b_success;
}

BOOL fcurrent_set_status(int mtidx, WORD di, GASMETER_READ_STATUS status) {
	FCURRENT_DATA *p;
	FCURRENT_INFO *pinfo = &fcurrent_info;
	BYTE address[7];
	BOOL b_success = FALSE;

	if (mtidx < 0 || mtidx >= MAX_GASMETER_NUMBER)
		return FALSE;
	if (!fgasmeter_getgasmeter(mtidx, address, NULL))
		return FALSE;
	sem_wait(&pinfo->sem_db);
	p = &pinfo->data[mtidx];
	p->valid = TRUE;
	memcpy(p->address, address, 7);
	switch (di) {
	case 0x901F:
		if (p->u.data.data_901f.status == GASMETER_READ_STATUS_UNREAD) {
			p->u.data.data_901f.status = status;
		}
		fcurrent_update(mtidx, TRUE); /// write into file /// meter index
		b_success = TRUE;
		break;
	default:
		break;
	}
	sem_post(&pinfo->sem_db);
	return b_success;
}

BOOL fcurrent_get_data(int mtidx, WORD di, void *di_data) {
	FCURRENT_DATA *p;
	FCURRENT_INFO *pinfo = &fcurrent_info;
	BYTE address[7];
	BOOL b_success = FALSE;

	if (mtidx < 0 || mtidx >= MAX_GASMETER_NUMBER)
		return FALSE;
	if (!fgasmeter_getgasmeter(mtidx, address, NULL))
		return FALSE;
	sem_wait(&pinfo->sem_db); //// code is a place, is also a resource NEEDING LOCK MECHANISM TO MAKE SURE UNIQUE USING
	p = &pinfo->data[mtidx]; /// meaningful.....
	if (p->valid) { /// valid
		switch (di) {
		case 0x901F:
			if (memcmp(address, p->address, 7) == 0) {
				if (di_data) {
					memcpy(di_data, &p->u.data.data_901f,
							sizeof(GASMETER_CJT188_901F));
				}
				b_success = TRUE;
			}
			break;
		default:
			break;
		}
	}
	sem_post(&pinfo->sem_db);
	return b_success;
}

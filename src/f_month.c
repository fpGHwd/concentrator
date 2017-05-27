/*
 * f_month.c
 *
 *  Created on: 2015-8-11
 *      Author: Johnnyzhang
 */

#include "f_month.h"
#include "common.h"

typedef struct {
	FMON_DATA_BLOCK datablock[MAX_GASMETER_MON_CNT];  /// 10 years 120 month
	sem_t sem_db;
	sem_t sem_f_mon;
	int fd;
} FMON_INFO;

static FMON_INFO fmon_info;

static void fmon_data_init(FMON_DATA *pdata) {
	if (pdata == NULL)
		return;
	pdata->valid = FALSE;
	ptl_cjt188_data_init(0x901F, &pdata->u.data.data_901f);
}

static void fmon_block_init(FMON_DATA_BLOCK *pdata) {
	int i;

	if (pdata == NULL)
		return;
	for (i = 0; i < MAX_GASMETER_NUMBER; i++) {
		pdata->valid = FALSE;
		fmon_data_init(&pdata->mondata[i]);
	}
}

void fmon_open(void)
{
	int i, size;
	const char *name = F_MON_NAME;
	FMON_INFO *pinfo = &fmon_info;

	size = sizeof(FMON_DATA_BLOCK) * MAX_GASMETER_MON_CNT;
	sem_init(&pinfo->sem_db, 0, 1);
	sem_init(&pinfo->sem_f_mon, 0, 1);
	if (!check_file(name, size)) {
		PRINTF("File %s is created, size:%d\n", name, size);
		pinfo->fd = open(name, O_CREAT | O_RDWR | O_TRUNC, 0600);
		for (i = 0; i < MAX_GASMETER_MON_CNT; i++) {
			fmon_block_init(&pinfo->datablock[i]);
		}
		safe_write(pinfo->fd, pinfo->datablock, size);
		fdatasync(pinfo->fd);
		close(pinfo->fd);
	} else {
		;
	}

	pinfo->fd = open(name, O_RDWR);
	if (pinfo->fd < 0) {
		return; /// 不明确
	} else {

	}
	///printf("safe read file f_month.dat\n");
	///int size_test = 0;
	////size_test = 
	safe_read(pinfo->fd, pinfo->datablock, size); /// read file and save data info static fmon_info
	/// printf("if size_test(%d) == size(%d)\n", size_test, size);
}

static void fmon_flush(void) {
	fdatasync(fmon_info.fd);
}

static void fmon_update_monblock(int monidx, BOOL flush_flag) /// write monidx block to file
{
	FMON_INFO *pinfo = &fmon_info;
	off_t offset; /// off_t == int (signed integer)
	if (pinfo->fd < 0)
		return;
	sem_wait(&pinfo->sem_f_mon);
	///offset = monidx * sizeof(FMON_DATA_BLOCK) + offsetof(FMON_DATA_BLOCK, mondata); /// offsetof
	offset = monidx * sizeof(FMON_DATA_BLOCK);
	lseek(pinfo->fd, offset, SEEK_SET); /// seek for the inserting location of the file which file descriptor "pinfo->fd" points to
	safe_write(pinfo->fd, &pinfo->datablock[monidx], sizeof(FMON_DATA_BLOCK));
	if (flush_flag) {
		fmon_flush();
	}
	sem_post(&pinfo->sem_f_mon);
}

static void fmon_update(int monidx, int mtidx, BOOL flush_flag) /// write pinfo to f_month.dat 
{
	FMON_INFO *pinfo = &fmon_info;
	off_t offset;

	if (pinfo->fd < 0) /// can't securily access to the file which fd points to
		return;
	sem_wait(&pinfo->sem_f_mon);
	offset = monidx * sizeof(FMON_DATA_BLOCK)
			+ offsetof(FMON_DATA_BLOCK, mondata) + mtidx * sizeof(FMON_DATA); /// mtidx == meter index
	lseek(pinfo->fd, offset, SEEK_SET); /// set file pointer to here(offset)

	safe_write(pinfo->fd, &pinfo->datablock[monidx].mondata[mtidx],
			sizeof(FMON_DATA));  /// month - day, modified and it's OK

	if (flush_flag) {
		///printf("month data update and flush\n");
		fmon_flush();
	}
	sem_post(&pinfo->sem_f_mon);
}

void fmon_close(void) {
	FMON_INFO *pinfo = &fmon_info;

	fdatasync(pinfo->fd);
	close(pinfo->fd);
	pinfo->fd = -1;
	sem_destroy(&pinfo->sem_f_mon);
	sem_destroy(&pinfo->sem_db);
}

int fmon_create_monblock(void) /// 
{
	FMON_INFO *pinfo = &fmon_info;
	FMON_DATA_BLOCK *pblock = NULL;
	int i, monidx = -1;
	BOOL b_success = FALSE;

	sem_wait(&pinfo->sem_db); ///增加pinfo资源使用标记
	for (i = 0; i < MAX_GASMETER_MON_CNT; i++) { /// 12个月的块
		pblock = &pinfo->datablock[i];
		if (!pblock->valid) { /// 刚被初始化，还是false
			b_success = TRUE; /// 获得block成功
			///sem_post(&pinfo->sem_db);
			///return i;
			monidx = i;
			break;
		} else {
			continue;
		}
	}
	if (!b_success) { /// 如果所有的块都已创建并被使用 /// 其实就是找的最小的一个时间的块a，最后i的意义
		b_success = TRUE;
		monidx = 0;
		pblock = &pinfo->datablock[0];
		long min_tt = pblock->tt;
		for (i = 1; i < MAX_GASMETER_MON_CNT; i++) {
			pblock = &pinfo->datablock[i];
			if (pblock->tt < min_tt) { /// 时间被第一个块小的获得index
				monidx = i;
			}
		}
	}
	if (b_success) {
		pblock = &pinfo->datablock[monidx];
		fmon_block_init(pblock);
		pblock->valid = TRUE;
		time(&pblock->tt); /// 存储当前系统时间
		///printf("save now time and start to update monblock\n");
		fmon_update_monblock(monidx, TRUE); /// 写到文件中
	}
	sem_post(&pinfo->sem_db); /// 去掉pinfo资源使用标记
	return monidx;
}

BOOL fmon_set_data(int monidx, int mtidx, WORD di, const void *di_data) //// const传参进来后不能修改
{
	FMON_DATA *p;
	FMON_INFO *pinfo = &fmon_info;
	BYTE address[7];
	BOOL b_success = FALSE;
	///GASMETER_CJT188_901F* di_data_buf_p;

	if (monidx < 0 || monidx >= MAX_GASMETER_MON_CNT)
		return FALSE;
	if (mtidx < 0 || mtidx >= MAX_GASMETER_NUMBER)
		return FALSE;
	if (!fgasmeter_getgasmeter(mtidx, address, NULL))
		return FALSE;
	sem_wait(&pinfo->sem_db); /// 开始set data
	p = &pinfo->datablock[monidx].mondata[mtidx];

	p->valid = TRUE;
	memcpy(p->address, address, 7);
	switch (di) {
	case 0x901F:
		if (di_data) { /// if di_data is not NULL
			memcpy(&p->u.data.data_901f, di_data, sizeof(GASMETER_CJT188_901F));
			///printf("save month_data index: %d\n", monidx); ///
			p->u.data.data_901f.status = GASMETER_READ_STATUS_NORMAL;
		} else {
			;
		}
		/// added by wd
		///di_data_buf_p = (GASMETER_CJT188_901F*)di_data;
		///pinfo->datablock[monidx].tt = ((GASMETER_CJT188_901F*)di_data)->read_tt; 
		fmon_update(monidx, mtidx, TRUE); /// update to file
		b_success = TRUE;
		break;
	default:
		break;
	}

	sem_post(&pinfo->sem_db);
	return b_success;
}

BOOL fmon_get_data(int monidx, int mtidx, WORD di, void *di_data) /// get data with di
{
	FMON_DATA *p;
	FMON_INFO *pinfo = &fmon_info;
	BYTE address[7];
	BOOL b_success = FALSE;

	if (monidx < 0 || monidx >= MAX_GASMETER_MON_CNT)
		return FALSE;
	if (mtidx < 0 || mtidx >= MAX_GASMETER_NUMBER) /// 用来获取meter address
		return FALSE;
	if (!fgasmeter_getgasmeter(mtidx, address, NULL))
		return FALSE;
	sem_wait(&pinfo->sem_db);
	p = &pinfo->datablock[monidx].mondata[mtidx];
	/// printf("p->valid = %d\n", (p->valid)?1:0);
	if (p->valid) {
		switch (di) {
		case 0x901F: /// protocol
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

int fmon_get_datablock_index(void)
{
	int i, index = -1;
	struct tm cur_tm, mon_tm;
	FMON_INFO *pinfo = &fmon_info;

	sys_time(&cur_tm); /// write system time to cur_tm
	sem_wait(&pinfo->sem_db);
	for (i = 0; i < MAX_GASMETER_MON_CNT; i++) {
		localtime_r(&pinfo->datablock[i].tt, &mon_tm);
		if (pinfo->datablock[i].valid && cur_tm.tm_year == mon_tm.tm_year
				&& cur_tm.tm_mon == mon_tm.tm_mon) {
			index = i;
			break;
		}
	}
	sem_post(&pinfo->sem_db);
	return index;
}

int fmon_get_datablock_index_by_time(WORD year, BYTE month)
{
	int i, index = -1;
	struct tm mon_tm;
	FMON_INFO *pinfo = &fmon_info;

	sem_wait(&pinfo->sem_db);
	for (i = 0; i < MAX_GASMETER_MON_CNT; i++) {
		localtime_r(&pinfo->datablock[i].tt, &mon_tm);
		if (pinfo->datablock[i].valid && (year - 1900) == mon_tm.tm_year
				&& month == (mon_tm.tm_mon + 1)) {
			index = i;
			break;
		}
	}
	sem_post(&pinfo->sem_db);
	return index;
}

// add by wd

int fmon_block_success_sum(int mon_block_index) {
	unsigned short meter_idx;
	FMON_INFO *pinfo = &fmon_info;
	FMON_DATA *fd;
	int ret;

	ret = 0;
	if (pinfo->datablock[mon_block_index].valid) {
		for (meter_idx = 0; meter_idx < MAX_GASMETER_NUMBER; meter_idx++) {
			fd = &(pinfo->datablock[mon_block_index].mondata[meter_idx]);
			if (fd->valid) {
				ret++;
			}
		}
		return ret;
	} else {
		return 0;
	}
}

// add by wd 
int reset_fmonth_data(void) {
	FMON_INFO *pinfo = &fmon_info;
	int ret;

	sem_wait(&pinfo->sem_db);
	if (remove(F_MON_NAME) == 0) {
		ret = 0;
	} else {
		ret = -1;
	}
	fmon_open(); // open file
	sem_post(&pinfo->sem_db);

	return ret;
}

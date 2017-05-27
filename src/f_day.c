/*
 * f_day.c
 *
 *  Created on: 2015-8-11
 *      Author: Johnnyzhang
 */

#include "f_day.h"
#include "common.h"

typedef struct {
	FDAY_DATA_BLOCK datablock[MAX_GASMETER_DAY_CNT];
	sem_t sem_db;
	sem_t sem_f_day;
	int fd;
} FDAY_INFO;

static FDAY_INFO fday_info;

/*
 GASMETER_CJT188_DATA_901F /// 流量数据
 GASMETER_CJT188_DAY /// 读状态
 FDAY_DATA /// 表号，有效标志
 FDAY_DATA_BLOCK /// 有效标志
 FDAY_INFO /// 信号量 - 保护内存
 fday_info /// 文件签名
 */

static void fday_data_init(FDAY_DATA *pdata) {
	if (pdata == NULL)
		return;
	pdata->valid = FALSE;
	ptl_cjt188_data_init(0x901F, &pdata->u.data.data_901f);
}

static void fday_block_init(FDAY_DATA_BLOCK *pdata)
{
	int i;

	if (pdata == NULL)
		return;
	for (i = 0; i < MAX_GASMETER_NUMBER; i++) {
		pdata->valid = FALSE;
		fday_data_init(&pdata->daydata[i]);
	}
}

void fday_open(void)
{
	int i, size;
	const char *name = F_DAY_NAME;
	FDAY_INFO *pinfo = &fday_info;

	size = sizeof(FDAY_DATA_BLOCK) * MAX_GASMETER_DAY_CNT;  /// 60 days

	///f_day.dat size:4320480 // 60 /// 72008
	//// File f_month.dat is created, size:8640960

	sem_init(&pinfo->sem_db, 0, 1);
	sem_init(&pinfo->sem_f_day, 0, 1);
	if (!check_file(name, size)) { /// file not exits
		PRINTF("File %s is being created, size:%d\n", name, size);
		pinfo->fd = open(name, O_CREAT | O_RDWR | O_TRUNC, 0600);
		for (i = 0; i < MAX_GASMETER_DAY_CNT; i++) {
			fday_block_init(&pinfo->datablock[i]); /// initiate after open
		}
		safe_write(pinfo->fd, pinfo->datablock, size);
		fdatasync(pinfo->fd);
		close(pinfo->fd); /// why close fd, clear, because we need open downward
	} else {
		///PRINTF("File %s exists, and load file %s with size:%d\n", name, size); 
	}

	pinfo->fd = open(name, O_RDWR);
	if (pinfo->fd < 0) {
		///printf("fday_open open-2 error, return forward\n");
		return;
	} else if (pinfo->fd >= 0) {
		///printf("fday_open OK, read data from f_day.dat, this process needed\n");
	}
	//printf("fday_open OK, read data from f_day.dat, this process needed\n");
	///int size_1;
	///size_1 = 
	safe_read(pinfo->fd, pinfo->datablock, size); /// complete the logic and read file content into file variable(fday_info)
	///printf("fday_info read the file f_day.dat\n");
	///printf("size: %d, read size_1 = %d\n", size, size_1);

	/// need to close the fd ?
}

static void fday_flush(void) {
	fdatasync(fday_info.fd);  /// system call, ez /// flush data into files
}

static void fday_update_dayblock(int dayidx, BOOL flush_flag) /// day index == block index /// 
{
	FDAY_INFO *pinfo = &fday_info;
	off_t offset;

	if (pinfo->fd < 0)
		return;
	sem_wait(&pinfo->sem_f_day);
	///offset = dayidx * sizeof(FDAY_DATA_BLOCK) + offsetof(FDAY_DATA_BLOCK, daydata); /// offsetof - 获得daydata在FDAY_DATA_BLOCK的偏移量
	offset = dayidx * sizeof(FDAY_DATA_BLOCK); /// modified by wd
	lseek(pinfo->fd, offset, SEEK_SET); /// lseek改变相关文件（pinfo->fd）指针的位置
	safe_write(pinfo->fd, &pinfo->datablock[dayidx], sizeof(FDAY_DATA_BLOCK)); /// 从该位置读写 /// sizeof(FDAY_DATA_BLOCK)是写的长度
	if (flush_flag) {
		fday_flush();
	}
	sem_post(&pinfo->sem_f_day); /// 使用资源数减少
}

static void fday_update(int dayidx, int mtidx, BOOL flush_flag) /// save variable data to file
{
	FDAY_INFO *pinfo = &fday_info;
	off_t offset;

	if (pinfo->fd < 0) {
		return;
	}

	///PRINTF("pinfo->fd = %d(> 0)\n", pinfo->fd);

	sem_wait(&pinfo->sem_f_day); /// cannot visit this function, when here, because lock in &pinfo->sem_db(here is not necessary)
	///offset = dayidx * sizeof(FDAY_DATA_BLOCK) + offsetof(FDAY_DATA_BLOCK, daydata) + mtidx * sizeof(FDAY_DATA); /// offset
	offset = dayidx * sizeof(FDAY_DATA_BLOCK)
			+ offsetof(FDAY_DATA_BLOCK, daydata) + mtidx * sizeof(FDAY_DATA);
	lseek(pinfo->fd, offset, SEEK_SET); /// 返回文件指针
	///printf("fday_update safe_write\n");

	///safe_write(pinfo->fd, &pinfo->datablock[dayidx].daydata + mtidx, sizeof(FDAY_DATA)); /// wd error
	safe_write(pinfo->fd, &pinfo->datablock[dayidx].daydata[mtidx],
			sizeof(FDAY_DATA));

	if (flush_flag) {
		///printf("day data flush\n");
		fday_flush();
	}

	sem_post(&pinfo->sem_f_day);
}

void fday_close(void)
{
	FDAY_INFO *pinfo = &fday_info;

	fdatasync(pinfo->fd);
	close(pinfo->fd);
	pinfo->fd = -1;
	sem_destroy(&pinfo->sem_f_day);
	sem_destroy(&pinfo->sem_db);
}

int fday_create_dayblock(void) {
	FDAY_INFO *pinfo = &fday_info;
	FDAY_DATA_BLOCK *pblock = NULL;
	int i, dayidx = -1;
	BOOL b_success = FALSE;

	sem_wait(&pinfo->sem_db); /// sem == secure memory
	for (i = 0; i < MAX_GASMETER_DAY_CNT; i++) {
		pblock = &pinfo->datablock[i];
		if (!pblock->valid) {
			///printf("fday_create_dayblock block[%d] is not valid\n", i);
			b_success = TRUE;
			///sem_post(&pinfo->sem_db); /// comment by wd
			///return i; 
			dayidx = i;
			break;
		} else {
			continue;
		}
	}
	if (!b_success) { /// if block is not success
		b_success = TRUE;
		dayidx = 0;
		pblock = &pinfo->datablock[0];
		long min_tt = pblock->tt;
		for (i = 1; i < MAX_GASMETER_DAY_CNT; i++) {
			pblock = &pinfo->datablock[i];
			if (pblock->tt < min_tt) {
				dayidx = i; /// find a minimun tt in all block
			} else {

			}
		}
	} else {

	}
	if (b_success) {
		//printf("fday_create_dayblock run in from 4\n");
		pblock = &pinfo->datablock[dayidx];
		fday_block_init(pblock);
		pblock->valid = TRUE; /// set this block valid
		time(&pblock->tt); /// refresh the time in the block with now time
		fday_update_dayblock(dayidx, TRUE);
		///printf("set block valid = ture, block tt = now time, and write block into file\n");
	} else {
		//printf("fday_create_dayblock run out from 4\n");
	}
	//printf("fday_create_dayblock not run out entirely\n");
	sem_post(&pinfo->sem_db);
	return dayidx;
}

BOOL fday_set_data(int dayidx, int mtidx, WORD di, const void *di_data) {
	FDAY_DATA *p;
	FDAY_INFO *pinfo = &fday_info;
	BYTE address[7];
	BOOL b_success = FALSE;

	if (dayidx < 0 || dayidx >= MAX_GASMETER_DAY_CNT)
		return FALSE;
	if (mtidx < 0 || mtidx >= MAX_GASMETER_NUMBER)
		return FALSE;
	if (!fgasmeter_getgasmeter(mtidx, address, NULL))
		return FALSE;
	sem_wait(&pinfo->sem_db);
	p = &pinfo->datablock[dayidx].daydata[mtidx];

	p->valid = TRUE;
	memcpy(p->address, address, 7);
	switch (di) {
	case 0x901F:
		if (di_data) {
			memcpy(&p->u.data.data_901f, di_data, sizeof(GASMETER_CJT188_901F));
			p->u.data.data_901f.status = GASMETER_READ_STATUS_NORMAL;
		} else {
			printf("it should not be here\n");
		}
		fday_update(dayidx, mtidx, TRUE);
		b_success = TRUE;
		break;
	default:
		break;
	}
	sem_post(&pinfo->sem_db);
	return b_success;
}

BOOL fday_get_data(int dayidx, int mtidx, WORD di, void *di_data)
{
	FDAY_DATA *p;
	FDAY_INFO *pinfo = &fday_info;
	BYTE address[7];
	BOOL b_success = FALSE;

	if (dayidx < 0 || dayidx >= MAX_GASMETER_DAY_CNT)
		return FALSE;
	if (mtidx < 0 || mtidx >= MAX_GASMETER_NUMBER)
		return FALSE;
	if (!fgasmeter_getgasmeter(mtidx, address, NULL))
		return FALSE;
	sem_wait(&pinfo->sem_db);
	p = &pinfo->datablock[dayidx].daydata[mtidx];
	if (p->valid) {
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

int fday_get_datablock_index(void)
{
	int i, index = -1;
	struct tm cur_tm, day_tm;
	FDAY_INFO *pinfo = &fday_info;

	sys_time(&cur_tm);
	sem_wait(&pinfo->sem_db);
	for (i = 0; i < MAX_GASMETER_DAY_CNT; i++) {
		localtime_r(&pinfo->datablock[i].tt, &day_tm);
		if (pinfo->datablock[i].valid && cur_tm.tm_year == day_tm.tm_year
				&& cur_tm.tm_mon == day_tm.tm_mon
				&& cur_tm.tm_mday == day_tm.tm_mday) {
			index = i;
			break;
		}
	}
	sem_post(&pinfo->sem_db);
	return index;
}

int fday_get_datablock_index_by_time(WORD year, BYTE month, BYTE day)
{
	int i, index = -1;
	struct tm day_tm;
	FDAY_INFO *pinfo = &fday_info;

	sem_wait(&pinfo->sem_db);
	for (i = 0; i < MAX_GASMETER_DAY_CNT; i++) {
		localtime_r(&pinfo->datablock[i].tt, &day_tm);
		if (pinfo->datablock[i].valid && (year - 1900) == day_tm.tm_year
				&& month == (day_tm.tm_mon + 1) && day == day_tm.tm_mday) {
			index = i;
			break;
		}
	}
	sem_post(&pinfo->sem_db);
	return index;
}

int get_dayindex_by_date(struct tm tm, bool date_or_month) /// get index of year-month-day( SPECIFIED DETAILS )
{
	int i, index = -1;
	struct tm day_tm;
	FDAY_INFO *pinfo = &fday_info;

	sem_wait(&pinfo->sem_db);
	for (i = 0; i < MAX_GASMETER_DAY_CNT; i++) {
		localtime_r(&pinfo->datablock[i].tt, &day_tm);
		if (pinfo->datablock[i].valid && tm.tm_year == day_tm.tm_year
				&& tm.tm_mon == day_tm.tm_mon) {
			if(date_or_month){ // date
				if(tm.tm_mday == day_tm.tm_mday){
					index = i;
					break;
				}else
					continue;
			}else{ // month
				index = i;
				break;
			}
		}
	}
	sem_post(&pinfo->sem_db);
	return index;
}

/// added by wd
int fday_block_success_sum(int day_block_index) {
	unsigned short meter_idx;
	FDAY_INFO *pinfo = &fday_info;
	FDAY_DATA *fd;
	int ret;

	ret = 0;
	if (pinfo->datablock[day_block_index].valid) {
		for (meter_idx = 0; meter_idx < MAX_GASMETER_NUMBER; meter_idx++) {
			fd = &(pinfo->datablock[day_block_index].daydata[meter_idx]);
			if (fd->valid) {
				ret++;
			}
		}
		return ret;
	} else {
		return 0;
	}
}

void fday_block_init_out_use(FDAY_DATA_BLOCK *pdata) {
	fday_block_init(pdata);
}

void read_fday_blocks(FDAY_DATA_BLOCK *data_block) {
	if (data_block != NULL) {
		memcpy(data_block, fday_info.datablock,
				sizeof(FDAY_DATA_BLOCK) * MAX_GASMETER_DAY_CNT);
	}
	return;
}

void fday_update_test(int day_idx, int meter_idx) {
	fday_update(day_idx, meter_idx, TRUE);
}

int reset_fday_data(void) {
	FDAY_INFO *pinfo = &fday_info;
	int ret;

	sem_wait(&pinfo->sem_db);
	if (remove(F_DAY_NAME) == 0) {
		ret = 0;
	} else {
		ret = -1;
	}
	fday_open(); // open file
	sem_post(&pinfo->sem_db);

	return ret;
}

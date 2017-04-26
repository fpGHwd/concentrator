/*
 * f_day.h
 *
 *  Created on: 2015-8-11
 *      Author: Johnnyzhang
 */

#ifndef F_DAY_H_
#define F_DAY_H_

#include "typedef.h"
#include "protocol_cjt188_data.h"
#include "f_gasmeter.h"

///#define F_DAY_NAME "f_day.dat" /// 
#define F_DAY_NAME "/opt/concentrator/data/f_day.dat"

#define MAX_GASMETER_DAY_CNT 60 /// GASMETER_DAY

typedef struct {
	GASMETER_CJT188_901F data_901f; /// 在这里添加其他协议数据
} GASMETER_CJT188_DAY;

typedef struct {
	BOOL valid;
	BYTE address[7];
	union {
		BYTE reserve_bytes[64];
		GASMETER_CJT188_DAY data; // size of data is less than size of reserve_bytes
	} u;
} FDAY_DATA;

typedef struct {
	BOOL valid;
	long tt; /// 时间， 年月日有效
	FDAY_DATA daydata[MAX_GASMETER_NUMBER];
} FDAY_DATA_BLOCK;

void fday_open(void);
void fday_close(void);
int fday_create_dayblock(void);
BOOL fday_set_data(int dayidx, int mtidx, WORD di, const void *di_data);
BOOL fday_get_data(int dayidx, int mtidx, WORD di, void *di_data);
int fday_get_datablock_index(void);
int fday_get_datablock_index_by_time(WORD year, BYTE month, BYTE day);

/// add by wd 
int fday_block_success_sum(int day_block_index);
void fday_block_init_out_use(FDAY_DATA_BLOCK *pdata);
void read_fday_blocks(FDAY_DATA_BLOCK *data_block);
void fday_update_test(int day_idx, int meter_idx);

// add by wd 
int reset_fday_data(void);

#endif /* F_DAY_H_ */

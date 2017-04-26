/*
 * f_month.h
 *
 *  Created on: 2015-8-11
 *      Author: Johnnyzhang
 */

#ifndef F_MONTH_H_
#define F_MONTH_H_

#include "typedef.h"
#include "protocol_cjt188_data.h"
#include "f_gasmeter.h"

//#define F_MON_NAME "f_month.dat"
#define F_MON_NAME "/opt/concentrator/data/f_month.dat"

#define MAX_GASMETER_MON_CNT 120

typedef struct {
	GASMETER_CJT188_901F data_901f;
} GASMETER_CJT188_MON;

typedef struct {
	BOOL valid;
	BYTE address[7];
	union {
		BYTE reserve_bytes[64];
		GASMETER_CJT188_MON data; // size of data is less than size of reserve_bytes 
	} u;
} FMON_DATA;

typedef struct {
	BOOL valid; /// 是否在使用
	long tt; /// time
	FMON_DATA mondata[MAX_GASMETER_NUMBER]; /// 
} FMON_DATA_BLOCK;

void fmon_open(void);
void fmon_close(void);
int fmon_create_monblock(void);
BOOL fmon_set_data(int monidx, int mtidx, WORD di, const void *di_data);
BOOL fmon_get_data(int monidx, int mtidx, WORD di, void *di_data);
int fmon_get_datablock_index(void);
int fmon_get_datablock_index_by_time(WORD year, BYTE month);

// add by wd 
int reset_fmonth_data(void);
int fmon_block_success_sum(int mon_block_index);

#endif /* F_MONTH_H_ */

/*
 * f_current.h
 *
 *  Created on: 2015-8-11
 *      Author: Johnnyzhang
 */

#ifndef F_CURRENT_H_
#define F_CURRENT_H_

#include "typedef.h"
#include "protocol_cjt188_data.h"
#include "f_gasmeter.h"

//#define F_CURRENT_NAME "f_current.dat"
#define F_CURRENT_NAME "../data/f_current.dat"

typedef struct {
	GASMETER_CJT188_901F data_901f;
} GASMETER_CJT188_CURRENT;

typedef struct {
	BOOL valid;
	BYTE address[7];
	union {
		BYTE reserve_bytes[64];
		GASMETER_CJT188_CURRENT data; // size of data is less than size of reserve_bytes
	} u;
} FCURRENT_DATA;

void fcurrent_open(void);
void fcurrent_close(void);
BOOL fcurrent_set_data(int mtidx, WORD di, const void *di_data);
BOOL fcurrent_get_data(int mtidx, WORD di, void *di_data);
BOOL fcurrent_set_status(int mtidx, WORD di, GASMETER_READ_STATUS status);

#endif /* F_CURRENT_H_ */

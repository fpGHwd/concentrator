/*
 * protocol_cjt188_data.h
 *
 *  Created on: 2015-8-30
 *      Author: Johnnyzhang
 */

#ifndef PROTOCOL_CJT188_DATA_H_
#define PROTOCOL_CJT188_DATA_H_

#include "typedef.h"
#include "common.h"

typedef struct {
	WORD di; /// x901F
	BYTE ser; /// 报文号? ///
	BYTE flux[5];  /// 当前累计流量 = 底码
	BYTE balance_flux[5]; /// 结算日累计流量
	BYTE clock[7]; /// 实时时间 /// 表具的时间
	BYTE st[2]; /// 表状态
} GASMETER_CJT188_DATA_901F;

typedef enum {
	GASMETER_READ_STATUS_UNREAD,
	GASMETER_READ_STATUS_NORMAL,
	GASMETER_READ_STATUS_ABORT,
} GASMETER_READ_STATUS; /// READ STATUS

typedef struct {
	GASMETER_READ_STATUS status;
	GASMETER_CJT188_DATA_901F di_data;
	long read_tt; /// 
} GASMETER_CJT188_901F; /// 901F协议

/// 其他协议的报文结构

#define MAX_HEXDATA_BUF 64
#define PRINT_ADDRESS(x, a, b) hex_to_str(x, sizeof(x), a, b, FALSE)

void ptl_cjt188_data_print(WORD di, void *p);
void ptl_cjt188_data_init(WORD di, void *p);
BOOL ptl_cjt188_data_format(void *p, WORD di, long tt, const BYTE *buf,
		int buflen);

#endif /* PROTOCOL_CJT188_DATA_H_ */

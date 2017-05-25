/*
 * protocol_cjt188.h
 *
 *  Created on: 2015-8-11
 *      Author: Johnnyzhang
 */

#ifndef PROTOCOL_CJT188_H_
#define PROTOCOL_CJT188_H_

#include "typedef.h"

typedef enum {
	CJT188_METER_TYPE_COLD_WATER = 0x10,
	CJT188_METER_TYPE_HOT_WATER = 0x11,
	CJT188_METER_TYPE_DIRECT_WATER = 0x12,
	CJT188_METER_TYPE_MEDIUM_WATER = 0x13,
	CJT188_METER_TYPE_HOT_HEAT = 0x20,
	CJT188_METER_TYPE_COLD_HEAT = 0x21,
	CJT188_METER_TYPE_GAS = 0x30, /// 0x68 0x30 - gasmeter
	CJT188_METER_TYPE_ELECTRICITY = 0x40,
} CJT188_METER_TYPE;  /// 仪表类型

typedef enum {
	CJT188_CTR_RESERVE = 0x00,
	CJT188_CTR_READ_DATA = 0x01,
	CJT188_CTR_WRITE_DATA = 0x04,
	CJT188_CTR_READ_VER = 0x05,
	CJT188_CTR_READ_ADDRESS = 0x03,
	CJT188_CTR_WRITE_ADDRESS = 0x15,
	CJT188_CTR_WRITE_SYNC_PARA = 0x16,
} CJT188_CTR; // 控制码

typedef struct {
	UINT8 type;
	UINT8 address[7];
	UINT8 ctrl;
	UINT8 datalen;
	UINT8 *data;
	UINT8 cs;
} PTL_CJT188_MSG;

int plt_cjt188_pack(UINT8 *buf, UINT32 max_len, const PTL_CJT188_MSG *msg);
BOOL plt_cjt188_unpack(PTL_CJT188_MSG *msg, const UINT8 *buf, UINT32 buflen);
BOOL plt_cjt188_check_packet(PTL_CJT188_MSG *msg, const UINT8 *buf,
		UINT32 buflen, const UINT8 *address, UINT8 ctrl, WORD di, UINT8 SER);
int plt_cjt188_pack_read(UINT8 *buf, UINT32 max_len, const UINT8 *address,
		UINT8 ctr_0, UINT16 di);
int plt_cjt188_pack_write(UINT8 *buf, UINT32 max_len, const UINT8 *address,
		UINT8 ctr_3, UINT16 di, UINT8 *data, UINT8 datalen);
int plt_cjt188_read_packet(UINT8 *buf, UINT32 max_len, int timeout,
		int (*read_fn)(void *, int, int));
void plt_cjt188_inc_ser(void);
UINT8 plt_cjt188_get_ser(void);

#endif /* PROTOCOL_CJT188_H_ */

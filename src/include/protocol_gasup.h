/*
 * protocol_gasup.h
 *
 *  Created on: 2015-8-11
 *      Author: Johnnyzhang
 */

#ifndef PROTOCOL_GASUP_H_
#define PROTOCOL_GASUP_H_

#include "typedef.h"
#include "f_con_alarm.h"
#include "f_gasmeter_alarm.h"

#define PTL_GASUP_MAX_PACK_CNT        50

enum { /// 主站 - 集中器协议
	PTL_GASUP_FN_COMM_HOSTTEST = 2001,
	PTL_GASUP_FN_COMM_CONTEST = 2002,
	PTL_GASUP_FN_COMM_HEARTBEAT = 2003,
	PTL_GASUP_FN_COMM_REGISTER = 2004, /// regitster and login (to) mainstation
	PTL_GASUP_FN_SET_HEARTBEAT = 2011,
	PTL_GASUP_FN_GET_HEARTBEAT = 2012,
	PTL_GASUP_FN_SET_RM_TIME = 2013, // Set time of read meter
	PTL_GASUP_FN_GET_RM_TIME = 2014,
	PTL_GASUP_FN_SET_CON_CLK = 2015, // Set clock of concentrator
	PTL_GASUP_FN_GET_CON_CLK = 2016,
	PTL_GASUP_FN_SET_WAKEUP_CYCLE = 2021, // Set Wake up cycle of meter
	PTL_GASUP_FN_GET_WAKEUP_CYCLE = 2022,
	PTL_GASUP_FN_SET_METER_CLK = 2023, // Set clock of meter
	PTL_GASUP_FN_GET_METER_CLK = 2024,
	PTL_GASUP_FN_ADD_COLLECTOR = 2031,
	PTL_GASUP_FN_DEL_COLLECTOR = 2032,
	PTL_GASUP_FN_ADD_METER = 2033,
	PTL_GASUP_FN_DEL_METER = 2034,
	PTL_GASUP_FN_GET_COLLECTOR = 2035,
	PTL_GASUP_FN_GET_METER = 2036,
	PTL_GASUP_FN_GET_DATA = 2041, // Get data for some meters
	PTL_GASUP_FN_GET_ONE_RD_DATA = 2042, // Get real time data for one meter //// 单表实时抄表
	PTL_GASUP_FN_GET_MONTHDATA = 2043,
	PTL_GASUP_FN_GET_DAYDATA = 2044,
	PTL_GASUP_FN_CTL_VALVEOFF = 2051,
	PTL_GASUP_FN_CTL_VALVEON = 2052,
	PTL_GASUP_FN_GET_VALVE = 2053,
	PTL_GASUP_FN_ALM_CON = 2061,
	PTL_GASUP_FN_ALM_METER = 2062,
};

enum {
	PTL_GASUP_DIR_MASTER = 0, PTL_GASUP_DIR_SLAVE = 1,
};

enum {
	PTL_GASUP_NEED_REQUEST = 0, PTL_GASUP_NEED_RESPOND = 1,
};

typedef enum {
	PACKET_TYPE_LOGIN = 1, PACKET_TYPE_HEARTBEAT,
} PACKET_TYPE;

typedef struct {
	UINT16 packetlen;
	UINT16 fn; // [BCD]
	UINT8 direction; // [HEX]
	UINT8 flag; // [HEX] 0: Request, 1: Respond
	UINT8 address[7]; // Address of concentrator, [BCD]
	UINT8 packetID[7]; // [BCD]
	UINT16 datalen; // [HEX]
	UINT8 *data;  /// data 
	UINT16 crc16; // [HEX]
} PTL_GASUP_MSG; /// 上行报文结构

/*
enum {
	PTL_GASUP_CODE_SUCCESS = 0,
};
*/

int plt_gasup_pack_special(UINT16 fn, UINT8 *buf, UINT32 max_len, UINT8 *data,
		int len, UINT32 packetID);
int ptl_gasup_pack_gasmeteralarm_data(BYTE *buf, int maxlen, WORD *pfn,
		GASMETER_ALARM_T *palarm);
int ptl_gasup_pack_conalarm_data(BYTE *buf, int maxlen, WORD *pfn,
		CON_ALARM_T *palarm);
BOOL plt_gasup_check_pack_special(const UINT8 *address, UINT16 fn, UINT8 *buf,
		UINT32 len, INT32 packetID);
int plt_gasup_pack(UINT8 *buf, UINT32 max_len, const PTL_GASUP_MSG *msg);
BOOL plt_gasup_unpack(PTL_GASUP_MSG *msg, const UINT8 *buf, UINT32 buflen);
INT32 plt_gasup_parse(const UINT8 *buf, INT32 buf_len);
int plt_gasup_read_socket(int fd, void *buf, int maxlen, int timeout);
void plt_gasup_proc(INT32 in_idx, INT32 out_idx, INT32 max_out_len, void *priv);

#endif /* PROTOCOL_GASUP_H_ */

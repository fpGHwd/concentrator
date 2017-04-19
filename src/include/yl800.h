/*
 * yl800.h
 *
 *  Created on: 2015-8-11
 *      Author: Johnnyzhang
 */

#ifndef YL800_H_
#define YL800_H_

#include "typedef.h"
//----------------------------------------------------------------------------------------------------

#define YL800_ID_SIZ 4U

#define YL800_RLY_SIZ 3U

#define YL800_SPC_SIZ 1U

#define YL800_ID_RLY_SIZ ( YL800_ID_SIZ + YL800_RLY_SIZ )  

#define YL800_PKT_MIN ( 1 + YL800_ID_SIZ ) /// package 4+1

typedef struct {
	UINT8 *XX;
	UINT8 *YY;
} YL800_MSG_DATA;

typedef struct {
	UINT8 id[2];
	UINT8 command_XX;
	UINT8 command_YY;
	UINT8 datalen;
	union {
		UINT8 *data;
		YL800_MSG_DATA *data_xxyy;
	} u;
	UINT8 cs;
} YL800_MSG;

typedef enum {
	YL800_CFG_SPEED_1200 = 1,
	YL800_CFG_SPEED_2400 = 2,
	YL800_CFG_SPEED_4800 = 3,
	YL800_CFG_SPEED_9600 = 4,
	YL800_CFG_SPEED_19200 = 5,
	YL800_CFG_SPEED_38400 = 6,
	YL800_CFG_SPEED_57600 = 7,
} YL800_CFG_SPEED;

typedef enum {
	YL800_CFG_PARITY_NONE = 0,
	YL800_CFG_PARITY_ODD = 1,
	YL800_CFG_PARITY_EVEN = 2,
} YL800_CFG_PARITY;

typedef enum {
	YL800_CFG_FACTOR_128 = 7,
	YL800_CFG_FACTOR_256 = 8,
	YL800_CFG_FACTOR_512 = 9,
	YL800_CFG_FACTOR_1024 = 10,
	YL800_CFG_FACTOR_2048 = 11,
	YL800_CFG_FACTOR_4096 = 12,
} YL800_CFG_FACTOR;

typedef enum {
	YL800_CFG_MODE_NORMAL = 0,
	YL800_CFG_MODE_LOW_POWER = 1,
	YL800_CFG_MODE_SLEEP = 2,
} YL800_CFG_MODE;

typedef enum {
	YL800_CFG_BANDWIDTH_61_5K = 6,
	YL800_CFG_BANDWIDTH_125K = 7,
	YL800_CFG_BANDWIDTH_256K = 8,
	YL800_CFG_BANDWIDTH_512K = 9,
} YL800_CFG_BANDWIDTH;

typedef enum {
	YL800_CFG_HB_2S = 0,
	YL800_CFG_HB_4S = 1,
	YL800_CFG_HB_6S = 2,
	YL800_CFG_HB_8S = 3,
	YL800_CFG_HB_10S = 4,
} YL800_CFG_HB;

typedef struct {
	YL800_CFG_SPEED speed;
	YL800_CFG_PARITY parity;
	UINT32 freq;
	YL800_CFG_FACTOR factor;
	YL800_CFG_MODE mode;
	YL800_CFG_BANDWIDTH bandwidth;
	UINT8 ID_H;
	UINT8 ID_L;
	UINT8 net_ID;
	UINT8 power;
	YL800_CFG_HB heartbead;
} YL800_CONFIG; /// module parameters

int yl800_atcmd_pack(UINT8 *buf, UINT32 max_len, const YL800_MSG *msg);
BOOL yl800_atcmd_unpack(YL800_MSG *msg, const UINT8 *buf, UINT32 buflen);
int yl800_pack(UINT8 *buf, UINT32 max_len, UINT8 *repeater, UINT8 *address,
		UINT8 *data, int datalen);
BOOL yl800_unpack(const UINT8 *buf, UINT32 buflen, UINT8 *repeater,
		UINT8 *address, UINT8 **data, int *datalen);
int yl800_read_packet(UINT8 *buf, UINT32 max_len, int timeout,
		int (*read_fn)(void *, int, int));

#endif /* YL800_H_ */

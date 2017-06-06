/*
 * msg_que.h
 *
 *  Created on: 2015-8-11
 *      Author: Johnnyzhang
 */

#ifndef MSG_QUE_H_
#define MSG_QUE_H_

#include "typedef.h"

#define MSG_QUE_START			0 // not used
#define MSG_QUE_GPRSCDMA_IN		1 // GAS UP communication for GPRS/CDMA
#define MSG_QUE_GPRSCDMA_OUT	2 // GAS UP communication for GPRS/CDMA
#define MSG_QUE_ETH_IN			3 // GAS UP communication for Ethernet
#define MSG_QUE_ETH_OUT			4 // GAS UP communication for Ethernet
#define MSG_QUE_METER_COMM_IN	5 // structure
#define MSG_QUE_METER_COMM_OUT	6 // structure
#define MSG_QUE_MENU_IN			7 // structure
#define MSG_QUE_MENU_OUT		8 // structure

#define MSG_QUE_MAX				9

#define MSG_QUE_NO_STAMP		0

void msg_que_init(void);

void msg_que_destroy(void);

int msg_que_is_empty(int idx, int stamp);

int msg_que_get(int idx, void *buf, int max_len, int *len, int stamp);

int msg_que_put(int idx, const void *buf, int len, int stamp);

#endif /* MSG_QUE_H_ */

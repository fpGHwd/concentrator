/*
 * yl800.c
 *
 *  Created on: 2015-8-11
 *      Author: Johnnyzhang
 */

#include "yl800.h"
#include "common.h"
#include "protocol_cjt188.h"
#include "main.h"

static YL800_CONFIG yl800_config = {
		.speed = YL800_CFG_SPEED_1200,
		.parity = YL800_CFG_PARITY_NONE,
		.freq = 0x6C4012, // 433M
		.factor = YL800_CFG_FACTOR_128,
		.mode = YL800_CFG_MODE_NORMAL,
		.bandwidth = YL800_CFG_BANDWIDTH_61_5K,
		.ID_H = 0,
		.ID_L = 0,
		.net_ID = 0,
		.power = 0,
		.heartbead = YL800_CFG_HB_2S,
};

// TODO: initiate the yl800 module
BOOL yl800_init(void)
{
	yl800_config.net_ID = rf_id;
	return FALSE;
}

int yl800_atcmd_pack(UINT8 *buf, UINT32 max_len, const YL800_MSG *msg)
{
	UINT8 *ptr = buf;

	if (buf == NULL || msg == NULL)
		return 0;
	if (max_len < 11)
		return 0;
	*ptr++ = 0xAF;
	*ptr++ = 0xAF;
	memcpy(ptr, msg->id, 2);
	ptr += 2;
	*ptr++ = 0xAF;
	*ptr++ = msg->command_XX;
	*ptr++ = msg->command_YY;
	*ptr++ = msg->datalen;
	if (max_len < 11 + msg->datalen)
		return 0;
	memcpy(ptr, msg->u.data, msg->datalen);
	ptr += msg->datalen;
	*ptr = check_sum(buf, ptr - buf);
	ptr++;
	*ptr++ = 0x0D;
	*ptr++ = 0x0A;
	return ptr - buf;
}

BOOL yl800_atcmd_unpack(YL800_MSG *msg, const UINT8 *buf, UINT32 buflen)
{
	const UINT8 *ptr = buf, *start;
	UINT8 cs;

	if (msg == NULL || buf == NULL)
		return FALSE;
	while (*ptr != 0xAF && buflen > 1) {
		ptr++;
		buflen--;
	}
	start = ptr;
	if (buflen < 11)
		return FALSE;
	ptr++;
	if (*ptr != 0xAF)
		return FALSE;
	ptr++;
	memcpy(msg->id, ptr, 2);
	ptr += 2;
	if (*ptr != 0xAF)
		return FALSE;
	ptr++;
	msg->command_XX = *ptr++;
	msg->command_YY = *ptr++;
	msg->datalen = *ptr++;
	if (buflen < 11 + msg->datalen)
		return FALSE;
	msg->u.data = (UINT8 *)ptr;
	ptr += msg->datalen;
	cs = check_sum(start, ptr - start);
	msg->cs = *ptr++;
	if (*ptr != 0x0D || *(ptr + 1) != 0x0A)
		return FALSE;
	ptr += 2;
	if (cs == msg->cs)
		return TRUE;
	else {
		PRINTF("%s CS ERROR, CS = %04X, CS in packet: %04X\n", __FUNCTION__, cs, msg->cs);
		return FALSE;
	}
}

//#define ADDRESS_BYTE_LENGTH 7
int yl800_pack(UINT8 *buf, UINT32 max_len, UINT8 *repeater, UINT8 *address,
		UINT8 *data, int datalen)
{
	UINT8 *ptr = buf;

	if (buf == NULL || address == NULL || max_len < 3 || data == NULL || datalen <= 0)
		return 0;
	if (repeater) {
		*ptr++ = 0x55;
		if (max_len < 6 + datalen)
			return 0;
		memcpy(ptr, repeater, 3);
		ptr += 3;
	}
	else {
		*ptr++ = 0xAA;
		if (max_len < 3 + datalen)
			return 0;
	}
	memcpy(ptr, address, 4);
	ptr += 4;
	memcpy(ptr, data, datalen);
	ptr += datalen;
	return ptr - buf;
}

BOOL yl800_unpack(const UINT8 *buf, UINT32 buflen, UINT8 *repeater,
		UINT8 *address, UINT8 **data, int *datalen)
{
	const UINT8 *ptr = buf;
	UINT8 ctrl;

	if (buf == NULL || data == NULL || datalen == NULL)
		return FALSE;
	if (buflen < 3)
		return FALSE;
	ctrl = *ptr++;
	switch (ctrl) {
	case 0x55:
		if (buflen <= 6)
			return FALSE;
		if (repeater) {
			memcpy(repeater, ptr, 3);
			ptr += 3;
		}
		break;
	case 0xAA:
		if (buflen <= 3)
			return FALSE;
		break;
	default:
		return FALSE;
	}
	if (address) {
		memcpy(address, ptr, 2);
		ptr += 2;
	}
	*data = (UINT8 *)ptr;
	*datalen = buflen - (ptr - buf);
	PRINTF("YL800 unpacket OK\n");
	return TRUE;
}

int yl800_read_packet(UINT8 *buf, UINT32 max_len, int timeout, int (*read_fn)(void *, int, int))
{
	UINT8 *ptr = buf;
	UINT8 ctrl;
	int len;

	if (buf == NULL || max_len < 6 || read_fn == NULL)
		return 0;
	if (read_fn(ptr, 1, timeout) != 1)
		return 0;
	ctrl = *ptr++;
	switch (ctrl) {
	case 0x55:
		PRINTF("%s This is YL800 packet with repeater\n", __FUNCTION__);
		if (read_fn(ptr, 5, timeout) != 5)
			return 0;
		ptr += 5;
		break;
	case 0xAA:
		PRINTF("%s This is YL800 packet without repeater\n", __FUNCTION__);
		if (read_fn(ptr, 2, timeout) != 2)
			return 0;
		ptr += 2;
		break;
	default:
		PRINTF("%s This is unknown YL800 packet with repeater\n", __FUNCTION__);
		return 0;
	}
	len = plt_cjt188_read_packet(ptr, max_len - (ptr - buf), timeout, read_fn);
	if (len <= 0)
		return 0;
	ptr += len;
	return ptr - buf;
}

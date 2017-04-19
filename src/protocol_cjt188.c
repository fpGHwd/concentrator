/*
 * protocol_cjt188.c
 *
 *  Created on: 2015-8-11
 *      Author: Johnnyzhang
 */

#include "protocol_cjt188.h"
#include "common.h"

static UINT8 cjt188_ser; /// 

int plt_cjt188_pack(UINT8 *buf, UINT32 max_len, const PTL_CJT188_MSG *msg) /// CJT188 /// 不包含网络地址
{
	UINT8 *ptr = buf, *pstart;
	int i;

	if (buf == NULL || msg == NULL || (msg->datalen > 0 && msg->data == NULL))
		return 0;
	if (max_len < 17 || max_len < 17 + msg->datalen)
		return 0;
	/* // 
	 if(max_len < 17 + msg->datalen)
	 return 0; // */ /// added by wd 
	memset(ptr, 0xFE, 4); /// 4个字节0xFE
	ptr += 4;
	pstart = ptr;
	*ptr++ = 0x68;  //// 0x68
	*ptr++ = msg->type; /// 仪表类型, 0x30
	for (i = 0; i < 7; i++) {
		*ptr++ = msg->address[7 - i - 1]; /// 表号, 23051606000102
	}
	*ptr++ = msg->ctrl; /// 控制码,C,
	*ptr++ = msg->datalen; /// 数据长度L /// 0x03 一个字节
	memcpy(ptr, msg->data, msg->datalen); /// 数据域DATA
	ptr += msg->datalen;
	*ptr = check_sum(pstart, ptr - pstart); /// CS ， 一个字节
	ptr++;
	*ptr++ = 0x16;
	return ptr - buf;
}

/// cjt188 protocol = CJ/T 188-2004 page4
BOOL plt_cjt188_unpack(PTL_CJT188_MSG *msg, const UINT8 *buf, UINT32 buflen) /// unpack the buf whose length is buflen /// buflen and buf make sure the space we need to copy the buf content
{
	/// buf(buflen) -> ptl_cjt188_msg *msg; move buf to ptl_cjt188_msg
	const UINT8 *ptr = buf, *pstart;
	UINT8 cs;
	int i;

	if (msg == NULL || buf == NULL)
		return FALSE;
	while (*ptr != 0x68 && buflen > 1) { /// 0x68
		ptr++;
		buflen--;
	}
	pstart = ptr;
	if (buflen < 13)
		return FALSE;
	ptr++;
	msg->type = *ptr++; /// type of meter
	for (i = 0; i < 7; i++) {
		msg->address[7 - i - 1] = *ptr++;  /// address of meter
	}
	msg->ctrl = *ptr++;  /// control code of meter
	msg->datalen = *ptr++;  /// length of data domain
	if (buflen < 13 + msg->datalen)
		return 0;
	msg->data = (UINT8 *) ptr; /// data just use a pointer?
	ptr += msg->datalen;
	cs = check_sum(pstart, ptr - pstart); /// get crc16, one byte
	msg->cs = *ptr++; /// msg->cs = *ptr; ptr += 1; /// 1 byte
	if (*ptr != 0x16)
		return FALSE;
	///ptr++; /// no sense
	if (cs == msg->cs)
		return TRUE;
	else {
		PRINTF("%s CS ERROR, CS = %04X, CS in packet: %04X\n", __FUNCTION__, cs,
				msg->cs); /// cs error, discard this packet
		return FALSE;
	}
}

BOOL plt_cjt188_check_packet(PTL_CJT188_MSG *msg, const UINT8 *buf,
		UINT32 buflen, const UINT8 *address, UINT8 ctrl, WORD di, UINT8 SER) {
	WORD di_in_msg;

	if (!plt_cjt188_unpack(msg, buf, buflen)) /// 
		return FALSE;
	if (memcmp(msg->address, address, 7))
		return -1;
	if ((msg->ctrl & 0x1f) != ctrl) { // 00011111
		PRINTF("%s CTR is error, CTR: %02X, correct CTR: %02X\n", __FUNCTION__,
				msg->ctrl, CJT188_CTR_READ_DATA);
		return FALSE;
	}
	if ((msg->ctrl & 0x80) == 0) { /// 10000000 /// control code
		PRINTF("%s DIR is error, CTR: %02X\n", __FUNCTION__, msg->ctrl);
		return FALSE;
	}
	if (msg->ctrl & 0x40) {
		PRINTF("%s is NAK, CTR: %02X\n", __FUNCTION__, msg->ctrl);
		return FALSE;
	}
	if (msg->datalen < 3 || !msg->data) {
		PRINTF("%s data length is too small, data length: %d\n", __FUNCTION__,
				msg->datalen);
		return FALSE;
	}
	di_in_msg = ctos(msg->data);
	if (di_in_msg != di) {
		PRINTF("%s DI is error, DI: %04X, correct DI: %04X\n", __FUNCTION__,
				di_in_msg, di);
		return FALSE;
	}
	if (msg->data[2] != SER) {
		PRINTF("%s SER is error, SER: 0x%02X, correct SER: 0x%02X\n",
				__FUNCTION__, msg->data[2], SER);
		return FALSE;
	}
	PRINTF("CJ/T188 unpacket OK\n"); /// unpacket
	return TRUE;
}

int plt_cjt188_pack_read(UINT8 *buf, UINT32 max_len, const UINT8 *address,
/// command type(code) 9015, 必要的message , pack message with 188 protocol, 
/// pack message with yl800, send message, waiting for response.
		UINT8 ctr_0, UINT16 di) {
	UINT8 databuf[3];
	PTL_CJT188_MSG msg;

	if (buf == NULL || address == NULL)
		return 0;
	stoc(databuf, di); /// 0x901F // 将2bytes字节设置到databuf[0]
	databuf[2] = cjt188_ser; /// service
	msg.type = CJT188_METER_TYPE_GAS; /// T
	memcpy(msg.address, address, 7); //// address domain
	msg.ctrl = ctr_0; /// ctr_0
	msg.datalen = 0x03;  /// data length
	msg.data = databuf; /// massage
	return plt_cjt188_pack(buf, max_len, &msg);
}

int plt_cjt188_pack_write(UINT8 *buf, UINT32 max_len, const UINT8 *address,
		UINT8 ctr_3, UINT16 di, UINT8 *data, UINT8 datalen) {
	UINT8 databuf[255];
	PTL_CJT188_MSG msg;

	if (buf == NULL || address == NULL || data == NULL || datalen == 0)
		return 0;
	stoc(databuf, di);
	databuf[2] = cjt188_ser;
	memcpy(&databuf[3], data, datalen);
	msg.type = CJT188_METER_TYPE_GAS;
	memcpy(msg.address, address, 7);
	msg.ctrl = ctr_3;
	msg.datalen = 0x03 + datalen;
	msg.data = databuf;
	return plt_cjt188_pack(buf, max_len, &msg);
}

int plt_cjt188_read_packet(UINT8 *buf, UINT32 max_len, int timeout,
		int (*read_fn)(void *, int, int)) {
	UINT8 *ptr = buf;
	UINT8 value = 0;
	UINT8 datalen;

	///printf("cjt188 read funtion\n"); /// printf == print function
	if (buf == NULL || max_len < 13 || read_fn == NULL)
		return 0;
	while (value != 0x68) {
		if (read_fn(&value, 1, timeout) != 1)
			return 0;
		*ptr++ = value;
	}
	if (read_fn(ptr, 10, timeout) != 10)
		return 0;
	datalen = ptr[9];
	ptr += 10;
	if (read_fn(ptr, datalen + 2, timeout) != datalen + 2)
		return 0;
	ptr += (datalen + 2);
	return ptr - buf;
}

void plt_cjt188_inc_ser(void) {
	cjt188_ser++;
}

UINT8 plt_cjt188_get_ser(void) {
	return cjt188_ser;
}

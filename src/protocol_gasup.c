/*
 * protocol_gasup.c
 *
 *  Created on: 2015-8-11
 *      Author: Johnnyzhang
 */

/// 上行协议
#include "protocol_gasup.h"
#include "protocol_gasup_fn.h"
#include "common.h"
#include "fcs.h"
#include "protocol.h"
#include "msg_que.h"
#include "f_param.h"
#include "up_comm.h"

#define MAX_PACK_CNT PTL_GASUP_MAX_PACK_CNT
#define MSG_BUF_LEN         (MAX_PACK_CNT * CONFIG_MAX_APDU_LEN)

#define UP_CHANNEL_CNT 2

static UINT8 msg_buf[UP_CHANNEL_CNT * MSG_BUF_LEN];

static struct msg { /// 上行的报文结构
	INT32 que_in, que_out;
	UINT8 *buf; /// UINT8* buf
	INT32 saved_fcb; /// never used
	PTL_GASUP_MSG msg_in;
	INT32 msg_out_cnt;
	PTL_GASUP_MSG msg_out[MAX_PACK_CNT]; /// 
} msg_arr[UP_CHANNEL_CNT] = { /// GPRS/CDMA + ETH
				{ MSG_QUE_GPRSCDMA_IN, MSG_QUE_GPRSCDMA_OUT, msg_buf
						+ MSG_BUF_LEN * 0 }, { MSG_QUE_ETH_IN, MSG_QUE_ETH_OUT,
						msg_buf + MSG_BUF_LEN * 0 }, };

int plt_gasup_pack(UINT8 *buf, UINT32 max_len, const PTL_GASUP_MSG *msg) //// PTL_GASUP_MSG: protocol gas up message
{
	int len;
	UINT8 *ptr = buf;
	UINT8 *ppacklen;
	UINT16 crc16;

	if (buf == NULL || msg == NULL || (msg->datalen > 0 && msg->data == NULL))
		return 0;
	if (max_len < 26 || max_len < 26 + msg->datalen) // buffer is too small
		return 0;
	*ptr++ = 0x68;
	ppacklen = ptr;
	ptr += 2;
	bcd_be_stoc(ptr, msg->fn); /// 从这里入手， 功能码 fn = 2004, fn -> bcd
	ptr += 2;
	*ptr++ = msg->direction;
	*ptr++ = msg->flag;
	memcpy(ptr, msg->address, 7);
	ptr += 7;
	memcpy(ptr, msg->packetID, 7);
	ptr += 7;
	stoc(ptr, msg->datalen);
	ptr += 2;
	if (msg->datalen > 0) {
		memcpy(ptr, msg->data, msg->datalen);
		ptr += msg->datalen;
	}
	len = ptr - buf;
	stoc(ppacklen, len + 3);
	crc16 = fcs16(INITFCS16, buf, len);
	stoc(ptr, crc16);
	ptr += 2;
	*ptr++ = 0x16;
	return ptr - buf;
}

BOOL plt_gasup_unpack(PTL_GASUP_MSG *msg, const UINT8 *buf, UINT32 buflen) {
	const UINT8 *ptr = buf;
	UINT16 crc16;

	if (msg == NULL || buf == NULL || buflen < 26)
		return FALSE;
	if (*ptr != 0x68)
		return FALSE;
	ptr++;
	msg->packetlen = ctos(ptr);
	if (buflen < msg->packetlen)
		return FALSE;
	ptr += 2;
	bcd_be_ctos(ptr, &msg->fn);
	ptr += 2;
	msg->direction = *ptr++;
	msg->flag = *ptr++;
	memcpy(msg->address, ptr, 7);
	ptr += 7;
	memcpy(msg->packetID, ptr, 7);
	ptr += 7;
	msg->datalen = ctos(ptr);
	if (buflen < 26 + msg->datalen)
		return FALSE;
	ptr += 2;
	msg->data = (UINT8 *) ptr;
	ptr += msg->datalen;
	crc16 = fcs16(INITFCS16, buf, ptr - buf);
	msg->crc16 = ctos(ptr);
	ptr += 2;
	if (*ptr != 0x16)
		return FALSE;
	ptr++;
	if (crc16 == msg->crc16)
		return TRUE;
	else {
		PRINTF("%s CRC ERROR, CRC = %04X, CRC in packet: %04X\n", __FUNCTION__,
				crc16, msg->crc16);
		return FALSE;
	}
}

int plt_gasup_pack_special(UINT16 fn, UINT8 *buf, UINT32 max_len, UINT8 *data,
		int len, UINT32 packetID) {
	PTL_GASUP_MSG msg;
	struct tm tm;
	WORD year;
	UINT8 month, day;

	if (buf == NULL || max_len <= 0)
		return FALSE;
	msg.fn = fn;  /// (int)2004
	msg.direction = PTL_GASUP_DIR_SLAVE;
	msg.flag = PTL_GASUP_NEED_REQUEST;
	fparam_get_value(FPARAMID_CON_ADDRESS, msg.address, sizeof(msg.address));
	sys_time(&tm);
	year = tm.tm_year + 1900;
	msg.packetID[0] = bin_to_bcd(year % 100);
	month = tm.tm_mon + 1;
	msg.packetID[1] = bin_to_bcd(month);
	day = tm.tm_mday;
	msg.packetID[2] = bin_to_bcd(day);
	bcd_be_ltoc(&msg.packetID[3], packetID);
	switch (fn) {
	case PTL_GASUP_FN_COMM_REGISTER:
		msg.datalen = 5;
		msg.data = &msg.address[2];
		break;
	case PTL_GASUP_FN_COMM_HEARTBEAT:
		msg.datalen = 5;
		msg.data = &msg.address[2];
		break;
	case PTL_GASUP_FN_ALM_CON:
		if (data == NULL)
			return FALSE;
		msg.datalen = len;
		msg.data = data;
		break;
	case PTL_GASUP_FN_ALM_METER:
		if (data == NULL)
			return FALSE;
		msg.datalen = len;
		msg.data = data;
		break;
	default:
		return FALSE;
	}
	return plt_gasup_pack(buf, max_len, &msg);
}

int ptl_gasup_pack_gasmeteralarm_data(BYTE *buf, int maxlen, WORD *pfn,
		GASMETER_ALARM_T *palarm) {
	BYTE *ptr = buf;
	struct tm tm;

	if (buf == NULL || maxlen < 28 || pfn == NULL || palarm == NULL)
		return 0;
	memcpy(ptr, palarm->address, 5);
	ptr += 5;
	memcpy(ptr, palarm->collector, 5);
	ptr += 5;
	memcpy(ptr, palarm->meterid, 7);
	ptr += 7;
	localtime_r(&palarm->data.alarm_tt, &tm);
	bcd_be_stoc(ptr, tm.tm_year + 1900);
	ptr += 2;
	*ptr++ = bin_to_bcd(tm.tm_mon + 1);
	*ptr++ = bin_to_bcd(tm.tm_mday);
	*ptr++ = bin_to_bcd(tm.tm_hour);
	*ptr++ = bin_to_bcd(tm.tm_min);
	*ptr++ = bin_to_bcd(tm.tm_sec);
	bcd_be_stoc(ptr, palarm->data.type);
	ptr += 2;
	return ptr - buf;
}

int ptl_gasup_pack_conalarm_data(BYTE *buf, int maxlen, WORD *pfn,
		CON_ALARM_T *palarm) {
	BYTE *ptr = buf;
	struct tm tm;

	if (buf == NULL || maxlen < 21 || pfn == NULL || palarm == NULL)
		return 0;
	memcpy(ptr, palarm->address, 5);
	ptr += 5;
	memcpy(ptr, palarm->collector, 5);
	ptr += 5;
	localtime_r(&palarm->data.alarm_tt, &tm);
	bcd_be_stoc(ptr, tm.tm_year + 1900);
	ptr += 2;
	*ptr++ = bin_to_bcd(tm.tm_mon + 1);
	*ptr++ = bin_to_bcd(tm.tm_mday);
	*ptr++ = bin_to_bcd(tm.tm_hour);
	*ptr++ = bin_to_bcd(tm.tm_min);
	*ptr++ = bin_to_bcd(tm.tm_sec);
	bcd_be_stoc(ptr, palarm->data.type);
	ptr += 2;
	return ptr - buf;
}

BOOL plt_gasup_check_pack_special(const UINT8 *address, UINT16 fn, UINT8 *buf,
		UINT32 len, INT32 packetID) {
	UINT8 *ptr = buf, zero[2];
	PTL_GASUP_MSG msg;
	INT32 packetID1;
	UINT16 fns[] = { PTL_GASUP_FN_COMM_HEARTBEAT, PTL_GASUP_FN_COMM_REGISTER,
			PTL_GASUP_FN_ALM_CON, PTL_GASUP_FN_ALM_METER, };
	int i;
	BOOL is_valid_fn = FALSE;

	if (ptr == NULL)
		return FALSE;
	while (*ptr != 0x68 && len > 1) {
		ptr++;
		len--;
	}
	if (!plt_gasup_unpack(&msg, ptr, len))
		return FALSE;
	if (memcmp(address, msg.address, 7)) {
		PRINTF("Check special packet Fail, address error\n");
		return FALSE;
	}
	memset(zero, 0, 2);
	bcd_be_ctol(&msg.packetID[3], &packetID1);
	for (i = 0; i < ARRAY_SIZE(fns); i++) {
		if (msg.fn == fn) {
			is_valid_fn = TRUE;
			break;
		}
	}
	if (!is_valid_fn)
		return FALSE;
	if (packetID != packetID1) {
		PRINTF("Check special packet Fail, packetID error(OK: %d, ERR: %d)\n",
				packetID, packetID1);
		return FALSE;
	}
	if (msg.datalen != 7) {
		PRINTF("Check special packet Fail, datalen error: %d\n", msg.datalen);
		return FALSE;
	}
	if (memcmp(msg.data, zero, 2)) {
		PRINTB("Check special packet Fail, response code error:\n", msg.data,
				2);
		return FALSE;
	}
	return TRUE;
}

INT32 plt_gasup_parse(const UINT8 *buf, INT32 buf_len) {
	const UINT8 *ptr = buf;
	PTL_GASUP_MSG msg;
	INT32 used_len = 0;

	if (ptr == NULL)
		return FALSE;
	while (*ptr != 0x68 && buf_len > 1) {
		ptr++;
		used_len++;
		buf_len--;
	}
	if (buf_len <= 0) /// Not found 0x68, so discard this data
		return used_len;
	if (buf_len < 26)
		return 0;
	ptr++;
	msg.packetlen = ctos(ptr);
	if (buf_len < msg.packetlen)
		return 0;
	ptr += 20;
	msg.datalen = ctos(ptr);
	if (buf_len < 26 + msg.datalen)
		return 0;
	used_len += (26 + msg.datalen);
	PRINTF("Parse GASUP OK, used length = %d\n", used_len);
	return used_len;
}

int plt_gasup_read_socket(int fd, void *buf, int maxlen, int timeout) {
	BYTE *ptr = buf;
	int packetlen, len;

	if (fd < 0 || buf == NULL)
		return 0;
	if (wait_for_ready(fd, timeout, 0) > 0) { /// timeout for checking if data ready
		while (maxlen > 0) {
			len = recv(fd, ptr, 1, 0);
			if (len == 1 && *ptr == 0x68) {
				ptr++;
				maxlen--;
				break;
			} else
				return 0; /// message head not valid
		} /// read and check message head /// protocol factor
		len = safe_read_timeout(fd, ptr, 2, 100); // read for message len
		if (len != 2) { // exception
			safe_read_timeout(fd, ptr, maxlen, 100);
			return 0;
		}
		packetlen = ctos(ptr);
		ptr += 2;
		maxlen -= 2;
		if (maxlen < packetlen - 3)
			return 0;
		len = safe_read_timeout(fd, ptr, packetlen - 3, 100); /// read remains message bytes
		if (len == packetlen - 3) { /// length suites
			ptr += (packetlen - 3);
			maxlen -= (packetlen - 3); /// remain length for read and save, from upper
			return ptr - (BYTE *) buf; /// read length, is just 
		} else
			return 0;
	} else
		return 0;
}

static INT32 find_idx(INT32 in_idx, INT32 out_idx) {
	INT32 i;

	for (i = 0; i < sizeof(msg_arr) / sizeof(struct msg); i++) {
		if (in_idx == msg_arr[i].que_in && out_idx == msg_arr[i].que_out)
			return i;
	}
	return -1;
}

static void msg_pack(struct msg *msg_ptr, INT32 max_out_len, INT32 *datalen,
		INT32 data_cnt) {
	INT8 *ptr = (INT8 *)msg_ptr->buf;
	int idx;
	PTL_GASUP_MSG *ptl_msg_in, *ptl_msg_out;

	if (msg_ptr == NULL)
		return;
	msg_ptr->msg_out_cnt = 0;
	if (datalen == NULL || data_cnt <= 0)
		return;
	ptl_msg_in = &msg_ptr->msg_in;
	for (idx = 0; idx < min(data_cnt, MAX_PACK_CNT); idx++) {
		if (max_out_len < datalen[idx])
			break;
		ptl_msg_out = &msg_ptr->msg_out[idx];
		ptl_msg_out->fn = ptl_msg_in->fn;
		ptl_msg_out->direction = PTL_GASUP_DIR_SLAVE;
		ptl_msg_out->flag = PTL_GASUP_NEED_RESPOND;
		memcpy(ptl_msg_out->address, ptl_msg_in->address, 7);
		memcpy(ptl_msg_out->packetID, ptl_msg_in->packetID, 7);
		ptl_msg_out->datalen = datalen[idx];
		ptl_msg_out->data = ptr;
		ptr += ptl_msg_out->datalen;
		max_out_len -= ptl_msg_out->datalen;
		msg_ptr->msg_out_cnt++;
	}
}

struct {
	UINT16 fn;
	UINT32 (*fn_proc)(const PTL_GASUP_MSG *, INT8 *, INT32, INT32 *, INT32);
	const char *describe;
} gasup_fn_arr[] = { //// up function struct
				{ 2001, ptl_gasup_fn_2001, "main station communication" }, /// 业务名称
				{ 2002, ptl_gasup_fn_2002, "concentrator test" }, { 2003,
						ptl_gasup_fn_2003, "heartbeat" }, { 2004,
						ptl_gasup_fn_2004, "registeration in main station" }, {
						2011, ptl_gasup_fn_2011, "set heartbeat" }, { 2012,
						ptl_gasup_fn_2012, "assembly read time set" }, { 2013,
						ptl_gasup_fn_2013, "" },
				{ 2014, ptl_gasup_fn_2014, "" },
				{ 2015, ptl_gasup_fn_2015, "" },
				{ 2016, ptl_gasup_fn_2016, "" },
				{ 2021, ptl_gasup_fn_2021, "" },
				{ 2022, ptl_gasup_fn_2022, "" },
				{ 2023, ptl_gasup_fn_2023, "" },
				{ 2024, ptl_gasup_fn_2024, "" },
				{ 2031, ptl_gasup_fn_2031, "" },
				{ 2032, ptl_gasup_fn_2032, "" },
				{ 2033, ptl_gasup_fn_2033, "" },
				{ 2034, ptl_gasup_fn_2034, "" },
				{ 2035, ptl_gasup_fn_2035, "" },
				{ 2036, ptl_gasup_fn_2036, "" },
				{ 2041, ptl_gasup_fn_2041, "" },
				{ 2042, ptl_gasup_fn_2042, "" },
				{ 2043, ptl_gasup_fn_2043, "" },
				{ 2044, ptl_gasup_fn_2044, "" },
				{ 2051, ptl_gasup_fn_2051, "" },
				{ 2052, ptl_gasup_fn_2052, "" },
				{ 2053, ptl_gasup_fn_2053, "" },
				{ 2061, ptl_gasup_fn_2061, "" }, { 2062, ptl_gasup_fn_2062, "" }, };

static UINT32 plt_gasup_fn(const PTL_GASUP_MSG *msg, INT8 *outdata,
		INT32 max_outlen, INT32 *datalen, INT32 data_cnt) {
	int i;

	if (msg == NULL || outdata == NULL || max_outlen <= 0 || datalen == NULL
			|| data_cnt <= 0)
		return 0;
	for (i = 0; i < ARRAY_SIZE(gasup_fn_arr); i++) {
		if (msg->fn == gasup_fn_arr[i].fn && gasup_fn_arr[i].fn_proc) {
			PRINTF("GASUP packet (Fn: %04d)\n", msg->fn);
			return gasup_fn_arr[i].fn_proc(msg, outdata, max_outlen, datalen,
					data_cnt);
		}
	}
	PRINTF("GASUP unknown packet (Fn: %04d)\n", msg->fn);
	return 0;
}

static BOOL plt_gasup_check_msg(PTL_GASUP_MSG *ptl_msg) {
	int i;
	BOOL is_valid_fn = FALSE;
	BYTE address[7];

	if (ptl_msg == NULL)
		return FALSE;
	if (ptl_msg->direction != PTL_GASUP_DIR_MASTER) {
		PRINTF("GASUP check fail, direction error\n");
		return FALSE;
	}
	if (ptl_msg->flag != PTL_GASUP_NEED_REQUEST) {
		PRINTF("GASUP check fail, flag error\n");
		return FALSE;
	}
	fparam_get_value(FPARAMID_CON_ADDRESS, address, 7);
	if (memcmp(ptl_msg->address, address, 7)) {
		PRINTF("GASUP check fail, concentrator address error\n");
		return FALSE;
	}
	for (i = 0; i < ARRAY_SIZE(gasup_fn_arr); i++) {
		if (ptl_msg->fn == gasup_fn_arr[i].fn && gasup_fn_arr[i].fn_proc) {
			is_valid_fn = TRUE;
			break;
		}
	}
	if (!is_valid_fn) {
		PRINTF("GASUP check fail, unknown fn (%d)\n", ptl_msg->fn);
		return FALSE;
	}
	return TRUE;
}

void plt_gasup_proc(INT32 in_idx, INT32 out_idx, INT32 max_out_len, void *priv) {
	UINT8 in_buf[CONFIG_MAX_APDU_LEN], out_buf[CONFIG_MAX_APDU_LEN];
	INT32 in_len, out_len, idx;
	INT32 data_len[MAX_PACK_CNT], data_cnt;
	struct msg *msg_ptr;
	UP_COMM_PRIVATE *up_private = (UP_COMM_PRIVATE *) priv;
	UINT8 address[7] = { 0 };

	max_out_len = min(sizeof(out_buf), max_out_len);
	msg_que_get(in_idx, in_buf, sizeof(in_buf), &in_len, 0);
	if (in_len <= 0 || (idx = find_idx(in_idx, out_idx)) < 0) {
		PRINTF("%s: argument check error\n", __FUNCTION__);
		return;
	}

	fparam_get_value(FPARAMID_CON_ADDRESS, address, 7);
	if (up_private && up_private->hb_status == e_up_wait_response) {
		if (plt_gasup_check_pack_special(address, PTL_GASUP_FN_COMM_HEARTBEAT,
				in_buf, in_len, up_private->save_hb_packetID)) {
			PRINTF("HEART BEAT Response OK\n");
			up_private->hb_status = e_up_finish;
			return;
		} else {
			PRINTF("%s:  HEART BEAT Response NOT OK\n", __FUNCTION__);
		}
	}
	if (up_private && up_private->spont_status == e_up_wait_response) {
		if (plt_gasup_check_pack_special(address, PTL_GASUP_FN_ALM_CON, in_buf,
				in_len, up_private->save_spont_packetID)) {
			PRINTF("SPONT CONCENTRATOR ALARM Response OK\n");
			up_private->spont_status = e_up_finish;
			spontalarm_set_response(PTL_GASUP_FN_ALM_CON,
					up_private->spont_chnidx, NULL, 0);
			return;
		} else {
			PRINTF("%s: SPONT CONCENTRATOR ALARM Response NOT OK\n",
					__FUNCTION__);
		}
		if (plt_gasup_check_pack_special(address, PTL_GASUP_FN_ALM_METER,
				in_buf, in_len, up_private->save_spont_packetID)) {
			PRINTF("SPONT GASMETER ALARM Response OK\n");
			up_private->spont_status = e_up_finish;
			spontalarm_set_response(PTL_GASUP_FN_ALM_METER,
					up_private->spont_chnidx, NULL, 0);
			return;
		} else {
			PRINTF("%s: SPONT GASMETER ALARM Response NOT OK\n", __FUNCTION__);
		}
	}
	msg_ptr = msg_arr + idx;
	if (!plt_gasup_unpack(&msg_ptr->msg_in, in_buf, in_len)) {
		PRINTF("UNPACK GASUP ERROR\n");
		return;
	}
	PRINTF("Unpack GASUP OK\n");

	if (!plt_gasup_check_msg(&msg_ptr->msg_in)) {
		PRINTF("CHECK MESSAGE GASUP ERROR\n");
		return;
	} else {
		; /// log
	}
	///PRINTF("CHECKMESSAGE OK\n");
	msg_ptr->msg_out_cnt = 0;
	data_cnt = (INT32)plt_gasup_fn(&msg_ptr->msg_in, msg_ptr->buf, MSG_BUF_LEN,
			data_len, ARRAY_SIZE(data_len));
	msg_pack(msg_ptr, max_out_len, data_len, data_cnt);

	for (idx = 0; idx < msg_ptr->msg_out_cnt; idx++) {
		out_len = plt_gasup_pack(out_buf, max_out_len, msg_ptr->msg_out + idx);
		if (out_len > 0)
			msg_que_put(msg_ptr->que_out, out_buf, out_len, MSG_QUE_NO_STAMP);
	}
}

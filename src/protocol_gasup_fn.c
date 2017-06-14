/*
 * protocol_gasup_fn.c
 *
 *  Created on: 2015-8-16
 *      Author: Johnnyzhang
 */

#include "protocol_gasup_fn.h"
#include "common.h"
#include "main.h"
#include "f_param.h"
#include "f_gasmeter.h"
#include "protocol_cjt188_data.h"
#include "f_current.h"
#include "f_day.h"
#include "f_month.h"

#define MAX_BLOCKDATA_CNT_IN_PACKET 20
#define MAX_METERDATA_CNT_IN_PACKET 30

UINT32 ptl_gasup_fn_2001(const PTL_GASUP_MSG *msg, INT8 *outdata, INT32 max_outlen, INT32 *datalen, INT32 max_datalen)
{
	INT8 *ptr = outdata;
	if (msg == NULL || msg->data == NULL || msg->datalen < 5 || outdata == NULL
			|| max_outlen < 7 || max_datalen < 1)
		return 0;
	if (memcmp(&msg->address[2], msg->data, 5) == 0) {
		bcd_be_stoc(ptr, 0);
	}
	else {
		bcd_be_stoc(ptr,2001);
	}
	ptr += 2;
	memcpy(ptr, &msg->address[2], 5);
	ptr += 5;
	datalen[0] = ptr - outdata;
	return 1;
}

UINT32 ptl_gasup_fn_2002(const PTL_GASUP_MSG *msg, INT8 *outdata, INT32 max_outlen, INT32 *datalen, INT32 max_datalen)
{
	// Nothing to do
	return 0;
}

UINT32 ptl_gasup_fn_2003(const PTL_GASUP_MSG *msg, INT8 *outdata, INT32 max_outlen, INT32 *datalen, INT32 max_datalen)
{
	// Nothing to do
	return 0;
}

UINT32 ptl_gasup_fn_2004(const PTL_GASUP_MSG *msg, INT8 *outdata, INT32 max_outlen, INT32 *datalen, INT32 max_datalen)
{
	// Nothing to do
	return 0;
}

UINT32 ptl_gasup_fn_2011(const PTL_GASUP_MSG *msg, INT8 *outdata, INT32 max_outlen, INT32 *datalen, INT32 max_datalen)
{
	INT8 *ptr = outdata;
	if (msg == NULL || msg->data == NULL || msg->datalen < 7 || outdata == NULL
			|| max_outlen < 7 || max_datalen < 1)
		return 0;
	if (memcmp(&msg->address[2], msg->data, 5) == 0) {
		fparam_set_value(FPARAMID_HEARTBEAT_CYCLE, &msg->data[5], 2);
		bcd_be_stoc(ptr, 0);
	}
	else {
		bcd_be_stoc(ptr,2101);
	}
	ptr += 2;
	memcpy(ptr, &msg->address[2], 5);
	ptr += 5;
	datalen[0] = ptr - outdata;
	return 1;
}

UINT32 ptl_gasup_fn_2012(const PTL_GASUP_MSG *msg, INT8 *outdata, INT32 max_outlen, INT32 *datalen, INT32 max_datalen)
{
	INT8 *ptr = outdata;
	if (msg == NULL || msg->data == NULL || msg->datalen < 5 || outdata == NULL
			|| max_outlen < 7 || max_datalen < 1)
		return 0;
	if (memcmp(&msg->address[2], msg->data, 5) == 0) {
		bcd_be_stoc(ptr, 0);
	}
	else {
		bcd_be_stoc(ptr,2102);
	}
	ptr += 2;
	memcpy(ptr, &msg->address[2], 5);
	ptr += 5;
	fparam_get_value(FPARAMID_HEARTBEAT_CYCLE, ptr, 2);
	ptr += 2;
	datalen[0] = ptr - outdata;
	return 1;
}

UINT32 ptl_gasup_fn_2013(const PTL_GASUP_MSG *msg, INT8 *outdata, INT32 max_outlen, INT32 *datalen, INT32 max_datalen)
{
	INT8 *ptr = outdata;
	if (msg == NULL || msg->data == NULL || msg->datalen < 7 || outdata == NULL
			|| max_outlen < 7 || max_datalen < 1)
		return 0;
	if (memcmp(&msg->address[2], msg->data, 5) == 0) {
		fparam_set_value(FPARAMID_READMETER_FREQ, &msg->data[5], 2);
		bcd_be_stoc(ptr, 0);
	}
	else {
		bcd_be_stoc(ptr,2111);
	}
	ptr += 2;
	memcpy(ptr, &msg->address[2], 5);
	ptr += 5;
	datalen[0] = ptr - outdata;
	return 1;
}

UINT32 ptl_gasup_fn_2014(const PTL_GASUP_MSG *msg, INT8 *outdata, INT32 max_outlen, INT32 *datalen, INT32 max_datalen)
{
	INT8 *ptr = outdata;
	if (msg == NULL || msg->data == NULL || msg->datalen < 5 || outdata == NULL
			|| max_outlen < 7 || max_datalen < 1)
		return 0;
	if (memcmp(&msg->address[2], msg->data, 5) == 0) {
		bcd_be_stoc(ptr, 0);
	}
	else {
		bcd_be_stoc(ptr,2112);
	}
	ptr += 2;
	memcpy(ptr, &msg->address[2], 5);
	ptr += 5;
	fparam_get_value(FPARAMID_READMETER_FREQ, ptr, 2);
	ptr += 2;
	datalen[0] = ptr - outdata;
	return 1;
}

UINT32 ptl_gasup_fn_2015(const PTL_GASUP_MSG *msg, INT8 *outdata, INT32 max_outlen, INT32 *datalen, INT32 max_datalen)
{
	struct tm tm;
	struct timeval tv;
	const BYTE *inptr;
	WORD year;
	INT8 *outptr = outdata;

	if (msg == NULL || msg->data == NULL || msg->datalen < 12 || outdata == NULL
			|| max_outlen < 7 || max_datalen < 1)
		return 0;
	inptr = msg->data;
	if (memcmp(&msg->address[2], msg->data, 5) == 0) {
		inptr += 5;
		bcd_be_ctos(inptr, &year);
		inptr += 2;
		tm.tm_isdst = -1;
		tm.tm_year = year - 1900;
		tm.tm_mon = bcd_to_bin(*inptr++) - 1;
		tm.tm_mday = bcd_to_bin(*inptr++);
		tm.tm_hour = bcd_to_bin(*inptr++);
		tm.tm_min = bcd_to_bin(*inptr++);
		tm.tm_sec = bcd_to_bin(*inptr++);
		tv.tv_sec = mktime(&tm);
		tv.tv_usec = 0;
		settimeofday(&tv, NULL);
		set_rtc();
		bcd_be_stoc(outptr, 0);
	}
	else {
		bcd_be_stoc(outptr,2121);
	}
	outptr += 2;
	memcpy(outptr, &msg->address[2], 5);
	outptr += 5;
	datalen[0] = outptr - outdata;
	return 1;
}

UINT32 ptl_gasup_fn_2016(const PTL_GASUP_MSG *msg, INT8 *outdata, INT32 max_outlen, INT32 *datalen, INT32 max_datalen)
{
	INT8 *ptr = outdata;
	struct tm tm;

	if (msg == NULL || msg->data == NULL || msg->datalen < 5 || outdata == NULL
			|| max_outlen < 14 || max_datalen < 1)
		return 0;
	if (memcmp(&msg->address[2], msg->data, 5) == 0) {
		bcd_stoc(ptr, 0);
		ptr += 2;
		memcpy(ptr, &msg->address[2], 5);
		ptr += 5;
		read_rtc();
		sys_time(&tm);
		bcd_be_stoc(ptr, tm.tm_year + 1900);
		ptr += 2;
		*ptr++ = bin_to_bcd(tm.tm_mon + 1);
		*ptr++ = bin_to_bcd(tm.tm_mday);
		*ptr++ = bin_to_bcd(tm.tm_hour);
		*ptr++ = bin_to_bcd(tm.tm_min);
		*ptr++ = bin_to_bcd(tm.tm_sec);
	}
	else {
		bcd_be_stoc(ptr,2122);
		ptr += 2;
		memcpy(ptr, &msg->address[2], 5);
		ptr += 5;
		memset(ptr, 0, 7);
		ptr += 7;
	}
	datalen[0] = ptr - outdata;
	return 1;
}

UINT32 ptl_gasup_fn_2021(const PTL_GASUP_MSG *msg, INT8 *outdata, INT32 max_outlen, INT32 *datalen, INT32 max_datalen)
{
	INT8 *ptr = outdata;
	INT8 address[7], collector[5];
	WORD wakeup_cycle;
	int gasmeter_index;

	if (msg == NULL || msg->data == NULL || msg->datalen < 19 || outdata == NULL
			|| max_outlen < 7 || max_datalen < 1)
		return 0;
	if (memcmp(&msg->address[2], msg->data, 5) == 0) {
		memcpy(collector, &msg->data[5], 5);
		memcpy(address, &msg->data[10], 7);
		wakeup_cycle = ctos_be(msg->data + 17);
		gasmeter_index = fgasmeter_getidx_by_gasmeter(address);
		if (gasmeter_index < 0) {
			bcd_be_stoc(ptr,2201);
		}
		else {
			fgasmeter_setgasmeter_wakeupcycle(gasmeter_index, wakeup_cycle);
			bcd_be_stoc(ptr, 0);
		}
	}
	else {
		bcd_be_stoc(ptr,2201);
	}
	ptr += 2;
	memcpy(ptr, msg->data, 17);
	ptr += 17;
	datalen[0] = ptr - outdata;
	return 1;
}

UINT32 ptl_gasup_fn_2022(const PTL_GASUP_MSG *msg, INT8 *outdata, INT32 max_outlen, INT32 *datalen, INT32 max_datalen)
{
	INT8 *ptr = outdata;
	INT8 address[7], collector[5];
	WORD wakeup_cycle;
	int gasmeter_index;

	if (msg == NULL || msg->data == NULL || msg->datalen < 17 || outdata == NULL
			|| max_outlen < 21 || max_datalen < 1)
		return 0;
	if (memcmp(&msg->address[2], msg->data, 5) == 0) {
		memcpy(collector, &msg->data[5], 5);
		memcpy(address, &msg->data[10], 7);
		gasmeter_index = fgasmeter_getidx_by_gasmeter(address);
		if (gasmeter_index < 0) {
			bcd_be_stoc(ptr,2202);
			ptr += 2;
			memcpy(ptr, msg->data, 17);
			ptr += 17;
			memset(ptr, 0, 2);
			ptr += 2;
		}
		else {
			bcd_stoc(ptr, 0);
			ptr += 2;
			memcpy(ptr, msg->data, 17);
			ptr += 17;
			wakeup_cycle = fgasmeter_getgasmeter_wakeupcycle(gasmeter_index);
			stoc_be(ptr, wakeup_cycle);
			ptr += 2;
		}
	}
	else {
		bcd_be_stoc(ptr,2202);
		ptr += 2;
		memcpy(ptr, msg->data, 17);
		ptr += 17;
		memset(ptr, 0, 2);
		ptr += 2;
	}
	datalen[0] = ptr - outdata;
	return 1;
}

int gasmeter_read_di(const BYTE *address, const BYTE *collector, WORD di, BYTE *buf, int max_len);
BOOL gasmeter_write_di(const BYTE *address, const BYTE *collector, WORD di, BYTE *buf, int buflen);

UINT32 ptl_gasup_fn_2023(const PTL_GASUP_MSG *msg, INT8 *outdata, INT32 max_outlen, INT32 *datalen, INT32 max_datalen)
{
	INT8 *ptr = outdata;
	UINT8 req[64];
	int i;

	if (msg == NULL || msg->data == NULL || msg->datalen < 24 || outdata == NULL
			|| max_outlen < 19 || max_datalen < 1)
		return 0;
	if (memcmp(&msg->address[2], msg->data, 5) == 0) {
		for (i = 0; i < 7; i++) {
			req[7 - i - 1] = msg->data[17 + i];
		}
		if (gasmeter_write_di(&msg->data[10], &msg->data[5], 0xA015, req, 7)) {
			bcd_be_stoc(ptr, 0);
		}
		else {
			bcd_be_stoc(ptr, 2121);
		}
	}
	else {
		bcd_be_stoc(ptr,2121);
	}
	ptr += 2;
	memcpy(ptr, msg->data, 17);
	ptr += 17;
	datalen[0] = ptr - outdata;
	return 1;
}

UINT32 ptl_gasup_fn_2024(const PTL_GASUP_MSG *msg, INT8 *outdata, INT32 max_outlen, INT32 *datalen, INT32 max_datalen)
{
	INT8 *ptr = outdata;
	UINT8 resp[64];
	int resp_len, i;

#define ERROR_CODE_2024  2122

	if (msg == NULL || msg->data == NULL || msg->datalen < 17 || outdata == NULL
			|| max_outlen < 26 || max_datalen < 1)
		return 0;
	if (memcmp(&msg->address[2], msg->data, 5) == 0) {
		resp_len = gasmeter_read_di(&msg->data[10], &msg->data[5], 0x8515, resp, sizeof(resp)); // 8515
		if (resp_len >= 3 + 7) {
			bcd_be_stoc(ptr, 0);
			ptr += 2;
			memcpy(ptr, msg->data, 17);
			ptr += 17;
			for (i = 0; i < 7; i++) {
				*ptr++ = resp[3 + 7 - i - 1];
			}
		}
		else {
			bcd_be_stoc(ptr, ERROR_CODE_2024);
			ptr += 2;
			memcpy(ptr, msg->data, 17);
			ptr += 17;
		}
	}
	else {
		bcd_be_stoc(ptr,ERROR_CODE_2024);
		ptr += 2;
		memcpy(ptr, msg->data, 17);
		ptr += 17;
	}
	datalen[0] = ptr - outdata;
	return 1;
}

UINT32 ptl_gasup_fn_2031(const PTL_GASUP_MSG *msg, INT8 *outdata, INT32 max_outlen, INT32 *datalen, INT32 max_datalen)
{
	INT8 *ptr = outdata;
	int index;

	if (msg == NULL || msg->data == NULL || msg->datalen < 10 || outdata == NULL
			|| max_outlen < 7 || max_datalen < 1)
		return 0;
	if (memcmp(&msg->address[2], msg->data, 5) == 0) {
		index = fgasmeter_addcollector(&msg->data[5]);
		if (index >= 0 && index < MAX_GASMETER_NUMBER) {
			bcd_stoc(ptr, 0);
		}
		else {
			bcd_be_stoc(ptr, 2301);
		}
	}
	else {
		bcd_be_stoc(ptr,2301);
	}
	ptr += 2;
	memcpy(ptr, &msg->address[2], 5);
	ptr += 5;
	datalen[0] = ptr - outdata;
	return 1;
}

UINT32 ptl_gasup_fn_2032(const PTL_GASUP_MSG *msg, INT8 *outdata, INT32 max_outlen, INT32 *datalen, INT32 max_datalen)
{
	INT8 *ptr = outdata;

	if (msg == NULL || msg->data == NULL || msg->datalen < 10 || outdata == NULL
			|| max_outlen < 7 || max_datalen < 1)
		return 0;
	if (memcmp(&msg->address[2], msg->data, 5) == 0) {
		if (fgasmeter_delcollector(&msg->data[5])) {
			bcd_stoc(ptr, 0);
		}
		else {
			bcd_be_stoc(ptr, 2302);
		}
	}
	else {
		bcd_be_stoc(ptr,2302);
	}
	ptr += 2;
	memcpy(ptr, &msg->address[2], 5);
	ptr += 5;
	datalen[0] = ptr - outdata;
	return 1;
}

UINT32 ptl_gasup_fn_2033(const PTL_GASUP_MSG *msg, INT8 *outdata, INT32 max_outlen, INT32 *datalen, INT32 max_datalen)
{
	INT8 *ptr = outdata;

	if (msg == NULL || msg->data == NULL || msg->datalen < 17 || outdata == NULL
			|| max_outlen < 12 || max_datalen < 1)
		return 0;
	if (memcmp(&msg->address[2], msg->data, 5) == 0) {
		if (fgasmeter_addgasmeter(&msg->data[10], &msg->data[5])) {
			bcd_stoc(ptr, 0);
		}
		else {
			bcd_be_stoc(ptr, 2303);
		}
	}
	else {
		bcd_be_stoc(ptr,2303);
	}
	ptr += 2;
	memcpy(ptr, msg->data, 10);
	ptr += 10;
	datalen[0] = ptr - outdata;
	return 1;
}

UINT32 ptl_gasup_fn_2034(const PTL_GASUP_MSG *msg, INT8 *outdata, INT32 max_outlen, INT32 *datalen, INT32 max_datalen)
{
	INT8 *ptr = outdata;
	INT8 *collector;
	INT8 zerobuf[5];

	if (msg == NULL || msg->data == NULL || msg->datalen < 17 || outdata == NULL
			|| max_outlen < 12 || max_datalen < 1)
		return 0;
	memset(zerobuf, 0, sizeof(zerobuf));
	if (memcmp(&msg->address[2], msg->data, 5) == 0) {
		if (memcmp(&msg->data[5], zerobuf, 5) == 0) {
			collector = NULL;
		}
		else {
			collector = &msg->data[5];
		}
		if (fgasmeter_delgasmeter(&msg->data[10], collector)) {
			bcd_stoc(ptr, 0);
		}
		else {
			bcd_be_stoc(ptr, 2304);
		}
	}
	else {
		bcd_be_stoc(ptr,2304);
	}
	ptr += 2;
	memcpy(ptr, msg->data, 10);
	ptr += 10;
	datalen[0] = ptr - outdata;
	return 1;
}

UINT32 ptl_gasup_fn_2035(const PTL_GASUP_MSG *msg, INT8 *outdata, INT32 max_outlen, INT32 *datalen, INT32 max_datalen)
{
	INT8 *ptr = outdata;
	INT8 *cntptr;
	int collector_cnt = 0;

	int i;

	if (msg == NULL || msg->data == NULL || msg->datalen < 5 || outdata == NULL
			|| max_outlen < 9 || max_datalen < 1)
		return 0;
	if (memcmp(&msg->address[2], msg->data, 5) == 0) {
		bcd_stoc(ptr, 0);
		ptr += 2;
		memcpy(ptr, msg->data, 5);
		ptr += 5;
		cntptr = ptr;
		ptr += 2;
		for (i = 0; i < MAX_GASMETER_NUMBER; i++) {
			if (max_outlen < 9 + (collector_cnt + 1) * 5)
				break;
			if (fgasmeter_getcollector(i, ptr)) {
				ptr += 5;
				collector_cnt++;
			}
		}
		stoc(cntptr, collector_cnt);
	}
	else {
		bcd_be_stoc(ptr,2305);
		ptr += 2;
		memcpy(ptr, msg->data, 5);
		ptr += 5;
		stoc_be(ptr, 0);
		ptr += 2;
	}
	datalen[0] = ptr - outdata;
	return 1;
}

UINT32 ptl_gasup_fn_2036(const PTL_GASUP_MSG *msg, INT8 *outdata, INT32 max_outlen, INT32 *datalen, INT32 max_datalen)
{
	INT8 *ptr = outdata;
	INT8 *cntptr;
	int cnt = 0, i;
	INT8 address[7], collector[5];
	INT8 zerobuf[5];
	BOOL b_get_all;

	if (msg == NULL || msg->data == NULL || msg->datalen < 10 || outdata == NULL
			|| max_outlen < 9 || max_datalen < 1)
		return 0;
	memset(zerobuf, 0, sizeof(zerobuf));
	if (memcmp(&msg->address[2], msg->data, 5) == 0) {
		bcd_stoc(ptr, 0);
		ptr += 2;
		memcpy(ptr, msg->data, 10);
		ptr += 10;
		cntptr = ptr;
		ptr += 2;
		if (memcmp(&msg->data[5], zerobuf, 5) == 0) {
			b_get_all = TRUE;
		}
		else {
			b_get_all = FALSE;
		}
		for (i = 0; i < MAX_GASMETER_NUMBER; i++) {
			if (max_outlen < 9 + (cnt + 1) * 7)
				break;
			if (!fgasmeter_getgasmeter(i, address, collector))
				continue;
			if (b_get_all) {
				memcpy(ptr, address, 7);
				ptr += 7;
				cnt++;
			}
			else {
				if (memcmp(collector, &msg->data[5], 5) == 0) {
					memcpy(ptr, address, 7);
					ptr += 7;
					cnt++;
					break;
				}
			}
		}
		stoc(cntptr, cnt);
	}
	else {
		bcd_be_stoc(ptr,2306);
		ptr += 2;
		memcpy(ptr, msg->data, 10);
		ptr += 10;
		stoc_be(ptr, 0);
		ptr += 2;
	}
	datalen[0] = ptr - outdata;
	return 1;
}

void meter_flux_to_mainstation(BYTE *mt_value, BYTE *ms_value, int len){
	long value = 0;
	int i, i_p, f_p ;
	BYTE byte[6];

	value = reverse_byte_array2bcd(mt_value, 5);

	f_p = value % 10;
	f_p *= 100;

	byte[0] = f_p % 256;
	byte[1] = f_p / 256;

	i_p = value / 10;
	for(i = 2; i< 6; i++){
		byte[i] = (i_p % (1u << 8));
		i_p /= (1u << 8);
	}
	memcpy(ms_value, byte, len);
	//PRINTB("ms_value", ms_value, len);

	return;
}

static UINT32 ptl_gasup_pack_meterdata(const PTL_GASUP_MSG *msg, UINT8 *outbuf,
		INT32 max_outlen, GASMETER_CJT188_901F *pdata)
{
	UINT8 *ptr = outbuf;
	struct tm tm;
	VALVE_STATUS valve_status = VALVE_STATUS_NONE;

	if (outbuf == NULL || max_outlen < 13 || pdata == NULL)
		return 0;
	ptl_cjt188_data_print(pdata->di_data.di, pdata);
	localtime_r(&pdata->read_tt, &tm);
	bcd_be_stoc(ptr, tm.tm_year + 1900);
	ptr += 2;
	*ptr++ = bin_to_bcd(tm.tm_mon + 1);
	*ptr++ = bin_to_bcd(tm.tm_mday);
	*ptr++ = bin_to_bcd(tm.tm_hour);
	*ptr++ = bin_to_bcd(tm.tm_min);
	*ptr++ = bin_to_bcd(tm.tm_sec);
	switch (msg->fn) {
	case PTL_GASUP_FN_GET_DATA:
	case PTL_GASUP_FN_GET_ONE_RD_DATA:
		if (max_outlen < 15)
			return 0;
		if (GASMETER_READ_STATUS_NORMAL == pdata->status) {
			BYTE ms_value[6] = {0};
			if (msg->fn == PTL_GASUP_FN_GET_DATA) {
				PRINTF("PTL_GASUP_FN_GET_DATA\n");
				//memcpy(ptr, pdata->di_data.balance_flux, 5);
				//meter_flux_to_mainstation(pdata->di_data.balance_flux,ms_value,6);
				meter_flux_to_mainstation(pdata->di_data.flux,ms_value,6);
				memcpy(ptr, ms_value, 6);
			}
			else {
				PRINTF("PTL_GASUP_FN_GET_ONE_RD_DATA\n");
				//memcpy(ptr, pdata->di_data.flux, 5);
				meter_flux_to_mainstation(pdata->di_data.flux, ms_value,6);
				memcpy(ptr, ms_value, 6);
			}
			//ptr += 5;
			ptr += 6;
		}
		else {
			PRINTF("ELSE\n");
			//memset(ptr, 0, 5);
			//ptr += 5;
			memset(ptr, 0, 6);
			ptr += 6;
		}
		*ptr++ = 0;
		*ptr++ = pdata->status;
		switch ((pdata->di_data.st[0] >> 6) & 0x03) {
		case 0:
			valve_status = VALVE_STATUS_ON;
			break;
		case 1:
			valve_status = VALVE_STATUS_OFF;
			break;
		case 3:
			valve_status = VALVE_STATUS_ABORT;
			break;
		default:
			break;
		}
		*ptr++ = valve_status;
		break;
	case PTL_GASUP_FN_GET_MONTHDATA:
	case PTL_GASUP_FN_GET_DAYDATA:
		memcpy(ptr, pdata->di_data.flux, 5);
		ptr += 5;
		*ptr++ = 0;
		break;
	default:
		return 0;
	}
	return ptr - outbuf;
}

static UINT32 ptl_gasup_pack_meterdata_nak(const PTL_GASUP_MSG *msg, INT8 *outbuf, INT32 max_outlen)
{
	INT8 *ptr = outbuf;

	if (msg == NULL || msg->data == NULL || outbuf == NULL
			|| max_outlen < 24)
		return 0;
	switch (msg->fn) {
	case PTL_GASUP_FN_GET_DATA:
		if (msg->datalen < 12)
			return 0;
		bcd_be_stoc(ptr,2401);
		ptr += 2;
		memcpy(ptr, msg->data, 5);
		ptr += 5;
		break;
	case PTL_GASUP_FN_GET_ONE_RD_DATA:
		if (msg->datalen < 12)
			return 0;
		bcd_be_stoc(ptr,2402);
		ptr += 2;
		memcpy(ptr, msg->data, 17);
		ptr += 5;
		break;
	case PTL_GASUP_FN_GET_MONTHDATA:
		if (msg->datalen < 17)
			return 0;
		bcd_be_stoc(ptr,2403);
		ptr += 2;
		memcpy(ptr, msg->data, 17);
		ptr += 17;
		break;
	case PTL_GASUP_FN_GET_DAYDATA:
		if (msg->datalen < 17)
			return 0;
		bcd_be_stoc(ptr,2404);
		ptr += 2;
		memcpy(ptr, msg->data, 17);
		ptr += 17;
		break;
	default:
		return 0;
	}
	*ptr++ = 1;
	stoc(ptr, 1);
	ptr += 2;
	stoc(ptr, 0);
	ptr += 2;
	return ptr - outbuf;
}

UINT32 ptl_gasup_fn_2041(const PTL_GASUP_MSG *msg, INT8 *outdata, INT32 max_outlen, INT32 *datalen, INT32 max_datalen)
{
	INT8 *ptr = outdata, *lastframe_ptr = NULL, *pstart, *pframe_cnt[PTL_GASUP_MAX_PACK_CNT], *meter_detail;
	int i;
	UINT16 frame_cnt = 0;
	int mt_cnt, next_mtidx, mtidx;
	GASMETER_CJT188_901F di_data;
	int meterdata_len;
	long tt;
	struct tm tm;
	WORD year;

	if (msg == NULL || msg->data == NULL || msg->datalen < 12 || outdata == NULL
			|| max_outlen < 12 || max_datalen < 1)
		return 0;
	if (memcmp(&msg->address[2], msg->data, 5) == 0) {
		bcd_be_ctos(&msg->data[5], &year);
		tm.tm_isdst = -1;
		tm.tm_year = year - 1900;
		tm.tm_mon = bcd_to_bin(msg->data[7]) - 1;
		tm.tm_mday = bcd_to_bin(msg->data[8]);
		tm.tm_hour = bcd_to_bin(msg->data[9]);
		tm.tm_min = bcd_to_bin(msg->data[10]);
		tm.tm_sec = bcd_to_bin(msg->data[11]);
		tt = mktime(&tm);
		next_mtidx = 0;
		for (i = 0; i < PTL_GASUP_MAX_PACK_CNT; i++) {
			if (max_outlen < 12)
				break;
			for (mtidx = next_mtidx; mtidx < MAX_GASMETER_NUMBER; mtidx++) {
				if (fcurrent_get_data(mtidx, 0x901F, &di_data) && tt <= di_data.read_tt) /// from this mtidx on
					break;
			}
			if (mtidx >= MAX_GASMETER_NUMBER) {
				if (frame_cnt > 0)
					break;
				datalen[0] = ptl_gasup_pack_meterdata_nak(msg, ptr, max_outlen); /// no valid data
				if (datalen[0] > 0)
					return 1;
				else
					return 0;
			}
			pstart = ptr;
			bcd_be_stoc(ptr, 0);
			ptr += 2;
			memcpy(ptr, msg->data, 5);
			ptr += 5;
			lastframe_ptr = ptr;
			*ptr++ = 0;
			stoc(ptr, i + 1);
			ptr += 2;

			/*
			pframe_cnt[i] = ptr;
			stoc(ptr, 0);
			ptr += 2;
			*/

			mt_cnt = 0;
			meter_detail = ptr;
			ptr += 2;

			max_outlen -= 12;
			for (mtidx = next_mtidx; mtidx < MAX_GASMETER_NUMBER; mtidx++) {
				if (!fcurrent_get_data(mtidx, 0x901F, &di_data) 
						|| tt > di_data.read_tt)
					continue;
				fgasmeter_getgasmeter(mtidx, ptr, NULL);
				ptr += 7;
				max_outlen -= 7;
				meterdata_len = ptl_gasup_pack_meterdata(msg, ptr, max_outlen,
						&di_data);
				ptr += meterdata_len;
				max_outlen -= meterdata_len;
				mt_cnt++;

				if (mt_cnt >= MAX_METERDATA_CNT_IN_PACKET){ // mt_cnt > MAX_METERDATA_CNT_IN_PACKET
					break;
				}
			}
			stoc(meter_detail, (mt_cnt > MAX_METERDATA_CNT_IN_PACKET)?MAX_METERDATA_CNT_IN_PACKET:mt_cnt);

			next_mtidx = (mtidx + 1);
			frame_cnt++;
			datalen[i] = ptr - pstart;
		}

		if (frame_cnt > 0) {
			for (i = 0; i < frame_cnt; i++) {
				//stoc(pframe_cnt[i], frame_cnt); // comment by wd
			}
			if (lastframe_ptr) {
				*lastframe_ptr = 1;
			}
			return frame_cnt;
		}
		else {
			datalen[0] = ptl_gasup_pack_meterdata_nak(msg, ptr, max_outlen);
			if (datalen[0] > 0)
				return 1;
			else
				return 0;
		}
	}
	else {
		PRINTF("WARNNING: message intention and form did not match!\n");
		datalen[0] = ptl_gasup_pack_meterdata_nak(msg, ptr, max_outlen);
		if (datalen[0] > 0)
			return 1;
		else
			return 0;
	}
	return 0;
}

UINT32 ptl_gasup_fn_2042(const PTL_GASUP_MSG *msg, INT8 *outdata, INT32 max_outlen, INT32 *datalen, INT32 max_datalen)
{
	INT8 *ptr = outdata;
	GASMETER_CJT188_901F di_data;
	int retry_cnt = 0, meterdata_len;
	BOOL respone_nak = FALSE;
	UINT8 resp[64];
	int resp_len = 0;
	WORD di = 0x901F;
	long tt = time(NULL);

	if (msg == NULL || msg->data == NULL || msg->datalen < 17 || outdata == NULL
			|| max_outlen < 30 || max_datalen < 1)
		return 0;
	if (memcmp(&msg->address[2], msg->data, 5) == 0) {
		do {
			resp_len = gasmeter_read_di(&msg->data[10], &msg->data[5], di, resp, sizeof(resp));
			tt = time(NULL);
			if (resp_len > 0)
				break;
		}while(0);
		///} while (retry_cnt++ >= g_retry_times); // commented by wd @ 20170613
		if (resp_len > 0 && ptl_cjt188_data_format(&di_data, di, tt, resp, resp_len)) {
			bcd_be_stoc(ptr, 0);
			ptr += 2;
			memcpy(ptr, msg->data, 17);
			ptr += 17;
			meterdata_len = ptl_gasup_pack_meterdata(msg, ptr, max_outlen - (ptr - outdata), &di_data);
			if (meterdata_len > 0) {
				ptr += meterdata_len;
				datalen[0] = ptr - outdata;
				return 1;
			}
			else {
				respone_nak = TRUE;
			}
		}
		else {
			respone_nak = TRUE;
		}
	}
	if (respone_nak) {
		datalen[0] = ptl_gasup_pack_meterdata_nak(msg, outdata, max_outlen);
		if (datalen[0] > 0)
			return 1;
		else
			return 0;
	}
	else
		return 0;
}

UINT32 ptl_gasup_fn_2043(const PTL_GASUP_MSG *msg, INT8 *outdata, INT32 max_outlen, INT32 *datalen, INT32 max_datalen)
{
	INT8 *ptr = outdata, *lastframe_ptr = NULL, *pstart, *pframe_cnt[PTL_GASUP_MAX_PACK_CNT];
	INT8 *detail_num[PTL_GASUP_MAX_PACK_CNT];
	int i;
	UINT16 frame_cnt = 0;
	int blockidx, block_cnt, next_blockidx, mtidx;
	GASMETER_CJT188_901F di_data;
	BYTE addbuf[MAX_HEXDATA_BUF];

	if (msg == NULL || msg->data == NULL || msg->datalen < 17 || outdata == NULL
			|| max_outlen < 24 || max_datalen < 1)
		return 0;
	if (memcmp(&msg->address[2], msg->data, 5) == 0) {
		mtidx = fgasmeter_getidx_by_gasmeter(&msg->data[10]);
		if (mtidx < 0) {
			PRINTF("%s [%s] is not exist in database\n", __FUNCTION__,
					PRINT_ADDRESS(addbuf, &msg->data[10], 7));
			datalen[0] = ptl_gasup_pack_meterdata_nak(msg, ptr, max_outlen);
			if (datalen[0] > 0)
				return 1;
			else
				return 0;
		}
		next_blockidx = 0;
		for (i = 0; i < PTL_GASUP_MAX_PACK_CNT; i++) {
			for (blockidx = next_blockidx; blockidx < MAX_GASMETER_MON_CNT; blockidx++) {
				if (fmon_get_data(blockidx, mtidx, 0x901F, NULL))
					break;
			}
			if (blockidx >= MAX_GASMETER_MON_CNT)
				break;
			pstart = ptr;
			bcd_be_stoc(ptr, 0);
			ptr += 2;
			memcpy(ptr, msg->data, 17);
			ptr += 17;
			lastframe_ptr = ptr;
			*ptr++ = 0;
			stoc(ptr, i + 1);
			ptr += 2;
			///pframe_cnt[i] = ptr;
			detail_num[i] = ptr;
			stoc(ptr, 0);
			ptr += 2;
			block_cnt = 0;
			for (blockidx = next_blockidx; blockidx < MAX_GASMETER_MON_CNT; blockidx++) {
				if (!fmon_get_data(blockidx, mtidx, 0x901F, &di_data))
					continue;
				ptr += ptl_gasup_pack_meterdata(msg, ptr, max_outlen, &di_data);
				block_cnt++;
				if (block_cnt > MAX_BLOCKDATA_CNT_IN_PACKET)
					break;
			}
			next_blockidx = (blockidx + 1);
			frame_cnt++;
			datalen[i] = ptr - pstart;
			stoc(detail_num[i], block_cnt);
		}
		if (frame_cnt > 0) {
			for (i = 0; i < frame_cnt; i++) {
				//stoc(pframe_cnt[i], frame_cnt);
			}
			if (lastframe_ptr) {
				*lastframe_ptr = 1;
			}
			return frame_cnt;
		}
		else {
			datalen[0] = ptl_gasup_pack_meterdata_nak(msg, ptr, max_outlen);
			if (datalen[0] > 0)
				return 1;
			else
				return 0;
		}
	}
	else {
		datalen[0] = ptl_gasup_pack_meterdata_nak(msg, ptr, max_outlen);
		if (datalen[0] > 0)
			return 1;
		else
			return 0;
	}
	return 0;
}

UINT32 ptl_gasup_fn_2044(const PTL_GASUP_MSG *msg, INT8 *outdata, INT32 max_outlen, INT32 *datalen, INT32 max_datalen)
{
	INT8 *ptr = outdata, *lastframe_ptr = NULL, *pstart, *pframe_cnt[PTL_GASUP_MAX_PACK_CNT];
	INT8 *detail_num[PTL_GASUP_MAX_PACK_CNT];
	int i;
	UINT16 frame_cnt = 0;
	int blockidx, block_cnt, next_blockidx, mtidx;
	GASMETER_CJT188_901F di_data;
	BYTE addbuf[MAX_HEXDATA_BUF];

	if (msg == NULL || msg->data == NULL || msg->datalen < 17 || outdata == NULL
			|| max_outlen < 24 || max_datalen < 1)
		return 0;
	if (memcmp(&msg->address[2], msg->data, 5) == 0) {
		mtidx = fgasmeter_getidx_by_gasmeter(&msg->data[10]);
		if (mtidx < 0) {
			PRINTF("%s [%s] is not exist in database\n", __FUNCTION__,
					PRINT_ADDRESS(addbuf, &msg->data[10], 7));
			datalen[0] = ptl_gasup_pack_meterdata_nak(msg, ptr, max_outlen);
			if (datalen[0] > 0)
				return 1;
			else
				return 0;
		}
		next_blockidx = 0;
		for (i = 0; i < PTL_GASUP_MAX_PACK_CNT; i++) {
			for (blockidx = next_blockidx; blockidx < MAX_GASMETER_DAY_CNT; blockidx++) {
				if (fday_get_data(blockidx, mtidx, 0x901F, NULL))
					break;
			}
			if (blockidx >= MAX_GASMETER_DAY_CNT)
				break;
			pstart = ptr;
			bcd_be_stoc(ptr, 0); // xiang ying ma
			ptr += 2;
			memcpy(ptr, msg->data, 17); //  5 + 5 + 7
			ptr += 17;
			lastframe_ptr = ptr; // 01
			*ptr++ = 0;
			stoc(ptr, i + 1); // frame index
			ptr += 2;
			//pframe_cnt[i] = ptr; // number of meter
			detail_num[i] = ptr;
			stoc(ptr, 0);
			ptr += 2;
			block_cnt = 0;
			for (blockidx = next_blockidx; blockidx < MAX_GASMETER_DAY_CNT; blockidx++) {
				if (!fday_get_data(blockidx, mtidx, 0x901F, &di_data))
					continue;
				ptr += ptl_gasup_pack_meterdata(msg, ptr, max_outlen, &di_data);
				block_cnt++;
				if (block_cnt > MAX_BLOCKDATA_CNT_IN_PACKET)
					break;
			}
			next_blockidx = (blockidx + 1);
			frame_cnt++;
			datalen[i] = ptr - pstart;
			stoc(detail_num[i], block_cnt); // add by wd
		}
		if (frame_cnt > 0) {
			for (i = 0; i < frame_cnt; i++) {
				//stoc(pframe_cnt[i], frame_cnt);
				//stoc(pframe_cnt[i], block_cnt); /// frame_cnt -> block_cnt
			}
			if (lastframe_ptr) {
				*lastframe_ptr = 1;
			}
			return frame_cnt;
		}
		else {
			datalen[0] = ptl_gasup_pack_meterdata_nak(msg, ptr, max_outlen);
			if (datalen[0] > 0)
				return 1;
			else
				return 0;
		}
	}
	else {
		datalen[0] = ptl_gasup_pack_meterdata_nak(msg, ptr, max_outlen);
		if (datalen[0] > 0)
			return 1;
		else
			return 0;
	}
	return 0;
}

BOOL gasmeter_set_valve(const BYTE *address, const BYTE *collector, BOOL on);
UINT32 ptl_gasup_fn_2051_2502(const PTL_GASUP_MSG *msg, INT8 *outdata,
		INT32 max_outlen, INT32 *datalen, INT32 max_datalen, BOOL on)
{
	INT8 *ptr = outdata;

	if (msg == NULL || msg->data == NULL || msg->datalen < 17 || outdata == NULL
			|| max_outlen < 19 || max_datalen < 1)
		return 0;
	if (memcmp(&msg->address[2], msg->data, 5) == 0) {
		if (gasmeter_set_valve(&msg->data[10], &msg->data[5], on)) {
			bcd_be_stoc(ptr, 0);
		}
		else {
			bcd_be_stoc(ptr, 2501);
		}
	}
	else {
		bcd_be_stoc(ptr,2501);
	}
	ptr += 2;
	memcpy(ptr, msg->data, 17);
	ptr += 17;
	datalen[0] = ptr - outdata;
	return 1;
}

UINT32 ptl_gasup_fn_2051(const PTL_GASUP_MSG *msg, INT8 *outdata, INT32 max_outlen, INT32 *datalen, INT32 max_datalen)
{
	return ptl_gasup_fn_2051_2502(msg, outdata, max_outlen, datalen, max_datalen, TRUE); // TRUE -> SWITCH ON VALVE
}

UINT32 ptl_gasup_fn_2052(const PTL_GASUP_MSG *msg, INT8 *outdata, INT32 max_outlen, INT32 *datalen, INT32 max_datalen)
{
	return ptl_gasup_fn_2051_2502(msg, outdata, max_outlen, datalen, max_datalen, FALSE); // FALSE -> SWITCH OFF VALVE
}

UINT32 ptl_gasup_fn_2053(const PTL_GASUP_MSG *msg, INT8 *outdata, INT32 max_outlen, INT32 *datalen, INT32 max_datalen)
{
	INT8 *ptr = outdata;
	UINT8 resp[64];
	int resp_len;
	VALVE_STATUS valve_status = VALVE_STATUS_NONE;

	if (msg == NULL || msg->data == NULL || msg->datalen < 17 || outdata == NULL
			|| max_outlen < 20 || max_datalen < 1)
		return 0;
	if (memcmp(&msg->address[2], msg->data, 5) == 0) {
		bcd_be_stoc(ptr, 0);
		resp_len = gasmeter_read_di(&msg->data[10], &msg->data[5], 0x901F, resp, sizeof(resp));
		if (resp_len >= 22) {
			//PRINTB("resp", resp, sizeof(resp));
			//PRINTB("resp[21]", &resp[17], 1);
			//switch ((resp[21] >> 6) & 0x03){ // 17
			switch ((resp[20]) & 0x01) {
			case 0:
				valve_status = 1;
				//valve_status = VALVE_STATUS_ON;
				break;
			case 1:
				valve_status = 0;
				//valve_status = VALVE_STATUS_OFF;
				break;
			case 3:
				valve_status = VALVE_STATUS_ABORT;
				break;
			default:
				break;
			}
		}
		else {
			valve_status = VALVE_STATUS_NONE;
		}
	}
	else {
		bcd_be_stoc(ptr,2501);
	}
	ptr += 2;
	memcpy(ptr, msg->data, 17);
	ptr += 17;
	*ptr++ = valve_status;
	datalen[0] = ptr - outdata;
	return 1;
}

UINT32 ptl_gasup_fn_2061(const PTL_GASUP_MSG *msg, INT8 *outdata, INT32 max_outlen, INT32 *datalen, INT32 max_datalen)
{
	// Nothing to do
	return 0;
}

UINT32 ptl_gasup_fn_2062(const PTL_GASUP_MSG *msg, INT8 *outdata, INT32 max_outlen, INT32 *datalen, INT32 max_datalen)
{
	// Nothing to do
	return 0;
}

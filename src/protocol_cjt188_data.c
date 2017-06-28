/*
 * protocol_cjt188_data.c
 *
 *  Created on: 2015-8-30
 *      Author: Johnnyzhang
 */

#include "protocol_cjt188_data.h"

void ptl_cjt188_data_print(WORD di, void *p) {
#define PRINT_HEX(x, a, b) hex_to_str(x, sizeof(x), a, b, TRUE)
	GASMETER_CJT188_901F *data_901f;
	char s[4][MAX_HEXDATA_BUF];

	if (p == NULL)
		return;
	switch (di) {
	case 0x901F:
		data_901f = (GASMETER_CJT188_901F *) p;
		PRINTF("%s DI: %04X, SER: %d, \n\tFLUX: %s, \n\tBALANCE_FLUX: %s, \n\t"
				"CLOCK:%s, \n\tST: %s\n", __FUNCTION__, data_901f->di_data.di,
				data_901f->di_data.ser,
				PRINT_HEX(s[0], data_901f->di_data.flux, 5), /// 累计流量 == 底码
				PRINT_HEX(s[1], data_901f->di_data.balance_flux, 5), /// 结算日累计流量
				PRINT_HEX(s[2], data_901f->di_data.clock, 7), /// 实时时间
				PRINT_HEX(s[3], data_901f->di_data.st, 2)); /// 状态
		break;
	default:
		break;
	}
}

void ptl_cjt188_data_init(WORD di, void *p)
{
	GASMETER_CJT188_901F *data_901f;

	if (p == NULL)
		return;
	switch (di) {
	case 0x901F: /// case 901F
		data_901f = (GASMETER_CJT188_901F *) p;
		data_901f->status = GASMETER_READ_STATUS_UNREAD;
		break;
	default:
		break;
	}
}

BOOL ptl_cjt188_data_format(void *p, WORD di, long tt, const BYTE *buf,
		int buflen)
{
	GASMETER_CJT188_901F *data_901f;

	if (p == NULL || buf == NULL || buflen < 0)
		return FALSE;
	switch (di) {
	// TODO: the protocol analysis
	case 0x901F:
		if (buflen < 22)
			return FALSE;
		data_901f = (GASMETER_CJT188_901F *) p;
		data_901f->read_tt = tt;
		data_901f->status = GASMETER_READ_STATUS_NORMAL;
		data_901f->di_data.di = di;
		data_901f->di_data.ser = buf[2];
		memcpy(data_901f->di_data.flux, &buf[3], 5);
		memcpy(data_901f->di_data.balance_flux, &buf[8], 5);
		memcpy(data_901f->di_data.clock, &buf[13], 7);
		memcpy(data_901f->di_data.st, &buf[20], 2);
		return TRUE;

	default:
		PRINTF("%s DI: %04X is not support\n", __FUNCTION__, di);
		return FALSE;
	}
}

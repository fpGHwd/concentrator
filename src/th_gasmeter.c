/*
 * th_gasmeter.c
 *
 *  Created on: 2015-8-11
 *      Author: Johnnyzhang
 */

#include "threads.h"
#include "main.h"
#include "devices.h"
#include "serial.h"
#include "common.h"
#include "f_param.h"
#include "f_gasmeter.h"
#include "app_event.h"
#include "protocol_cjt188.h"
#include "protocol_cjt188_data.h"
#include "yl800.h"
#include "f_current.h"
#include "f_day.h"
#include "f_month.h"

#define MAX_READ_DI_CNT 100

#define EVENT_RDMETER 0x00000001

static app_event_t event_rdmeter;
static int gasmeter_fd = -1;
static sem_t sem_serial;

static WORD read_dis[] = {
		0x901F,
};

static struct tm last_read_tm;

void start_read_gasmeter(void) {
	struct tm cur_tm;

	sys_time(&cur_tm);
	app_event_send(&event_rdmeter, EVENT_RDMETER);
	last_read_tm = cur_tm;
}

void *th_gasmeter_event(void *arg) {
	BYTE read_freq[2];
	static BOOL is_first = TRUE;
	struct tm cur_tm;

	print_thread_info();
	sys_time(&cur_tm);
	last_read_tm = cur_tm;
	while (!g_terminated) {
		sleep(100); /// add by nayowang
		notify_watchdog();
		if (fgasmeter_is_empty()) {
			msleep(10);
			continue;
		}
		if (is_first) {
			app_event_send(&event_rdmeter, EVENT_RDMETER);
			sys_time(&last_read_tm);
			is_first = FALSE;
		} else {
			sys_time(&cur_tm);
			fparam_get_value(FPARAMID_READMETER_FREQ, read_freq,
					sizeof(read_freq));
			switch (read_freq[1]) {
			case 0x11: /// every hour
				if ((cur_tm.tm_year != last_read_tm.tm_year
						|| cur_tm.tm_mon != last_read_tm.tm_mon
						|| cur_tm.tm_mday != last_read_tm.tm_mday
						|| cur_tm.tm_hour != last_read_tm.tm_hour)
						&& (cur_tm.tm_min >= 5 && cur_tm.tm_min < 55)) {
					app_event_send(&event_rdmeter, EVENT_RDMETER);
					last_read_tm = cur_tm;
				}
				break;
			case 0x21: /// every day
				if ((cur_tm.tm_year != last_read_tm.tm_year
						|| cur_tm.tm_mon != last_read_tm.tm_mon
						|| cur_tm.tm_mday != last_read_tm.tm_mday)
						&& (cur_tm.tm_min >= 5 && cur_tm.tm_min < 55)) {
					app_event_send(&event_rdmeter, EVENT_RDMETER);
					last_read_tm = cur_tm;
				}
				break;
			case 0x31: /// every month
				if ((cur_tm.tm_year != last_read_tm.tm_year
						|| cur_tm.tm_mon != last_read_tm.tm_mon)
						&& (cur_tm.tm_min >= 5 && cur_tm.tm_min < 55)) {
					app_event_send(&event_rdmeter, EVENT_RDMETER);
					last_read_tm = cur_tm;
				}
				break;
			default:
				break;
			}
		}
	}
	app_event_send(&event_rdmeter, EVENT_RDMETER);
	return NULL;
}

static int gasmeter_read_serial(void *buf, int len, int timeout) {
	return read_serial(gasmeter_fd, buf, len, timeout);
}


// # every thread need a watchdog

int gasmeter_read_di(const BYTE *address, const BYTE *collector, WORD di,
		BYTE *buf, int max_len) /// read meter
{

	BYTE cjt188_reqbuf[512];
	BYTE yl800_reqbuf[512];
	BYTE yl800_respbuf[512], *cjt188_data;
	int cjt188_len, yl800_len = 0, tmplen;
	int fd = gasmeter_fd;
	int timeout = 15 * 1000;
	PTL_CJT188_MSG cjt188_msg;
	BYTE yl800_address[4];
	BYTE repeater[3] = { 0x01, 0x02, 0x03 }, *p_repeater = repeater;
	UINT8 ctrl = CJT188_CTR_READ_DATA;
	BYTE SER;
	/*BYTE debug_yl800_respbuf[] = {
	 0x55, 0x01, 0x02, 0x03, 0x00, 0x05, 0x68, 0x30, 0x05, 0x00, 0x00, 0x07, 0x15, 0x05, 0x23, 0x81,
	 0x16, 0x1F,	0x90, 0x00, 0x56, 0x34, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x44,
	 0x00, 0x02, 0x01, 0x15, 0x20, 0x00, 0x00, 0x44,	0x16,
	 };*/

	if (fd < 0) {
		PRINTF("%s: FILE DESCRIPTOR error\n", __FUNCTION__);
		return -1;
	}

	cjt188_len = plt_cjt188_pack_read(cjt188_reqbuf, sizeof(cjt188_reqbuf),
			address, ctrl, di);

	if (cjt188_len <= 0) {
		PRINTF("%s: 188 PROTOCOL PACK LENGTH error\n", __FUNCTION__);
		return -1;
	}

	if (!fgasmeter_get_repeater(address, repeater) && debug_ctrl.repeater_enable) {
		p_repeater = repeater;
	} else {
		if(debug_ctrl.repeater_enable)
			PRINTF("%s: null repeater\n", __FUNCTION__);
		else
			PRINTF("%s: debug.repeater_enable = false\n", __FUNCTION__);
		p_repeater = NULL;
	}

	yl800_address[0] = 0x00;
	yl800_address[1] = address[4];
	yl800_address[2] = address[5];
	yl800_address[3] = address[6];
	yl800_len = yl800_pack(yl800_reqbuf, sizeof(yl800_reqbuf), p_repeater,
			yl800_address, cjt188_reqbuf, cjt188_len);

	if (yl800_len <= 0) {
		PRINTF("%s: YL800 PACK LENGTH error\n", __FUNCTION__);
		return -1;
	}

	sem_wait(&sem_serial);
	while (read_serial(fd, yl800_respbuf, sizeof(yl800_respbuf), 100) > 0)
		; /// read when NOTHING in buff
	tmplen = write_serial(fd, yl800_reqbuf, yl800_len, 500);
	PRINTB("To GAS METER: ", yl800_reqbuf, yl800_len);

	SER = plt_cjt188_get_ser();
	plt_cjt188_inc_ser();

	if (yl800_len != tmplen) {
		sem_post(&sem_serial);
		PRINTF("%s Read %04X send fail\n", __FUNCTION__, di);
		return -1;
	}

	yl800_len = yl800_read_packet(yl800_respbuf, sizeof(yl800_respbuf), timeout,
			gasmeter_read_serial); //int (*read_fn)(void *buf, int len, int timeout);

	sem_post(&sem_serial);

	if (yl800_len <= 0) {
		PRINTF("%s Read %04X receive fail\n", __FUNCTION__, di);
		return -1;
	}

	PRINTB("From GAS METER: ", yl800_respbuf, yl800_len);

	if (!yl800_unpack(yl800_respbuf, yl800_len, NULL, yl800_address,
			&cjt188_data, &cjt188_len)) {
		return -1;
	}

	if (!plt_cjt188_check_packet(&cjt188_msg, cjt188_data, cjt188_len, address,
			ctrl, di, SER)) { /// check packet
		PRINTF("%s: CHECH PACKET FAILED\n", __FUNCTION__);
		return -1;
	}

	if (max_len < cjt188_msg.datalen) {
		PRINTF("%s: CHECH PACKET LENGTH FAILED\n", __FUNCTION__);
		return -1;
	}

	memcpy(buf, cjt188_msg.data, cjt188_msg.datalen);
	return (cjt188_msg.datalen);
}

BOOL gasmeter_write_di(const BYTE *address, const BYTE *collector, WORD di,
		BYTE *buf, int buflen) {
	BYTE cjt188_reqbuf[512];
	BYTE yl800_reqbuf[512];
	BYTE yl800_respbuf[512], *cjt188_data;
	int cjt188_len, yl800_len = 0, tmplen;
	int fd = gasmeter_fd;
	int timeout = 15 * 1000;
	PTL_CJT188_MSG cjt188_msg;
	BYTE yl800_address[4];
	BYTE repeater[3], *p_repeater = repeater;
	UINT8 ctrl = CJT188_CTR_WRITE_DATA;
	BYTE SER;
	//---------------------------------------------------------------------------------------------------------
	if (fd < 0){
		PRINTF("fd is invalid\n");
		return FALSE;
	}

	cjt188_len = plt_cjt188_pack_write(cjt188_reqbuf, sizeof(cjt188_reqbuf),
			address, ctrl, di, buf, buflen);

	if (cjt188_len <= 0)
		return FALSE;

	if (fgasmeter_get_repeater(address, repeater)) {
		p_repeater = repeater;
	} else {
		p_repeater = NULL;
	}

	//memcpy(yl800_address, &address[5], 2);
	yl800_address[0] = 0x00;
	yl800_address[1] = address[4];
	yl800_address[2] = address[5];
	yl800_address[3] = address[6];

	yl800_len = yl800_pack(yl800_reqbuf, sizeof(yl800_reqbuf), p_repeater,
			yl800_address, cjt188_reqbuf, cjt188_len);

	if (yl800_len <= 0) {
		return ( FALSE);
	}

	sem_wait(&sem_serial);

	while (read_serial(fd, yl800_respbuf, sizeof(yl800_respbuf), 100) > 0) // READ TO CLEAR BUFF
		;
	tmplen = write_serial(fd, yl800_reqbuf, yl800_len, 500);
	PRINTB("To GAS METER: ", yl800_reqbuf, yl800_len);
	SER = plt_cjt188_get_ser();
	plt_cjt188_inc_ser();

	if (yl800_len != tmplen) {
		sem_post(&sem_serial);
		return ( FALSE);
	}

	yl800_len = yl800_read_packet(yl800_respbuf, sizeof(yl800_respbuf), timeout,
			gasmeter_read_serial);

	sem_post(&sem_serial);

	if (yl800_len <= 0) {

		return ( FALSE);
	}

	PRINTB("From GAS METER: ", yl800_respbuf, yl800_len);

	if (!yl800_unpack(yl800_respbuf, yl800_len, NULL, yl800_address,
			&cjt188_data, &cjt188_len)) {

		return ( FALSE);
	}

	if (!plt_cjt188_check_packet(&cjt188_msg, cjt188_data, cjt188_len, address,
			ctrl, di, SER)) {

		return ( FALSE);
	}

	return ( TRUE);
}

BOOL gasmeter_set_valve(const BYTE *address, const BYTE *collector, BOOL on) {
	BYTE cjt188_reqbuf[512];
	BYTE yl800_reqbuf[512];
	BYTE yl800_respbuf[512], *cjt188_data;
	int cjt188_len, yl800_len = 0, tmplen;
	int fd = gasmeter_fd;
	int timeout = 15 * 1000;
	PTL_CJT188_MSG cjt188_msg;

	BYTE repeater[3], *p_repeater = repeater;
	BYTE valvabuf;
	WORD di = 0xA017;
	UINT8 ctrl = CJT188_CTR_WRITE_DATA;
	BYTE SER;

	BYTE yl800_address[4];
//-------------------------------------------------------------------------------------------------------
	if (fd < 0){
		fprintf(stderr,"FATAL ERROR: ivalid\n");
		exit(-1);
	}


	valvabuf = (on ? 0x55 : 0x99);

	cjt188_len = plt_cjt188_pack_write(cjt188_reqbuf, sizeof(cjt188_reqbuf),
			address, ctrl, di, &valvabuf, 1);

	if (cjt188_len <= 0) {
		return FALSE;
	}

	if (fgasmeter_get_repeater(address, repeater)) {
		p_repeater = repeater;
		PRINTF(
				"%s: Repeater[0] = 0x%x, Repeater[1] = 0x%x, Repeater[2] = 0x%x \n",
				__FUNCTION__, repeater[0], repeater[1], repeater[2]);
	} else {
		PRINTF("%s: NO REPEATER connected to this gasmeter address\n",
				__FUNCTION__);
		p_repeater = NULL;
	}

	//memcpy(yl800_address, &address[5], 2);	//RF Node  Address
	yl800_address[0] = 0x00;
	yl800_address[1] = address[4];
	yl800_address[2] = address[5];
	yl800_address[3] = address[6];

	/// PRINTB
	PRINTF(
			"%s yl800_address[0] = 0x%x, yl800_address[1] = 0x%x, yl800_address[2] = 0x%x, yl800_address[3] = 0x%x \n",
			__FUNCTION__, yl800_address[0], yl800_address[1], yl800_address[2],
			yl800_address[3]);

	yl800_len = yl800_pack(yl800_reqbuf, sizeof(yl800_reqbuf), p_repeater,
			yl800_address, cjt188_reqbuf, cjt188_len);

	if (yl800_len <= 0) {
		return FALSE;
	}

	sem_wait(&sem_serial);

	while (read_serial(fd, yl800_respbuf, sizeof(yl800_respbuf), 100) > 0)
		; // clear data

	tmplen = write_serial(fd, yl800_reqbuf, yl800_len, 500);

	PRINTB("To GAS METER: ", yl800_reqbuf, yl800_len);

	SER = plt_cjt188_get_ser();

	plt_cjt188_inc_ser();

	if (yl800_len != tmplen) {

		sem_post(&sem_serial);

		return FALSE;
	}

	yl800_len = yl800_read_packet(yl800_respbuf, sizeof(yl800_respbuf), timeout,
			gasmeter_read_serial);

	sem_post(&sem_serial);

	if (yl800_len <= 0)
		return FALSE;

	PRINTB("From GAS METER: ", yl800_respbuf, yl800_len);

	if (!yl800_unpack(yl800_respbuf, yl800_len, NULL, yl800_address,
			&cjt188_data, &cjt188_len))
		return FALSE;

	if (!plt_cjt188_check_packet(&cjt188_msg, cjt188_data, cjt188_len, address,
			ctrl, di, SER))
		return FALSE;

	if (cjt188_msg.datalen == (3 + 2))
		return TRUE;
	else
		return FALSE;
}

BOOL gasmeter_save_currentdata(const BYTE *address, const BYTE *collector,
		WORD di, long tt, BYTE *buf, int buflen)  /// save current data
{
	int mtidx;
	GASMETER_CJT188_901F di_data;
	BOOL b_success;
	char addbuf[MAX_HEXDATA_BUF], colbuf[MAX_HEXDATA_BUF];

	mtidx = fgasmeter_getidx_by_gasmeter(address);
	if (mtidx < 0)
		return FALSE;
	if (!ptl_cjt188_data_format(&di_data, di, tt, buf, buflen))
		return FALSE;
	b_success = fcurrent_set_data(mtidx, di, &di_data);

	if (b_success) {
		PRINTF("%s Save CURRENT data OK, ADDR: [%s], COL: [%s]\n", __FUNCTION__,
				PRINT_ADDRESS(addbuf, address, 7),
				PRINT_ADDRESS(colbuf, collector, 5));
	}
	return b_success;
}

BOOL gasmeter_save_daydata(const BYTE *address, const BYTE *collector, WORD di,
		long tt, BYTE *buf, int buflen) {
	int mtidx, dayidx;
	GASMETER_CJT188_901F di_data;
	BOOL b_success;
	char addbuf[MAX_HEXDATA_BUF], colbuf[MAX_HEXDATA_BUF];

	mtidx = fgasmeter_getidx_by_gasmeter(address);

	if (mtidx < 0) {
		return FALSE;
	}

	dayidx = fday_get_datablock_index();

	if (dayidx < 0) {
		dayidx = fday_create_dayblock();
	}

	if (dayidx < 0) {
		return FALSE;
	}

	if (fday_get_data(dayidx, mtidx, di, &di_data)) {
		return TRUE;
	}

	if (!ptl_cjt188_data_format(&di_data, di, tt, buf, buflen)) {
		return FALSE;
	}
	b_success = fday_set_data(dayidx, mtidx, di, &di_data);

	if (b_success) {
		PRINTF("%s Save DAY data OK, ADDR: [%s], COL: [%s]\n", __FUNCTION__,
				PRINT_ADDRESS(addbuf, address, 7),
				PRINT_ADDRESS(colbuf, collector, 5));
	}
	return b_success;
}

BOOL gasmeter_save_monthdata(const BYTE *address, const BYTE *collector,
		WORD di, long tt,
		BYTE *buf, int buflen) {
	int mtidx, monidx;
	GASMETER_CJT188_901F di_data;
	BOOL b_success;
	char addbuf[MAX_HEXDATA_BUF], colbuf[MAX_HEXDATA_BUF];

	mtidx = fgasmeter_getidx_by_gasmeter(address);
	if (mtidx < 0)
		return FALSE;
	monidx = fmon_get_datablock_index();
	if (monidx < 0) {
		monidx = fmon_create_monblock();
	}
	if (monidx < 0)
		return FALSE;
	if (fmon_get_data(monidx, mtidx, di, &di_data))
		return TRUE;
	if (!ptl_cjt188_data_format(&di_data, di, tt, buf, buflen))
		return FALSE;
	b_success = fmon_set_data(monidx, mtidx, di, &di_data);

	if (b_success) {
		PRINTF("%s Save MONTH data OK, ADDR: [%s], COL: [%s]\n", __FUNCTION__,
				PRINT_ADDRESS(addbuf, address, 7),
				PRINT_ADDRESS(colbuf, collector, 5));
	}
	return b_success;
}

static void gasmeter_save_data(const BYTE *address, const BYTE *collector,
		WORD di, long tt, BYTE *buf, int buflen) /// it's new
{
	gasmeter_save_currentdata(address, collector, di, tt, buf, buflen);
	gasmeter_save_daydata(address, collector, di, tt, buf, buflen);
	gasmeter_save_monthdata(address, collector, di, tt, buf, buflen);
}

void *th_gasmeter(void *arg) {
	UINT32 ev;
	int i, j, k;
	BYTE address[7], collector[5];
	BYTE resp_buf[512];
	int resp_len;
	long read_tt;
	char addbuf[MAX_HEXDATA_BUF];
	char colbuf[MAX_HEXDATA_BUF];

	print_thread_info();
	sem_init(&sem_serial, 0, 1);
	gasmeter_fd = open_serial(rdmeter_device, B9600, 8, 0);
	while (!g_terminated) {
		notify_watchdog();
		app_event_wait(&event_rdmeter, 1, EVENT_RDMETER, &ev); /// event_rdmeter
		if (ev & EVENT_RDMETER) {
			for (i = 0; !g_terminated && i < MAX_GASMETER_NUMBER; i++) {
				if (!fgasmeter_getgasmeter(i, address, collector))
					continue; /// not set correct 
				for (j = 0; !g_terminated && j < ARRAY_SIZE(read_dis); j++) {
					for (k = 0; !g_terminated && k < g_retry_times; k++) {
						PRINTF(
								"Read DI: %04X, index: %d, ADDR: [%s], COL: [%s]\n",
								read_dis[j], i,
								PRINT_ADDRESS(addbuf, (const BYTE *)address, 7),
								PRINT_ADDRESS(colbuf, (const BYTE *)collector, 5));
						time(&read_tt);
						resp_len = gasmeter_read_di(address, collector,
								read_dis[j], resp_buf, sizeof(resp_buf));
						if (resp_len <= 0) {
							PRINTF("%s: Receive no data till timeout\n", __FUNCTION__);
							fcurrent_set_status(
									fgasmeter_getidx_by_gasmeter(address),
									read_dis[j], GASMETER_READ_STATUS_ABORT);
							continue;
						}else{
							/* save data */
							gasmeter_save_data(address, collector, read_dis[j],
									read_tt, resp_buf, resp_len);
							break;
						}
					}
				}
			}
		}
	}
	if (gasmeter_fd >= 0) {
		close_serial(gasmeter_fd);
	}
	sem_destroy(&sem_serial);
	return NULL;
}

void save_data(const BYTE *address, const BYTE *collector, WORD di, long tt,
		BYTE *buf, int buflen) {
	gasmeter_save_data(address, collector, di, tt, buf, buflen);
}

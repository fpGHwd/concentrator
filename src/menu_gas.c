/*
 * menu_gas.c
 *
 *  Created on: 2016.09
 *      Author: Nayo Wang
 */


//lcd_update_info(c_login_ok_str)

#include "menu_gas.h"
#include "main.h"
#include "common.h"
#include "lcd.h"
#include "language.h"
#include "f_param.h"
#include "gprscdma.h"
#include <stdbool.h>
#include "f_gasmeter.h"

#include "f_day.h"
#include "th_gasmeter.h"
#include "f_month.h"
#include "f_current.h"
#include "f_gasmeter_alarm.h"
#include "th_gasmeter.h"

#include <unistd.h>

static const char *fmt_num = "0123456789";

#define METER_ID_LENGTH 14
#define COLLECTOR_ID_LENGTH 10
#define REPEATER_ID_LENGTH 4


#define METER_ID_LENGTH 14
#define DEFALUT_METER {'2','3','0','5','1','5','0','7','0','0','0','0','0','1'}
#define ERROR_DELAY_TIME_MSECONDS 1000
#define NORMAL_TIP_DELAY_TIME_MSECONDS 10


static void next_char(const char *buf, char *ch, int direct)
{
	const char *ptr;
	int len, pos;

	len = strlen(buf);
	if ((ptr = memchr(buf, *ch, len)) != NULL) {
		pos = ptr - buf;
		if (direct == 0) {
			if (pos == len - 1)
				pos = 0;
			else
				pos++;
		} else {
			if (pos == 0)
				pos = len - 1;
			else
				pos--;
		}
		*ch = buf[pos];
	}
}

static int input_string(int row, const char *name, const char *str, char *buf,
		int len)
{
	unsigned char key;
	int name_len, pos, ret;

	ret = 0;
	name_len = strlen(name);
	lcd_show_string(row, 1, name_len, name);
	pos = len - 1;
	lcd_show_arrow(1, 1, 1, 1);
	while (ret == 0) {
		lcd_show_string(row, name_len + 1, len, buf);
		lcd_show_cursor(row, name_len + 1 + pos, 0);
		key = getch_timeout();
		switch (key) {
		case KEY_UP:
			next_char(str, buf + pos, 0);
			break;
		case KEY_DOWN:
			next_char(str, buf + pos, 1);
			break;
		case KEY_LEFT:
			if (pos > 0)
				pos--;
			else
				pos = len - 1;
			break;
		case KEY_RIGHT:
			if (pos < len - 1)
				pos++;
			else
				pos = 0;
			break;
		case KEY_ENTER:
			ret = 1;
			break;
		case KEY_ESC:
			ret = -1;
			break;
		case KEY_NONE:
			lcd_mode_set(0);
			ret = -1;
			break;
		default:
			break;
		}
	}
	lcd_show_arrow(0, 0, 0, 0);
	return (ret < 0) ? 0 : 1;
}

static void menu_ongoing(BYTE flag, void *para, const char *info) {
	MENU menu;

	init_menu(&menu);
	menu.line_num = 1;
	menu.str[0] = c_menu_ongoing_str;
	process_menu(&menu, info);
}

static void error_tip(const char *tip_string, int msec){
	//lcd_update_info(c_login_ok_str);
	lcd_update_info(tip_string);
	//lcd_show_string(MAX_SCREEN_ROW, 1, 20, "                    ");
	//lcd_show_string(MAX_SCREEN_ROW, 1, strlen(tip_string), (const void*)tip_string);
	wait_delay(msec);
	return;
}

static void meter_data_summery(BYTE flag, void *para, const char *info)
{

	struct tm tm;
	int metersum, dayidx, monthidx, current_row;
	char buff[512];
	unsigned char key;

	do{
		current_row = 1;
		dayidx = -1;
		monthidx = -1;
		lcd_clean_workspace();

		//error_tip(please_select_date, NORMAL_TIP_DELAY_TIME_MSECONDS);
		if(!input_calendar(&tm))
			return;

		metersum = valid_meter_sum();
		if(metersum < 0){
			error_tip(no_meter, ERROR_DELAY_TIME_MSECONDS);
		}
		// meter sum
		if(sprintf(buff, "%s%d", meter_sum, metersum) < 0){
			error_tip(inner_error, ERROR_DELAY_TIME_MSECONDS);
			return;
		}
		lcd_show_string(++current_row, 1, strlen(buff),buff);
		// day valid
		dayidx = get_dayindex_by_date(tm, TRUE);
		if(sprintf(buff, "%s%d",c_day_success,
				(dayidx < 0)?0:fday_block_success_sum(dayidx)) < 0){
			error_tip(inner_error, ERROR_DELAY_TIME_MSECONDS);
			return;
		}
		lcd_show_string(++current_row, 1, strlen(buff),buff);
		// month valid
		monthidx = get_dayindex_by_date(tm, FALSE);
		if(sprintf(buff, "%s%d",c_month_success,
				(monthidx < 0)?0:fmon_block_success_sum(monthidx)) < 0){
			error_tip(inner_error, ERROR_DELAY_TIME_MSECONDS);
			return;
		}
		lcd_show_string(++current_row, 1, strlen(buff),buff);

		key = getch_timeout();
	}while(key != KEY_NONE && key != KEY_ESC);

	return;

}

static void gasmeter_alarm_details(BYTE flag, void *para, const char *info) {
	menu_ongoing(flag, NULL, NULL);
}

static void concentrator_alarm_details(BYTE flag, void *para, const char *info) {
	menu_ongoing(flag, NULL, NULL);
}

static void alarm_events(BYTE flag, void *para, const char *info) {
	ITEMS_MENU items_menu;
	int idx = 0;

	init_menu(&items_menu.menu);
	items_menu.cur_line = 1;

	items_menu.menu.str[idx] = c_gasmeter_alarm_str;
	items_menu.func[idx++] = gasmeter_alarm_details;
	items_menu.menu.str[idx] = c_concentrator_alarm_str;
	items_menu.func[idx++] = concentrator_alarm_details;
	items_menu.menu.line_num = idx;
	process_items(&items_menu, info, FALSE);
}

float get_month_flux(WORD year, BYTE month, BYTE *meter_id) {
	float start_flux, end_flux;
	struct tm tm;
	time_t rawtime;
	signed char monidx, mtidx;
	GASMETER_CJT188_901F di_data;

	time(&rawtime);
	localtime_r(&rawtime, &tm);

	mtidx = fgasmeter_getidx_by_gasmeter(meter_id);
	if (mtidx < 0) {
		return -1;
	} else { // continue;
	} //get_meter_id

	if (year == tm.tm_year + 1900 && month == tm.tm_mon + 1) {
		monidx = fmon_get_datablock_index();
		///printf("monidx = %d---1\n", monidx);
		if (monidx < 0)
			return -1;
		if (!fcurrent_get_data(mtidx, 0x901F, &di_data))
			return -1; // get current flux
	} else {
		tm.tm_mon += 1;
		mktime(&tm);
		monidx = fmon_get_datablock_index_by_time(tm.tm_year + 1900,
				tm.tm_mon + 1);
		///printf("monidx = %d---2\n", monidx);
		if (monidx < 0)
			return -1;
		if (!fmon_get_data(monidx, mtidx, 0x901F, &di_data))
			return -1; // get next month start flux
	}
	/*
	 if(query_time == now_time ){
	 end_flux = current_month_flux;
	 }else{
	 end_flux = next_month_flux;
	 }
	 */
	end_flux = reverse_byte_array2bcd(di_data.di_data.flux,
			sizeof(di_data.di_data.flux)) * 0.1; // end_flux
	///printf("end_flux = %5.1f\n", end_flux);

	if (!fmon_get_data(monidx, mtidx, 0x901F, &di_data))
		return -1; // get_start_flux
	start_flux = reverse_byte_array2bcd(di_data.di_data.flux,
			sizeof(di_data.di_data.flux)) * 0.1;
	///printf("start_flux = %5.1f\n", end_flux);
	// start_flux;

	return end_flux - start_flux;
}


static void add_a_meter_implementation(BYTE flag, void *para, const char *info)
{
	int current_row, index_of_meter = 0;
	BYTE meter_id_byte[7];
	char meter_id[14] = {'2','3','0','5','1','7','0','6','0','0','0','0','0','0'};
	BYTE collector_byte[7];
	char collector[14] = {'0','0','0','0','0','0','0','0','0','0','0','0','0','0'};

	memset(collector_byte, 0, sizeof(collector_byte));
	unsigned char key = KEY_NONE;

	do{
		current_row = 1;
		lcd_clean_workspace();
		if(input_string(++current_row,c_current_reading_meter_str,
				fmt_num,meter_id, sizeof(meter_id)) <= 0){
				//error_tip(invalid_meter, ERROR_DELAY_TIME_MSECONDS);
				break;
		}
		/*// collector
		if(input_string(++current_row,c_current_collector_str, fmt_num,collector,sizeof(collector)) <= 0){
			break;
		}*/

		hexstr_to_str(meter_id_byte, meter_id, 14);
		index_of_meter = fgasmeter_getidx_by_gasmeter(meter_id_byte);
		//hexstr_to_str(collector_byte, collector, 10);
		if(index_of_meter >= 0 && index_of_meter < MAX_GASMETER_NUMBER){
			error_tip(meter_has_existed, ERROR_DELAY_TIME_MSECONDS);
			error_tip(ensure_to_delete_meter,NORMAL_TIP_DELAY_TIME_MSECONDS);
			key = getch_timeout();
			if(key == KEY_ENTER){
				//fgasmeter_getgasmeter(index_of_meter, meter_id_byte, collector);
				if(fgasmeter_delgasmeter(meter_id_byte, collector_byte)){
					error_tip(delete_success,ERROR_DELAY_TIME_MSECONDS);
				}else{
					error_tip("failed to delete",ERROR_DELAY_TIME_MSECONDS);
				}
			}
			error_tip(add_a_meter,NORMAL_TIP_DELAY_TIME_MSECONDS);
			continue;
		}else{// not exist and add // fixme: added and exited try to delete
			// not exist and add
		}

		if(fgasmeter_addgasmeter(meter_id_byte, collector_byte)){
			error_tip(add_meter_success, ERROR_DELAY_TIME_MSECONDS);
			error_tip(add_a_meter,NORMAL_TIP_DELAY_TIME_MSECONDS);
			continue;
		}else{
			error_tip(inner_error, ERROR_DELAY_TIME_MSECONDS);
			return;
		}
	}while(1);

	return;


	/*
	index = -1;
	index = fgasmeter_getidx_by_gasmeter(meter_address);
	if (index >= 0 && index < MAX_GASMETER_NUMBER)
		meter_in_the_database_flag = true;
	else
		meter_in_the_database_flag = false;
	if (meter_in_the_database_flag) {
		printf("meters already in the gasmeter database, and continue;\n");
		continue;
	} else {
		if (fgasmeter_addgasmeter(meter_address, collector))
			printf("success adding a gasmeter: %s\n", a_meter_id);
		else
			printf(
					"fail adding a gasmeter: %s, which is already in the meters database\n",
					a_meter_id);
	}
	*/
}

static void history_data_query(BYTE flag, void *para, const char *info) {

	struct tm tm;
	int current_row, day_idx, month_idx,meter_idx;
	char meter_id[METER_ID_LENGTH] = DEFALUT_METER;
	BYTE meter_id_byte[METER_ID_LENGTH/2];
	GASMETER_CJT188_901F read_data;
	bool blean;
	char buff[MAX_SCREEN_COL*2 + 1];
	unsigned char key;

	do{
		// input meter
		current_row = 1;
		lcd_clean_workspace();
		error_tip(please_input_meter_id, NORMAL_TIP_DELAY_TIME_MSECONDS);
		if(input_string(++current_row,c_current_reading_meter_str,
				fmt_num,meter_id, sizeof(meter_id)) <= 0){
			//error_tip(invalid_meter, ERROR_DELAY_TIME_MSECONDS);
			return;
		}
		hexstr_to_str(meter_id_byte, meter_id, 14);
		meter_idx = fgasmeter_getidx_by_gasmeter(meter_id_byte);
		if(meter_idx < 0){
			error_tip(invalid_meter, ERROR_DELAY_TIME_MSECONDS);
			continue; // TO WHILE so useless and is equal to return
		}
		// select date
		if(!input_calendar(&tm)){
			//lcd_clean_workspace();
			return;
		}

		error_tip(menu_name1_3, NORMAL_TIP_DELAY_TIME_MSECONDS);

		sprintf(buff, "%s%04d-%02d-%02d",
				date_string,tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday);
		lcd_show_string(++current_row, 1, strlen(buff), buff);
		// day_idx
		day_idx = fday_get_datablock_index_by_time(tm.tm_year+1900,
				tm.tm_mon+1, tm.tm_mday);
		/*
		if(day_idx < 0){
			error_tip(no_date_index, ERROR_DELAY_TIME_MSECONDS);
			continue;
		}*/
		// display day
		blean = fday_get_data(day_idx, meter_idx, 0x901F, &read_data);
		if(!blean)
			sprintf(buff, "%s%s",day_data,no_data);
		else
			sprintf(buff, "%s%.1f", day_data,
					reverse_byte_array2bcd(read_data.di_data.flux, 5) * 0.1);
		lcd_show_string(++current_row, 1, strlen(buff), buff);

		month_idx = fmon_get_datablock_index_by_time(tm.tm_year+1900,
				tm.tm_mon+1);
		blean = fmon_get_data(month_idx, meter_idx, 0x901F, &read_data);
		if(!blean)
			sprintf(buff, "%s%s",month_data,no_data);
		else
			sprintf(buff, "%s%.1f", month_data,
					reverse_byte_array2bcd(read_data.di_data.flux, 5) * 0.1);
		lcd_show_string(++current_row, 1, strlen(buff), buff);

		blean = fcurrent_get_data(meter_idx, 0x901F, &read_data);
		if(!blean)
			sprintf(buff, "%s%s",current_data,no_data);
		else
			sprintf(buff, "%s%.1f", current_data,
					reverse_byte_array2bcd(read_data.di_data.flux, 5) * 0.1);
		lcd_show_string(++current_row, 1, strlen(buff), buff);

		key = getch_timeout();

	}while(key != KEY_NONE && key != KEY_ESC);

	return;
}

int gasmeter_read_di(const BYTE *address, const BYTE *collector, WORD di,
		BYTE *buf, int max_len);
static void realtime_read_meter(BYTE flag, void *para, const char *info)
{
	unsigned char key;
	BYTE meter_id_byte[METER_ID_LENGTH/2];
	BYTE resp_buf[512];
	int resp_len;
	int index_of_meter;
	char time_s[30] = {0};

	int current_row;
	unsigned int value;

	char meter_id[METER_ID_LENGTH] = DEFALUT_METER;
	char buff[MAX_SCREEN_COL*2 + 1];

	do{
		current_row = 1;
		lcd_clean_workspace();
		error_tip(please_input_meter_id, NORMAL_TIP_DELAY_TIME_MSECONDS);
		if(input_string(++current_row,c_current_reading_meter_str,
				fmt_num,meter_id, sizeof(meter_id)) <= 0){
			//error_tip(invalid_meter, ERROR_DELAY_TIME_MSECONDS);
			return;
		}
		hexstr_to_str(meter_id_byte, meter_id, 14);
		index_of_meter = fgasmeter_getidx_by_gasmeter(meter_id_byte);
		if(index_of_meter < 0){
			error_tip(invalid_meter, ERROR_DELAY_TIME_MSECONDS);
			continue;
		}
		error_tip(c_reading_meter_str, NORMAL_TIP_DELAY_TIME_MSECONDS);
		resp_len = gasmeter_read_di(meter_id_byte, NULL, 0x901F, resp_buf,
					sizeof(resp_buf));
		if(resp_len <= 0){
			error_tip(read_no_data, ERROR_DELAY_TIME_MSECONDS);
			continue;
		}
		error_tip(read_meter_ok, NORMAL_TIP_DELAY_TIME_MSECONDS);
		// display value and valve state
		value = reverse_byte_array2bcd(resp_buf + 3, 5);
		if(sprintf(buff, "%s%.1f", c_current_flux_display_value_str,
				value * 0.1) < 0){
			error_tip(inner_error, ERROR_DELAY_TIME_MSECONDS);
			return;
		}
		lcd_show_string(++current_row, 1, strlen(buff),buff);
		// bill day value
		value = reverse_byte_array2bcd(resp_buf + 8, 5);
		if(sprintf(buff, "%s%.1f", c_balance_flux_amount_str,value * 0.1) < 0){
			error_tip(inner_error, ERROR_DELAY_TIME_MSECONDS);
			return;
		}
		lcd_show_string(++current_row, 1, strlen(buff),buff);

		// time
		buff[0] = 0;
		strcat(buff,time_string);
		strcat(buff,hex_to_str(time_s, sizeof(time_s) - 1,resp_buf + 18, 2, FALSE));
		strcat(buff,hex_to_str(time_s, sizeof(time_s) - 1,resp_buf + 13, 5, TRUE));
		//PRINTB("resp_buf:", resp_buf, resp_len);
		//PRINTB("resp_buf:", resp_buf, sizeof(resp_buf));
		lcd_show_string(++current_row, 1, strlen(buff),buff);

		// value state
		if(sprintf(buff, "%s%s", c_valve_status_str,
				((resp_buf[20] >> 6) & 0x03)? ((((resp_buf[20] >> 6) & 0x03) == 0x01)?
				c_valve_status_closed_str: valve_abnormal):c_valve_status_open_str) < 0){
			error_tip(inner_error, ERROR_DELAY_TIME_MSECONDS);
			return;
		}
		lcd_show_string(++current_row, 1, strlen(buff),buff); // valve state

		key = getch_timeout();
	}while(key != KEY_NONE && key != KEY_ESC);

	return;
}

void *assembly_read_meter(void *argu){
	int meters_amount, valid_num, failed_num, idx;
	int resp_len; /// merely is a space
	BYTE address[7], collector[5], repeater[2];
	BYTE resp_buf[512];
	char buf[21], meter_address_str[15], collector_str[11], repeater_str[5];
	long read_tt;
	int current_row;
	BYTE key_value = KEY_ENTER;

	memset(address, 0, sizeof(address));
	memset(collector, 0, sizeof(collector));
	memset(repeater, 0, sizeof(repeater));

	lcd_clean_workspace();
	meters_amount = get_valid_meter_amount_in_database();
	valid_num = 0;
	failed_num = 0;
	for (idx = 0; (idx < MAX_GASMETER_NUMBER) && (key_value != KEY_ESC) &&(!g_terminated); idx++) {
		current_row = 1;

		if (fgasmeter_getgasmeter(idx, address, collector)) {

			snprintf(buf, 21, "%s%d\x2F%d", c_reading_meter_str, idx + 1,
					meters_amount);
			//lcd_show_string(2, 1, strlen(c_allspace_str), c_allspace_str);
			lcd_show_string(++current_row, 1, strlen(buf), buf);

			snprintf(buf, 21, "%s%d\x2F%d", c_reading_success_str, valid_num,
					meters_amount);
			lcd_show_string(++current_row, 1, strlen(buf), buf);

			snprintf(buf, 21, "%s%d\x2F%d", c_reading_failure_str, failed_num,
					meters_amount);
			lcd_show_string(++current_row, 1, strlen(buf), buf);

			//get the collector index
			//get the collector address
			snprintf(buf, 21, "%s%s", c_current_collector_str,
					hex_to_str(collector_str, sizeof(collector_str), collector,
							5, FALSE));
			lcd_show_string(++current_row, 1, strlen(buf), buf);

			if (fgasmeter_get_repeater(address, repeater)) { // get the repeater address
				; // get repeater address and continue
			} else {
				memset(repeater, 0, sizeof(repeater));
			}

			snprintf(buf, 21, "%s%s", c_current_repeater_str,
					hex_to_str(repeater_str, sizeof(repeater_str), repeater, 2,
							FALSE));
			lcd_show_string(++current_row, 1, strlen(buf), buf);

			snprintf(buf, 21, "%s%s", c_current_reading_meter_str,
					hex_to_str(meter_address_str, sizeof(meter_address_str),
							address, 7, FALSE));
			lcd_show_string(++current_row, 1, strlen(buf), buf);

			time(&read_tt);
			resp_len = gasmeter_read_di(address, collector, 0x901F, resp_buf,
					sizeof(resp_buf));
			if (resp_len <= 0) {
				failed_num++;
				fcurrent_set_status(fgasmeter_getidx_by_gasmeter(address),
						0x901F, GASMETER_READ_STATUS_ABORT);
				//continue; /// failure and set meter status
			} else {
				valid_num++;
				save_data(address, collector, 0x901F, read_tt, resp_buf,
						resp_len);
			}

			current_row = 2;
			snprintf(buf, 21, "%s%d/%d", c_reading_success_str, valid_num,
					meters_amount);
			lcd_show_string(++current_row, 1, strlen(buf), buf);

			snprintf(buf, 21, "%s%d/%d", c_reading_failure_str, failed_num,
					meters_amount);
			lcd_show_string(++current_row, 1, strlen(buf), buf);
			//continue;
		} /*else {
			continue;
		}*/
		key_value = getch_wait_ms(100);
	}

	if( idx == MAX_GASMETER_NUMBER -1)
		error_tip(c_complete_str,ERROR_DELAY_TIME_MSECONDS);
	else
		error_tip(c_abort_str,ERROR_DELAY_TIME_MSECONDS);

	return NULL;
}

static void read_meter_assembly_function(BYTE flag, void *para,
		const char *info) {

	pthread_t pid[2];
	pid[0] = pthread_self();
	//BYTE key_value = KEY_ENTER;

	if(pthread_create(&pid[1], NULL, assembly_read_meter, NULL/*&key_value*/) == 0){
		error_tip("Reading...",NORMAL_TIP_DELAY_TIME_MSECONDS);
	}else{
		if(errno == EAGAIN)
			error_tip("pthread:EAGAIN",ERROR_DELAY_TIME_MSECONDS);
		else if(errno == EINVAL)
			error_tip("pthread:EINVAL",ERROR_DELAY_TIME_MSECONDS);
		else if(errno == EPERM)
			error_tip("pthread:EPERM",ERROR_DELAY_TIME_MSECONDS);
		else
			error_tip("pthread:UNKNOWN",ERROR_DELAY_TIME_MSECONDS);
		return;
	}

	/*
	while((key_value != KEY_ESC) && !g_terminated){
		key_value = getch_wait_ms(100);
	}*/

	pthread_join(pid[1], NULL);
	// pthread_cancel(pid[1]);
}

static void read_meter_assembly(BYTE flag, void *para, const char *info) {
	ITEMS_MENU items_menu;
	int idx = 0;

	if (verify_password() != 0)
		return;

	init_menu(&items_menu.menu);
	items_menu.cur_line = 1;

	items_menu.menu.str[idx] = c_ensure_str;
	items_menu.func[idx++] = read_meter_assembly_function;
	items_menu.menu.line_num = idx;
	process_items(&items_menu, info, FALSE);

}


static void repeater_involved(BYTE flag, void *para, const char *info)
{

	return;

	/*
	char repeater_str[5], address_str[15], buff[MAX_SCREEN_COL + 1];
	BYTE repeater[2], address[7];
	int i;

	fgasmeter_getidx_by_gasmeter(address);

	lcd_clean_workspace();
	memcpy(address_str, "23051606000007", 15);
	if (!input_string(2, c_history_meter_id_str, fmt_num, address_str, 14))
		return;

	for (i = 0; i < sizeof(address); i++) {
		address[i] = bin_to_bcd(
				(address_str[2 * i] - '0') * 10 + address_str[2 * i + 1] - '0');
	}
	snprintf(buff, sizeof(buff), "%s%s", c_history_meter_id_str, address_str);
	lcd_show_string(2, 1, strlen(buff), buff);

	if (!fgasmeter_get_repeater(address, repeater)) {
		memcpy(repeater_str, "0000", 5);
	} else {
		hex_to_str(repeater_str, sizeof(repeater_str), repeater, 2, FALSE);
	}

	snprintf(buff, sizeof(buff), "%s", c_repeater_str);

	if (!input_string(3, c_repeater_str, fmt_num, repeater_str, 4)) {
		return;
	}
	snprintf(buff, sizeof(buff), "%s%s", c_repeater_str, repeater_str);
	lcd_show_string(3, 1, strlen(buff), buff);

	for (i = 0; i < sizeof(repeater); i++) {
		repeater[i] = bin_to_bcd(
				(repeater_str[2 * i] - '0') * 10 + repeater_str[2 * i + 1]
						- '0');
	}

	if (!fgasmeter_set_repeater(address, repeater))
			{
		lcd_show_string(4, 1, strlen("FAILED!"), "FAILED!");
	} else {
		lcd_show_string(4, 1, strlen("SUCCESS!"), "SUCCESS!");
	}
	sleep(1);
	return;
	*/
}


void import_meters_into_the_fgasmeter_structure(const char *filename) {

	FILE *fp;
	char buff[50];
	char a_meter_id[15];
	BYTE meter_address[7], collector[5];
	int i, index;
	int meter_in_the_database_flag = false;

	memset(collector, 0, sizeof(collector));

	memset(buff, 0, sizeof(buff));
	if ((fp = fopen(filename, "r")) == NULL) {
		///perror(filename);
		return;///exit(1);
	} else {
		/// success, continue;
	}

	while (fgets(buff, sizeof(buff), fp) != NULL) {
		memcpy(a_meter_id, buff, 15);
		for (i = 0; i < 7; i++)
			meter_address[i] = bin_to_bcd(
					(a_meter_id[2 * i] - '0') * 10 + a_meter_id[2 * i + 1] - '0');
		PRINTB("meter_address: ", meter_address, sizeof(meter_address));

		index = -1;
		index = fgasmeter_getidx_by_gasmeter(meter_address);
		if (index >= 0 && index < MAX_GASMETER_NUMBER)
			meter_in_the_database_flag = true;
		else
			meter_in_the_database_flag = false;
		if (meter_in_the_database_flag) { /* meter address is already in*/
			printf("meters already in the gasmeter database, and continue;\n");
			continue;
		} else {
			if (fgasmeter_addgasmeter(meter_address, collector))
				printf("success adding a gasmeter: %s\n", a_meter_id);
			else
				printf(
						"fail adding a gasmeter: %s, which is already in the meters database\n",
						a_meter_id);
		}
	}
	return;

}

static void import_meters_in_bunch_function(BYTE flag, void *para,
		const char *info) {
	const char *filename = "/media/usb/meters.txt";
	const char suffix[5][5];
	bool file_exist_flag;
	struct stat buf;
	int current_row = 1;

	memset(suffix, 0, sizeof(suffix));

	lcd_clean_workspace();

	file_exist_flag = false;
	if (stat(filename, &buf) == 0)
		file_exist_flag = true;
	else
		file_exist_flag = false;

	if (!file_exist_flag) {
		error_tip(c_meters_file_not_exist_str,ERROR_DELAY_TIME_MSECONDS);
		return;
	}

	error_tip(c_meters_file_importing_str,NORMAL_TIP_DELAY_TIME_MSECONDS);
	/*lcd_show_string(2, 1, strlen(c_meters_file_importing_str),
			c_meters_file_importing_str);*/
	import_meters_into_the_fgasmeter_structure(filename);
	error_tip(c_meters_import_success_str,ERROR_DELAY_TIME_MSECONDS);
	/*
	lcd_show_string(3, 1, strlen(c_meters_import_success_str),
			c_meters_import_success_str);
	sleep(1);
	*/

	return;
}

static void import_meters_in_bunch(BYTE flag, void *para, const char *info)
{

	int idx = 0;
	ITEMS_MENU items_menu;

	if (verify_password() != 0)
		return;

	init_menu(&items_menu.menu);
	items_menu.cur_line = 1;
	items_menu.menu.str[idx] = c_ensure_str;
	items_menu.func[idx++] = import_meters_in_bunch_function;
	items_menu.menu.line_num = idx;
	process_items(&items_menu, info, FALSE);
}

static void meter_data_reset(BYTE flag, void *para, const char *info);
static void query_data(BYTE flag, void *para, const char *info)
{

	ITEMS_MENU items_menu;
	int idx = 0;

	init_menu(&items_menu.menu);
	items_menu.cur_line = 1;

	items_menu.menu.str[idx] = add_a_meter; // fixme: implement the string
	items_menu.func[idx++] = add_a_meter_implementation; // fixme: implementation
	items_menu.menu.str[idx] = menu_name1_3;
	items_menu.func[idx++] = history_data_query;
	items_menu.menu.str[idx] = menu_name1_1;
	items_menu.func[idx++] = meter_data_summery;
	items_menu.menu.str[idx] = menu_name1_2;
	items_menu.func[idx++] = alarm_events;
	items_menu.menu.str[idx] = menu_name1_4;
	items_menu.func[idx++] = realtime_read_meter;
	items_menu.menu.str[idx] = menu_name1_5;
	items_menu.func[idx++] = read_meter_assembly;
	items_menu.menu.str[idx] = menu_name1_7;
	items_menu.func[idx++] = import_meters_in_bunch;
	items_menu.menu.str[idx] = menu_name3_1_2;
	items_menu.func[idx++] = meter_data_reset;

	items_menu.menu.line_num = idx;
	process_items(&items_menu, info, FALSE);
}

static void set_comm_channel(BYTE flag, void *para, const char *info)
{
	menu_ongoing(flag, para, info);
}

static void set_prior_host_ip_and_port(BYTE flag, void *para, const char *info) {
	if (verify_password() != 0)
		return;

	PARAM_SET param_set;
	BYTE host_ip[4], host_port[2];
	char ip_addr[16 + 1], port_buf[5 + 1];
	int idx;
	unsigned short ip_port;
	unsigned long int ip;

	memset(&param_set, 0, sizeof(param_set));
	memset(ip_addr, 0, sizeof(ip_addr));
	memset(port_buf, 0, sizeof(port_buf));

	fparam_get_value(FPARAMID_COMM_HOST_IP_PRI, host_ip, sizeof(host_ip));
	fparam_get_value(FPARAMID_COMM_HOST_PORT_PRI, host_port, sizeof(host_port));

	snprintf(ip_addr, sizeof(ip_addr), "%u.%u.%u.%u", host_ip[0], host_ip[1],
			host_ip[2], host_ip[3]);
	ip_port = (host_port[0] << 8) + host_port[1];
	sprintf(port_buf, "%d", ip_port);

	init_menu(&param_set.menu);
	param_set.menu.line_num = 3;
	param_set.menu.str[0] = c_ip_address_str;
	param_set.menu.str[1] = NULL;
	param_set.menu.str[2] = c_port_str;
	param_set.group_num = 2;
	param_set.group_idx = 0;
	idx = 0;
	init_input_set(&param_set.input[idx], 2, 1, strlen(ip_addr), ip_addr, 15);
	param_set.keyboard_type[idx++] = _num_keyboard;
	init_input_set(&param_set.input[idx], 3, 6, strlen(port_buf), port_buf, 5);
	param_set.keyboard_type[idx++] = _num_keyboard;
	process_param_set(&param_set, info);
	strcpy(ip_addr, param_set.input[0].str);
	ip = inet_addr(ip_addr);
	host_ip[0] = ip;
	host_ip[1] = ip >> 8;
	host_ip[2] = ip >> 16;
	host_ip[3] = ip >> 24;
	strcpy(port_buf, param_set.input[1].str);
	ip_port = atoi(port_buf);
	host_port[0] = ip_port >> 8;
	host_port[1] = ip_port;
	fparam_set_value(FPARAMID_COMM_HOST_IP_PRI, host_ip, sizeof(host_ip));
	fparam_set_value(FPARAMID_COMM_HOST_PORT_PRI, host_port, sizeof(host_port));
}

static void set_apn(BYTE flag, void *para, const char *info) {
	if (verify_password() != 0)
		return;

	PARAM_SET param_set;
	char apn_id[APN_LENGTH], apn_user_id[APN_LENGTH],
			apn_user_password[APN_LENGTH];
	int idx;

	memset(&param_set, 0, sizeof(param_set));
	memset(apn_id, 0, sizeof(apn_id));
	memset(apn_user_id, 0, sizeof(apn_user_id));
	memset(apn_user_password, 0, sizeof(apn_user_password));

	fparam_get_value(FPARAMID_APN_ID, apn_id, sizeof(apn_id));
	fparam_get_value(FPARAMID_APN_USER_ID, apn_user_id, sizeof(apn_user_id));
	fparam_get_value(FPARAMID_APN_USER_PASSWD, apn_user_password,
			sizeof(apn_user_password));

	init_menu(&param_set.menu);
	param_set.menu.line_num = 6;
	param_set.menu.str[0] = c_apn_str;
	param_set.menu.str[1] = NULL;
	param_set.menu.str[2] = c_apn_id_str;
	param_set.menu.str[3] = NULL;
	param_set.menu.str[4] = c_apn_password_str;
	param_set.menu.str[5] = NULL;
	param_set.group_num = 3;
	param_set.group_idx = 0;
	idx = 0;
	init_input_set(&param_set.input[idx], 2, 1, strlen(apn_id), apn_id,
			APN_LENGTH - 1);
	param_set.keyboard_type[idx++] = _upper_letter_keyboard;
	init_input_set(&param_set.input[idx], 4, 1, strlen(apn_user_id),
			apn_user_id, APN_LENGTH - 1);
	param_set.keyboard_type[idx++] = _upper_letter_keyboard;
	init_input_set(&param_set.input[idx], 6, 1, strlen(apn_user_password),
			apn_user_password, APN_LENGTH - 1);
	param_set.keyboard_type[idx++] = _upper_letter_keyboard;

	process_param_set(&param_set, info);

	strcpy(apn_id, param_set.input[0].str);
	strcpy(apn_user_id, param_set.input[1].str);
	strcpy(apn_user_password, param_set.input[2].str);
	fparam_set_value(FPARAMID_APN_ID, apn_id, 32);
	fparam_set_value(FPARAMID_APN_USER_ID, apn_user_id, 32);
	fparam_set_value(FPARAMID_APN_USER_PASSWD, apn_user_password, 32);

}

static void set_heartbeat_to_mainstation(BYTE flag, void *para,
		const char *info) {

	PARAM_SET param_set;
	BYTE heart_beat[2];
	char heartbeat_string[6];
	int idx = 0;

	if (verify_password() != 0)
		return;

	memset(&param_set, 0, sizeof(param_set));
	memset(heart_beat, 0, sizeof(heart_beat));

	fparam_get_value(FPARAMID_HEARTBEAT_CYCLE, heart_beat, sizeof(heart_beat));
	sprintf(heartbeat_string, "%d", (heart_beat[1] << 8) + heart_beat[0]);

	init_menu(&param_set.menu);
	param_set.menu.line_num = 2;
	param_set.menu.str[0] = c_heart_beat_min_str;
	param_set.menu.str[1] = NULL;
	param_set.group_num = 1;
	param_set.group_idx = 0;

	init_input_set(&param_set.input[idx], 2, 1, strlen(heartbeat_string), heartbeat_string, 4);
	param_set.keyboard_type[idx++] = _num_keyboard;
	process_param_set(&param_set, info);

	strcpy(heartbeat_string, param_set.input[0].str);
	stoc(heart_beat, atoi(heartbeat_string));
	if(fparam_set_value(FPARAMID_HEARTBEAT_CYCLE, heart_beat, 2))
		error_tip(set_successfully, ERROR_DELAY_TIME_MSECONDS);
	else
		error_tip(set_unsuccessfully, ERROR_DELAY_TIME_MSECONDS);
}

static void reconstruct_network(BYTE flag, void *para, const char *info)
{
	if (verify_password() != 0)
		return;
	menu_ongoing(flag, para, info);
}

static void param_data_reset(BYTE flag, void *para, const char *info);
static void set_param(BYTE flag, void *para, const char *info)
{
	int idx = 0;
	ITEMS_MENU items_menu;

	init_menu(&items_menu.menu);
	items_menu.cur_line = 1;

	//items_menu.menu.str[idx] = menu_name1_6;
	//items_menu.func[idx++] = repeater_involved; //TODO: repeater function setting
	items_menu.menu.str[idx] = menu_name2_3;
	items_menu.func[idx++] = reconstruct_network; // reconstruct
	items_menu.menu.str[idx] = menu_name2_1_5;
	items_menu.func[idx++] = set_heartbeat_to_mainstation;
	items_menu.menu.str[idx] = menu_name2_1;
	items_menu.func[idx++] = set_prior_host_ip_and_port;
	items_menu.menu.str[idx] = menu_name2_1_1;
	items_menu.func[idx++] = set_comm_channel;
	items_menu.menu.str[idx] = menu_name2_1_4;
	items_menu.func[idx++] = set_apn;
	items_menu.menu.str[idx] = menu_name3_1_3;
	items_menu.func[idx++] = param_data_reset;

	items_menu.menu.line_num = idx;
	process_items(&items_menu, info, FALSE);
}

static void view_version_addr(BYTE flag, void *para, const char *info) {
	MENU menu;
	BYTE addr[7];
	char string[6][MAX_SCREEN_COL + 1];
	int i;
	char addr_str[32];

	init_menu(&menu);
	menu.line_num = 6;
	for (i = 0; i < menu.line_num; i++)
		menu.str[i] = string[i];
	sprintf(string[0], "%s:%s", c_con_model_str, CONCENTRATOR_MODEL);
	sprintf(string[1], "%s:%s", c_con_version_str, APP_VERSION);
	sprintf(string[2], "%s:%s", c_con_compiletime_str,g_release_time);
	sprintf(string[3], "%s:", c_con_address_str);
	fparam_get_value(FPARAMID_CON_ADDRESS, addr, sizeof(addr));
	sprintf(string[4], "  %s",
			hex_to_str(addr_str, sizeof(addr_str), addr, 7, FALSE));
	process_menu(&menu, info);
}

static void query_network_ip(BYTE flag, void *para, const char *info)
{
	MENU menu;
	char string[4][MAX_SCREEN_COL + 1];
	char ipaddr[32], dstaddr[16];
	int i;

	init_menu(&menu);
	for (i = 0; i < sizeof(string) / sizeof(string[0]); i++) {
		menu.str[i] = string[i];
	}
	menu.line_num = 4;
	if (remote_module_get_ip(ipaddr)) {
		ipaddr[31] = 0;
		sprintf(string[0], "GPRS/CDMA IP:");
		sprintf(string[1], "  %s", ipaddr);
	} else {
		sprintf(string[0], "GPRS/CDMA IP:");
		sprintf(string[1], "  %s", "0.0.0.0");
	}
	if (get_network_addr("eth", ipaddr, dstaddr)) {
		sprintf(string[2], "%s:", c_ethernet_ip_str);
		sprintf(string[3], "  %s", ipaddr);
	} else {
		sprintf(string[2], "%s:", c_ethernet_ip_str);
		sprintf(string[3], "  %s", "0.0.0.0");
	}
	process_menu(&menu, info);
}

void modem_gprs_shutdown(void);
static void restart_terminal_function(BYTE flag, void *para, const char *info) {
	int ret, current_row = 1;

	lcd_clean_workspace();
	lcd_show_string(++current_row, 1, strlen(c_restarting_terminal_str),
			c_restarting_terminal_str);
	g_terminated = 1;
	led_fade();
	///modem_gprs_shutdown();
	sleep(5);
	lcd_clean_workspace();
	printf("reboot\n");
	LOG_PRINTF_ALLOCATE(
			"Reboot in file:%s, in func: %s, line: %d, allocation size: %d\n",
			__FILE__, __func__, __LINE__);
	//fflush(stdout); // FIXME: when get SIGTERM should fflush and fsync files
	//fsync();
	ret = system("/sbin/reboot");

	return;
}

static void restart_terminal_in(BYTE flag, void *para, const char *info) {
	if (verify_password() != 0)
		return;

	ITEMS_MENU items_menu;
	int idx = 0;

	init_menu(&items_menu.menu);
	items_menu.cur_line = 1;
	items_menu.menu.str[idx] = c_ensure_str;
	items_menu.func[idx++] = restart_terminal_function;
	items_menu.menu.line_num = idx;
	process_items(&items_menu, info, FALSE);

}

static void reset_data(BYTE flag, void *para, const char *info){
	int current_row = 1;

	lcd_clean_workspace();
	if(reset_fcurrent_data() == 0)
		lcd_show_string(++current_row, 1, strlen(c_reset_data_suc_str), c_reset_data_suc_str);
	else
		error_tip(inner_error, ERROR_DELAY_TIME_MSECONDS);
	if (reset_fday_data() == 0)
		lcd_show_string(++current_row, 1, strlen(c_reset_day_data_suc_str), c_reset_day_data_suc_str);
	else
		error_tip(inner_error, ERROR_DELAY_TIME_MSECONDS);
	if (reset_fmonth_data() == 0)
		lcd_show_string(++current_row, 1, strlen(c_reset_month_data_suc_str), c_reset_month_data_suc_str);
	else
		error_tip(inner_error, ERROR_DELAY_TIME_MSECONDS);
	if (reset_fgasmeteralm_data() == 0)
		lcd_show_string(++current_row, 1, strlen(c_reset_gas_alarm_data_suc_str), c_reset_gas_alarm_data_suc_str);
	else
		error_tip(inner_error, ERROR_DELAY_TIME_MSECONDS);

	error_tip(c_success_reset_str,ERROR_DELAY_TIME_MSECONDS);
}


static void reset_meters(BYTE flag, void *para, const char *info){
	//menu_ongoing(flag, NULL, NULL);
	int current_row = 1;

	lcd_clean_workspace();

	lcd_show_string(++current_row,1,strlen(clearing),clearing);
	if(reset_gasmeter_data() == 0){
		error_tip(clear_successfully,ERROR_DELAY_TIME_MSECONDS);
	}else{
		error_tip(clear_unsuccessfully,ERROR_DELAY_TIME_MSECONDS);
	}
	return;
}

static void meter_data_reset(BYTE flag, void *para, const char *info) {

	if (verify_password() != 0)
		return;

	ITEMS_MENU items_menu;
	int idx = 0;

	init_menu(&items_menu.menu);
	items_menu.cur_line = 1;
	items_menu.menu.str[idx] = c_reset_data_str;
	items_menu.func[idx++] = reset_data;
	items_menu.menu.str[idx] = c_reset_meters_str;
	items_menu.func[idx++] = reset_meters;
	items_menu.menu.line_num = idx;
	process_items(&items_menu, info, FALSE);
}

static void reset_param_data(BYTE flag, void *para, const char *info) {
	lcd_clean_workspace();
	lcd_show_string(2, 1, strlen(c_resetting_str), c_resetting_str);
	if (reset_fparam_data() == 0)
		lcd_show_string(2, 1, strlen(c_success_reset_str), c_success_reset_str);
	sleep(1);
}

static void param_data_reset(BYTE flag, void *para, const char *info) {
	if (verify_password() != 0)
		return;

	int idx = 0;
	ITEMS_MENU items_menu;

	init_menu(&items_menu.menu);
	items_menu.cur_line = 1;
	items_menu.menu.str[idx] = c_ensure_str;
	items_menu.func[idx++] = reset_param_data;
	items_menu.menu.line_num = idx;
	process_items(&items_menu, info, FALSE);

}

#ifdef AM335X
#define BINARY_NAME "/media/usb/concentrator-am335x"
#else defined(IMX28)
#define BINARY_NAME "/media/usb/concentrator-imx28"
#endif

static void usb_update_function(BYTE flag, void *para, const char *info) {
	const char *filename = BINARY_NAME;
	bool file_exist_flag;
	struct stat buf;
	int ret, current_row = 1;

	lcd_clean_workspace();

	file_exist_flag = false;
	if (stat(filename, &buf) == 0) {
		file_exist_flag = true;
	} else {
		file_exist_flag = false;
	}

	if (!file_exist_flag) {
		lcd_show_string(++current_row, 1, strlen(c_no_update_file_str),
				c_no_update_file_str);
		sleep(1);
		return;
	}
	lcd_show_string(++current_row, 1, strlen(c_find_update_file_and_reboot_then_str),
			c_find_update_file_and_reboot_then_str);
#ifdef AM335X
	ret = system("cp /media/usb/concentrator-am335x /opt/concentrator/bin/concentrator-am335x");
	ret = system("sync");
#elif defined IMX28
	ret = system("cp /media/usb/concentrator-imx28 /opt/concentrator/bin/concentrator-imx28");
	ret = system("sync");
#endif

	sleep(2);
	restart_terminal_function( false, NULL, NULL);

	return;

}

static void usb_update(BYTE flag, void *para, const char *info) {
	if (verify_password() != 0)
		return;

	int idx = 0;
	ITEMS_MENU items_menu;

	init_menu(&items_menu.menu);
	items_menu.cur_line = 1;
	items_menu.menu.str[idx] = c_ensure_str;
	items_menu.func[idx++] = usb_update_function;
	items_menu.menu.line_num = idx;
	process_items(&items_menu, info, FALSE);
}

/*
 static void liquid_switch(BYTE flag, void *para, const char *info)
 {
 menu_ongoing(flag, para, info);
 }
 */

static void terminal_id(BYTE flag, void *para, const char *info)
{
	MENU menu;
	BYTE addr[7];
	char string[2][MAX_SCREEN_COL + 1];
	int i;
	char addr_str[32];

	init_menu(&menu);
	menu.line_num = 2;
	for (i = 0; i < menu.line_num; i++)
		menu.str[i] = string[i];
	sprintf(string[0], "%s:", c_con_address_str);
	fparam_get_value(FPARAMID_CON_ADDRESS, addr, sizeof(addr));
	sprintf(string[1], "  %s",
			hex_to_str(addr_str, sizeof(addr_str), addr, 7, FALSE));
	process_menu(&menu, info);
}

static void set_terminal_time(BYTE flag, void *para, const char *info) {

	if (verify_password() != 0)
		return;

	time_t tt;
	int current_row = 1;
	struct tm tm, tm1;
	char buff[MAX_SCREEN_COL * 2 + 1];
	char hour[2], min[2], sec[2];
	struct timeval tv;

	if(!input_calendar(&tm1))
		return;

	lcd_update_info(menu_name3_5);

	sprintf(buff, "%s%04d-%02d-%02d",
				date_string,tm1.tm_year+1900, tm1.tm_mon+1, tm1.tm_mday);
	lcd_show_string(++current_row, 1, strlen(buff), buff);

	time(&tt);
	localtime_r(&tt, &tm);

	sprintf(buff, "%s%02d:%02d:%02d",
			time_string, tm.tm_hour, tm.tm_min, tm.tm_sec);
	lcd_show_string(++current_row, 1, strlen(buff), buff);

	buff[0] = 0;
	strcat(buff, time_string);
	hour[0] = tm.tm_hour/10 + '0';
	hour[1] = tm.tm_hour%10 + '0';
	if(!input_string(current_row, buff, fmt_num, hour, sizeof(hour)))
		return;
	sprintf(buff, "%s%c%c", buff, hour[0], hour[1]);
	strcat(buff, ":");

	min[0] = tm.tm_min/10 + '0';
	min[1] = tm.tm_min%10 + '0';
	if(!input_string(current_row, buff, fmt_num, min, sizeof(min)))
		return;
	sprintf(buff, "%s%c%c", buff, min[0], min[1]);
	strcat(buff, ":");

	sec[0] = tm.tm_sec/10 + '0';
	sec[1] = tm.tm_sec%10 + '0';
	if(!input_string(current_row, buff, fmt_num, sec, sizeof(sec)))
		return;

	tm.tm_isdst = -1;
	tm.tm_year = tm1.tm_year;
	tm.tm_mon = tm1.tm_mon;
	tm.tm_mday = tm1.tm_mday;
	tm.tm_hour = (hour[0] - '0') * 10 + (hour[1] - '0');
	tm.tm_min = (min[0] - '0') * 10 + (min[1] - '0');
	tm.tm_sec = (sec[0] - '0') * 10 + (sec[1] - '0');
	tv.tv_sec = mktime(&tm);
	tv.tv_usec = 0;
	if(settimeofday(&tv, NULL) == 0){ // ptl_gasup_fn_2015();
		set_rtc();
		error_tip(set_successfully, ERROR_DELAY_TIME_MSECONDS);
	}else{
		return;
	}
	return;
}

// fixme: concentrator03 seems not readmeter

static void password_setting(BYTE flag, void *para, const char *info) {

	if (verify_password() != 0)
		return;

	PARAM_SET param_set;
	BYTE password_byte[3];
	char buf_s[7];
	int idx;

	memset(password_byte, 0, sizeof(password_byte));
	memset(&param_set, 0, sizeof(param_set));

	if (!fparam_get_value(FPARAMID_CON_VERIFY_PASSWD, password_byte,
			sizeof(password_byte)))
		return;
	snprintf(buf_s, 7, "%02d%02d%02d", bcd_to_bin(password_byte[0]),
			bcd_to_bin(password_byte[1]), bcd_to_bin(password_byte[2])); // 0x12, 0x34, 0x56 -> "123456"

	init_menu(&param_set.menu);
	param_set.menu.line_num = 2;
	param_set.menu.str[0] = c_password_verify_str;
	param_set.menu.str[1] = NULL;
	param_set.group_num = 1;
	param_set.group_idx = 0;
	idx = 0;
	init_input_set(&param_set.input[idx], 2, 4, strlen(buf_s), buf_s, 6);
	param_set.keyboard_type[idx++] = _num_keyboard;
	process_param_set(&param_set, info);

	strcpy(buf_s, param_set.input[0].str);
	if (strlen(buf_s) < 6)
		return;
	password_byte[0] = bin_to_bcd(atoi(buf_s) / 10000);
	password_byte[1] = bin_to_bcd(atoi(buf_s) % 10000 / 100);
	password_byte[2] = bin_to_bcd(atoi(buf_s) % 100);

	if (!fparam_set_value(FPARAMID_CON_VERIFY_PASSWD, password_byte,
			sizeof(password_byte))) {
		// TIPS
		printf("set password failed\n");
		////return;
	} else {
		printf("set password ok\n");
	}

	return;
}

static void local_module_status(BYTE flag, void *para, const char *info) {
	menu_ongoing(flag, para, info);
}

static void signal_intensity_and_battery_status(BYTE flag, void *para,
		const char *info) {
	menu_ongoing(flag, para, info);
}

static void get_modem_flux_in_sum(BYTE flag, void *para, const char *info) {
	MENU menu;
	char string[3][MAX_SCREEN_COL + 1]; /// MAX_SCREEN_COL + 1
	int i, j = 0;
	//j = get_flux_in_sum();

	init_menu(&menu);
	menu.line_num = 2;
	for (i = 0; i < menu.line_num; i++)
		menu.str[i] = string[i];
	i = 0;
	sprintf(string[i++], "%s", modem_flux_sum_str);
	sprintf(string[i++], (j / (1024 * 1024)) ? "   %8.2f MB" : "   %8.2f KB",
			(j / (1024 * 1024)) ? (j / (1024.0 * 1024)) : (j / 1024.0));
	process_menu(&menu, info);
}

static void terminal_maintenance(BYTE flag, void *para, const char *info) {
	ITEMS_MENU items_menu;
	int idx = 0;

	init_menu(&items_menu.menu);
	items_menu.cur_line = 1;
	//items_menu.menu.str[idx] = menu_name3_1;
	//items_menu.func[idx++] = restart_terminal; // restart terminal
	//items_menu.menu.str[idx] = menu_name3_2;
	//items_menu.func[idx++] = liquid_switch; // crystal switch
	//items_menu.menu.str[idx] = menu_name3_3;
	//items_menu.func[idx++] = terminal_id;
	items_menu.menu.str[idx] = menu_name3_4;
	//items_menu.func[idx++] = view_con_info;
	items_menu.func[idx++] = view_version_addr;
	items_menu.menu.str[idx] = menu_name3_10;
	items_menu.func[idx++] = query_network_ip;
	//items_menu.func[idx++] = view_version_addr;
	items_menu.menu.str[idx] = menu_name3_5;
	items_menu.func[idx++] = set_terminal_time;
	items_menu.menu.str[idx] = menu_name3_6;
	items_menu.func[idx++] = password_setting; // password setting
	//tems_menu.menu.str[idx] = menu_name3_7;
	//items_menu.func[idx++] = channel_setting; // a duplicate
	//items_menu.menu.str[idx] = menu_name3_8;
	//items_menu.func[idx++] = local_module_status;
	//items_menu.menu.str[idx] = menu_name3_9;
	//items_menu.func[idx++] = signal_intensity_and_battery_status;
	items_menu.menu.str[idx] = modem_flux_sum_menu;
	items_menu.func[idx++] = get_modem_flux_in_sum;
	items_menu.menu.str[idx] = menu_name3_1;
	items_menu.func[idx++] = restart_terminal_in;
	items_menu.menu.str[idx] = menu_name3_1_4;
	items_menu.func[idx++] = usb_update;

	items_menu.menu.line_num = idx;
	process_items(&items_menu, info, FALSE);
}

void main_menu_init(ITEMS_MENU *items_menu) {
	int idx = 0;

	init_menu(&items_menu->menu);
	items_menu->cur_line = 1;
	items_menu->menu.str[idx] = menu_name1;
	items_menu->func[idx++] = query_data; // query data
	items_menu->menu.str[idx] = menu_name2;
	items_menu->func[idx++] = set_param; // set parameter
	items_menu->menu.str[idx] = menu_name3;
	items_menu->func[idx++] = terminal_maintenance; // maintenance

	items_menu->menu.line_num = idx;
}

int verify_password(void) {
	int tries_times;
	BYTE password_byte[3];
	char buf[7];
	char buf_save[7];
	int current_row = 1;

	memcpy(buf, "000000", sizeof(buf));
	tries_times = 3;

	while (1) {
		current_row = 1;
		lcd_clean_workspace();
		lcd_show_string(++current_row, 1, strlen(c_please_input_password_str),
				c_please_input_password_str);
		lcd_update_info(c_password_setting_str);
		/*
		lcd_show_string(9, 1, strlen(c_allspace_str), c_allspace_str);
		lcd_show_string(9, 1, strlen(c_password_setting_str),
				c_password_setting_str);
		*/
		if (input_string(++current_row, "   ", fmt_num, buf, strlen(buf))) {
			lcd_show_string(current_row, 4, strlen(buf), buf); // valid continue the verification
		} else {
			return -1; // too long not input
		}

		fparam_get_value(FPARAMID_CON_VERIFY_PASSWD, password_byte,
				sizeof(password_byte));
		snprintf(buf_save, 7, "%02d%02d%02d", bcd_to_bin(password_byte[0]),
				bcd_to_bin(password_byte[1]), bcd_to_bin(password_byte[2]));
		///PRINTF("SECURITY! PASSWORD SAVED IN: %s\n", buf_save);
		if (memcmp(buf, buf_save, 6) == 0)
			return 0; // success in verifying
		tries_times--;
		if (!(tries_times > 0))
			return -1; // tries times is out of use
	}
}

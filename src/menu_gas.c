/*
 * menu_gas.c
 *
 *  Created on: 201609
 *      Author: Nayo Wang
 */

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

static const char *fmt_num = "0123456789";
//static const char *yes_or_no = "YN";

#define METER_ID_LENGTH 14
#define COLLECTOR_ID_LENGTH 10
#define REPEATER_ID_LENGTH 4

static void next_char(const char *buf, char *ch, int direct) /// buf = "YN" ,ch = 'N', direct = 0/1
{
	const char *ptr;
	int len, pos;

	len = strlen(buf); // len = 2;
	if ((ptr = memchr(buf, *ch, len)) != NULL) { // ptr = memchr("YN", 'N', 2)
		pos = ptr - buf; // ptr = &("YN")+1
		if (direct == 0) { // up
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
		int len)  /// 显示列号：3（即开始显示的位置），前面补偿字符串，候选序列，原始内容，内容string长度
{
	unsigned char key;
	int name_len, pos, ret;

	ret = 0;
	name_len = strlen(name); /// string length
	lcd_show_string(row, 1, name_len, name);
	pos = len - 1;
	lcd_show_arrow(1, 1, 1, 1); /// arrow
	while (ret == 0) { /// while
		lcd_show_string(row, name_len + 1, len, buf); /// 00002301000001
		lcd_show_cursor(row, name_len + 1 + pos, 0); /// cursor
		key = getch_timeout();
		switch (key) {
		case KEY_UP:
			next_char(str, buf + pos, 0); /// str本身是有顺序的字符串序列
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
			lcd_mode_set(0); /// for return
			ret = -1;
			break;
		default:
			break;
		}
	}
	lcd_show_arrow(0, 0, 0, 0);
	return (ret < 0) ? 0 : 1; /// return
}

static void menu_ongoing(BYTE flag, void *para, const char *info) {
	MENU menu;

	init_menu(&menu);
	menu.line_num = 1;
	menu.str[0] = c_menu_ongoing_str;
	process_menu(&menu, info);
}

static void read_meter_data(BYTE flag, void *para, const char *info) /// 抄表统计数据
{

	char buf_time[9]; /// sizeof(buf_time) = 9, strlen(buf_time) = 0;
	time_t timep;
	struct tm tm;
	unsigned char key;
	char buf[30];
	int valid_meter = -1, valid_month_data = -1, valid_day_data = -1, day_idx =
			-1, month_idx = -1;

	time(&timep);
	localtime_r(&timep, &tm);
	snprintf(buf_time, sizeof(buf_time), "%04d%02d%02d", tm.tm_year + 1900,
			tm.tm_mon + 1, tm.tm_mday);
	/// %d-%s-%s /// problem, segmentation fault

	while (1) {
		lcd_clean_workspace(); // clean screen
		lcd_show_string(2, 1, strlen(c_date_str), c_date_str); // 日时标
		lcd_show_string(9, 1, strlen(menu_name1_1), menu_name1_1);
		if (!input_string(3, "   ", fmt_num, buf_time, sizeof(buf_time) - 1)) {
			break;
		} else {
			/// valid continue;
		}

		if (if_date_str_is_valid(buf_time)) {
			/// valid continue
		} else {
			lcd_show_string(9, 1, strlen(c_allspace_str), c_allspace_str); // 删除显示
			lcd_show_string(9, 1, strlen(c_time_invalid), c_time_invalid); // 时间不正确
			sleep(1);
			break;
		}

		lcd_clean_workspace();
		valid_meter = valid_meter_sum(); // valid meter / all meter 

		day_idx = fday_get_datablock_index_by_time(atoi(buf_time) / 10000,
				atoi(buf_time) % 10000 / 100, atoi(buf_time) % 100);
		if (day_idx < 0) {
			//valid_day_data = 0;
			// no data tips
			//continue; 
			valid_day_data = 0;
		} else {
			valid_day_data = fday_block_success_sum(day_idx);
		}			// valid_day_data

		month_idx = fmon_get_datablock_index_by_time(atoi(buf_time) / 10000,
				atoi(buf_time) % 10000 / 100);
		if (month_idx < 0) {
			// no data tips
			//continue;
			valid_month_data = 0;
		} else {
			valid_month_data = fmon_block_success_sum(month_idx);
		}

		// display 
		snprintf(buf, sizeof(buf), "%s%d", c_meter_sum, valid_meter);
		lcd_show_string(2, 1, strlen(buf), buf);
		snprintf(buf, sizeof(buf), "%s%d", c_day_success, valid_day_data);
		lcd_show_string(3, 1, strlen(buf), buf);
		snprintf(buf, sizeof(buf), "%s%d", c_month_success, valid_month_data);
		lcd_show_string(4, 1, strlen((const char*) buf), buf);

		key = getch_timeout();
		if (key == KEY_ESC) {
			break;
		} else {
			continue;
		}
	}
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

/*
 static void alarm_concentrator(BYTE flag, void *para, const char *info)
 {
 int record_index;
 char meter_id[15];
 char time_buff[17];
 unsigned char key;
 int row,  record_index_max;
 bool first_branch = true; 
 char buff[16] = {0}; /// 最近第001条记录 6 * 2 + 3 + 1;
 
 record_index_max = 10; /// 获取改最大值 /// 根据文件数据而定

 do{
 if(first_branch){
 record_index = 1;	
 lcd_clean_workspace();
 // 获取并显示第一条数据
 lcd_show_string( 2, 1, strlen(c_alarm_name_str), c_alarm_name_str);
 lcd_show_string( 4, 1, strlen(c_alarm_meter_id_str), c_alarm_meter_id_str);
 lcd_show_string( 6, 1, strlen(c_alarm_occur_time_str), c_alarm_occur_time_str);
 first_branch = false;
 } else if(!first_branch){
 if(key == KEY_UP){
 if(record_index > 1){
 record_index--;

 lcd_clean_workspace();
 lcd_show_string( 2, 1, strlen(c_alarm_name_str), c_alarm_name_str);
 lcd_show_string( 4, 1, strlen(c_alarm_meter_id_str), c_alarm_meter_id_str);
 lcd_show_string( 6, 1, strlen(c_alarm_occur_time_str), c_alarm_occur_time_str);

 // 获取并显示告警信息 record_index
 
 lcd_show_string( 9, 1, strlen(c_allspace_str), c_allspace_str); // 清除内容
 snprintf(buff, sizeof(buff), "%s%d%s", c_alarm_last_record_str1, 
 record_index, c_alarm_last_record_str2); /// 最近第x条记录
 lcd_show_string( 9, 1, strlen(buff), buff); /// 底部信息更新
 } else if(record_index == 1){
 lcd_show_string( 9, 1, strlen(c_allspace_str), c_allspace_str); 
 lcd_show_string( 9, 1, strlen(c_alarm_already_get_the_first_record_str),
 c_alarm_already_get_the_first_record_str); /// 已经是第1条记录
 sleep(1); 
 lcd_show_string( 9, 1, strlen(c_allspace_str), c_allspace_str); // 清除内容
 snprintf(buff, sizeof(buff), "%s%d%s", c_alarm_last_record_str1, 
 record_index, c_alarm_last_record_str2); /// 最近第x条记录
 lcd_show_string( 9, 1, strlen(buff), buff); /// 底部信息更新
 continue;
 }
 } else if(key == KEY_DOWN){
 if(record_index < record_index_max){
 record_index++;

 lcd_clean_workspace();
 lcd_show_string( 2, 1, strlen(c_alarm_name_str), c_alarm_name_str);
 lcd_show_string( 4, 1, strlen(c_alarm_meter_id_str), c_alarm_meter_id_str);
 lcd_show_string( 6, 1, strlen(c_alarm_occur_time_str), c_alarm_occur_time_str);

 //index的具体数据填写
 lcd_show_string( 9, 1, strlen(c_allspace_str), c_allspace_str);
 snprintf(buff, sizeof(buff), "%s%d%s", c_alarm_last_record_str1, 
 record_index, c_alarm_last_record_str2);
 lcd_show_string( 9, 1, strlen(buff), buff); /// 更新底部信息

 } else if( record_index == record_index_max){
 lcd_show_string( 9, 1, strlen(c_alarm_already_get_the_last_record_str), 
 c_alarm_already_get_the_last_record_str); /// 无法用于其他尺寸屏幕重构 /// 已经是最后一条记录
 sleep(1); 
 lcd_show_string( 9, 1, strlen(c_allspace_str), c_allspace_str);
 snprintf(buff, sizeof(buff), "%s%d%s", c_alarm_last_record_str1, 
 record_index, c_alarm_last_record_str2);
 lcd_show_string( 9, 1, strlen(buff), buff); /// 更新底部信息

 }
 } else if(key == KEY_ESC || key == KEY_ENTER){
 break; /// 退出
 } else if(key == KEY_LEFT || key == KEY_RIGHT){
 continue; /// 继续循环
 }
 }
 }while(key = getch_timeout());

 }
 */
/*
 ---------------------
 告警名称：
 停电事件告警
 测量点表号：
 23051606000006
 发生时间：
 2016-10-15 12:21
 ---------------------
 最近第2条记录
 */

float get_month_flux(WORD year, BYTE month, BYTE *meter_id) {
	///float flux = -1;
	float start_flux, end_flux;
	struct tm tm;
	time_t rawtime;
	signed char monidx, mtidx;
	GASMETER_CJT188_901F di_data;

	time(&rawtime);
	localtime_r(&rawtime, &tm); //get_now_time

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

/*
 struct flux_time{
 float flux;
 time_t read_time;
 };

 bool get_month_flux_new(const WORD year, const BYTE month, const BYTE *meter_id, struct flux_time *flux_and_time)
 {
 float flux = -1;
 float start_flux, end_flux;
 struct tm tm;
 time_t rawtime;
 signed char monidx, mtidx;
 GASMETER_CJT188_901F di_data;
 struct flux_time buf[3];

 time(&rawtime);
 localtime_r(&rawtime, &tm);

 mtidx = fgasmeter_getidx_by_gasmeter(meter_id);
 if(mtidx < 0){
 return false;
 }else{// continue;
 }//get_meter_id

 if(year == tm.tm_year + 1900 && month == tm.tm_mon + 1){
 monidx = fmon_get_datablock_index();
 ///printf("monidx = %d---1\n", monidx);
 if(monidx < 0)
 return false;
 if(!fcurrent_get_data(mtidx, 0x901F, &di_data))
 return false; // get current flux
 }else{
 tm.tm_mon += 1;
 mktime(&tm);
 monidx = fmon_get_datablock_index_by_time(tm.tm_year + 1900, tm.tm_mon + 1);
 ///printf("monidx = %d---2\n", monidx);
 if(monidx < 0)
 return false;
 if(!fmon_get_data(monidx, mtidx, 0x901F, &di_data))
 return false; // get next month start flux
 }
 buf[2].flux = reverse_byte_array2bcd(di_data.di_data.flux, sizeof(di_data.di_data.flux)) * 0.1; // end_flux
 buf[2].read_time = di_data.read_tt;
 if(!fmon_get_data(monidx, mtidx, 0x901F, &di_data))
 return false; // get_start_flux
 buf[1].flux = reverse_byte_array2bcd(di_data.di_data.flux, sizeof(di_data.di_data.flux)) * 0.1; // start
 buf[1].read_time = di_data.read_tt;

 memcpy(flux_and_time, &buf, sizeof(struct flux_time));	
 return true;
 }
 */

/*
 static void month_data_query_new(BYTE flag, void *para, const char *info)
 {
 struct flux_time flux_time[3];
 unsigned char year_month[7];
 unsigned char meter_id[15];
 unsigned char key;
 time_t rawtime;
 struct tm tm;
 BYTE meter_id_byte[7];
 unsigned char prompt[30];
 int row;

 time(&rawtime);
 localtime_r(&rawtime, &tm);
 snprintf(year_month, sizeof(year_month), "%04d%02d", tm.tm_year+ 1900, tm.tm_mon + 1);// get now time
 memcpy(meter_id, "23051606000102", 14);

 while(1){
 for( row = 2; row <= 8; row++)
 lcd_show_string( row, 1, strlen(c_allspace_str), c_allspace_str);// clear 
 lcd_show_string( 9, 1, strlen(menu_name1_3_1), menu_name1_3_1);
 snprintf(prompt, sizeof(prompt), "%s%s", c_history_month_date_str, year_month);
 lcd_show_string( 3, 1, strlen(c_history_month_date_str),c_history_month_date_str); /// 月时标
 
 while(1){
 for( row = 2; row <= 8; row++)
 lcd_show_string( row, 1, strlen(c_allspace_str), c_allspace_str);// clear 
 snprintf(prompt, sizeof(prompt), "%s%s", c_history_month_date_str, year_month);
 lcd_show_string( 3, 1, strlen(prompt), prompt); 
 lcd_show_string( 9, 1, strlen(menu_name1_3_1),menu_name1_3_1);
 if(input_string(2, c_history_meter_id_str, fmt_num, meter_id, strlen(meter_id))){
 // success
 if(input_string(3, c_history_month_date_str, fmt_num, year_month, strlen(year_month))){
 //success and go on
 }else{
 // failure
 return;
 }
 }else{
 // failure
 return;
 }

 hexstr_to_str(meter_id_byte, meter_id, 14);
 if(fgasmeter_getidx_by_gasmeter(meter_id_byte) >= 0 && if_year_month_str_is_valid(year_month)){
 break;// make sure meter id and year month usable
 }else{
 if(!(fgasmeter_getidx_by_gasmeter(meter_id_byte) >= 0)){
 lcd_show_string( 9, 1, strlen(c_allspace_str),c_allspace_str);
 lcd_show_string( 9, 1, strlen(c_invalid_meter_id_str),c_invalid_meter_id_str);
 }else{
 lcd_show_string( 9, 1, strlen(c_allspace_str),c_allspace_str);
 lcd_show_string( 9, 1, strlen(c_invalid_year_month_str),c_invalid_year_month_str);
 }
 sleep(1);
 lcd_show_string( 9, 1, strlen(menu_name1_3_1), menu_name1_3_1); 
 continue;
 }
 } // make sure meter id and year month usable

 flux_time[1].flux = -1.0;
 flux_time[2].flux = -1.0;
 flux_time[0].flux = -1.0;

 for( row = 2; row <= 8; row++)
 lcd_show_string(row, 1, strlen(c_allspace_str), c_allspace_str);// clear 

 get_month_flux_new(atoi(year_month)/100, atoi(year_month)%100, meter_id, flux_time);
 
 if(!get_month_flux_new(atoi(year_month)/100, atoi(year_month)%100, meter_id, flux_time)){
 printf("!get_month_flux_new(atoi) +608\n");
 continue; // continue not about data;
 }

 if(flux_time[1].flux < 0){
 snprintf(prompt, sizeof(prompt), "%s%s",c_month_initiate_flux_str,c_no_data_str);
 }else{
 snprintf(prompt, sizeof(prompt), "%s%7.1f", c_month_initiate_flux_str, flux_time[1].flux);
 }
 lcd_show_string(3, 1, strlen(prompt), prompt);

 if(flux_time[2].flux < 0){
 prompt[0] = '\0';
 }else{
 localtime_r(&flux_time[1].read_time, &tm);
 snprintf(prompt, sizeof(prompt), "%04d-%02d-%02d %02d:%02d", 
 tm.tm_year + 1900, tm.tm_mon+1, tm.tm_mday,tm.tm_hour, tm.tm_min);
 }
 lcd_show_string(4, 1, strlen(prompt), prompt);/// 初抄表时间

 if(flux_time[2].flux < 0){
 snprintf(prompt, sizeof(prompt), "%s%s",c_month_end_flux_str,c_no_data_str);
 }else{
 snprintf(prompt, sizeof(prompt), "%s%7.1f", c_month_end_flux_str, flux_time[2].flux);
 }
 lcd_show_string(5, 1, strlen(prompt), prompt);

 if(flux_time[1].flux < 0){
 prompt[0] = '\0';
 }else{
 localtime_r(&flux_time[1].read_time, &tm);
 snprintf(prompt, sizeof(prompt), "%04d-%02d-%02d %02d:%02d", 
 tm.tm_year + 1900, tm.tm_mon+1, tm.tm_mday,tm.tm_hour, tm.tm_min);
 }
 lcd_show_string(6, 1, strlen(prompt), prompt);// 末抄表时间

 if(flux_time[2].flux < 0 || flux_time[1].flux < 0){
 snprintf(prompt, sizeof(prompt), "%s%s", c_month_end_flux_str,c_no_data_str);
 }else{
 snprintf(prompt, sizeof(prompt), "%s%7.1f", c_month_end_flux_str, flux_time[2].flux-flux_time[1].flux);
 }
 lcd_show_string(2, 1, strlen(prompt), prompt);

 key = getch_timeout();
 if(key == KEY_ENTER){
 // down
 }else if(key == KEY_ESC){
 break;
 }else if(key == KEY_UP || key == KEY_DOWN || key == KEY_RIGHT || key == KEY_LEFT){
 continue;
 }
 }
 }
 */

static void month_data_query(BYTE flag, void *para, const char *info) {
	char meter_id[METER_ID_LENGTH + 1];
	BYTE meter_id_byte[METER_ID_LENGTH / 2];
	char year_month[6 + 1];
	unsigned char key;
	int month_index, meter_index;
	struct tm t_m;
	time_t tt;
	///int j;
	char buf[21];
	GASMETER_CJT188_901F di_data[2]; // a di_data[2] buf
	float delta_flux = -1, start_flux = -1, end_flux = -1;
	/// bool current_month;
	int year, month;

	key = 0;

	time(&tt);
	localtime_r(&tt, &t_m);

	snprintf(year_month, METER_ID_LENGTH, "%04d%02d", t_m.tm_year + 1900,
			t_m.tm_mon + 1); // n = sizeof(month);
	memcpy(meter_id, "23051606000007", sizeof(meter_id));

	while (1) {

		lcd_clean_workspace(); /// clear 
		lcd_show_string(9, 1, strlen(c_allspace_str), c_allspace_str);
		lcd_show_string(9, 1, strlen(menu_name1_3_1), menu_name1_3_1);
		lcd_show_string(2, 1, strlen(c_history_meter_id_str),
				c_history_meter_id_str); /// 表号
		lcd_show_string(4, 1, strlen(c_history_month_date_str),
				c_history_month_date_str); /// 月时标 
		lcd_show_string(3, 4, strlen(meter_id), meter_id);
		lcd_show_string(5, 4, strlen(year_month), year_month); /// initiate displaying

		if (input_string(3, "   ", fmt_num, meter_id, strlen(meter_id))) {
			lcd_show_string(3, 4, strlen(meter_id), meter_id); // continue
		} else {
			return; // failure
		} // get meter id string successfully

		if (input_string(5, "   ", fmt_num, year_month,
				strlen((const char*) year_month))) {
			lcd_show_string(5, 4, strlen((const char*) year_month), year_month); // continue
		} else {
			return; // failure
		} // get month string successfully

		if (if_year_month_str_is_valid((const char*) year_month)) {
			// valid month and continue
		} else {
			lcd_show_string(9, 1, strlen(c_allspace_str), c_allspace_str);
			lcd_show_string(9, 1, strlen(c_year_month_invalid_str),
					c_year_month_invalid_str);
			sleep(1);
			continue; // invalid, continue
		} // month is valid

		hexstr_to_str(meter_id_byte, meter_id, 14); /// meter_id_byte
		meter_index = fgasmeter_getidx_by_gasmeter(meter_id_byte);
		if (meter_index >= 0) {
			// valid meter id and continue
		} else {
			lcd_show_string(9, 1, strlen(c_allspace_str), c_allspace_str);
			lcd_show_string(9, 1, strlen(c_not_find_meter_in_concentrator),
					c_not_find_meter_in_concentrator);
			sleep(1);
			continue; // invalid, continue
		} // the meter is in the database

		year = atoi((const char*) year_month) / 100;
		month = atoi((const char*) year_month) % 100;
		///tm_buf.tm_year = atoi(year_month)/100 - 1900;
		///tm_buf.tm_mon = atoi(year_month)%100 - 1;
		///mktime(&tm_buf);

		month_index = -1;
		month_index = fmon_get_datablock_index_by_time(
				atoi((const char*) year_month) / 100,
				atoi((const char*) year_month) % 100);
		if (month_index >= 0) {
			// valid and continue;
		} else {
			lcd_show_string(9, 1, strlen(c_allspace_str), c_allspace_str);
			lcd_show_string(9, 1, strlen(c_no_data_found), c_no_data_found);
			sleep(1);
			continue;
		}		// month is in data -> must have a result

		if (!fmon_get_data(month_index, meter_index, 0x901F, &di_data[0])) {
			// tips: get data error
			continue;// failure
		} else {
			// valid continue;
		} // get month start flux
		start_flux = reverse_byte_array2bcd(di_data[0].di_data.flux,
				sizeof(di_data[0].di_data.flux)) * 0.1; // start flux string
		//snprintf(buf, 21, "    %7.1f m3", start_flux);

		//current_month = false;
		if (year == t_m.tm_year + 1900 && month == t_m.tm_mon + 1) {
			//current_month = true;
			if (!fcurrent_get_data(meter_index, 0x901F, &di_data[1])) {
				// tips: get current data error
				continue;// failure
			} else {
				// logic continue;
			} // get current data as end data
		} else {
			//current_month = false;
			///snprintf(buf[2], 20,"    %7.1f m3", reverse_byte_array2bcd(di_data_2.di_data.flux, sizeof(di_data_2.di_data.flux)) * 0.1 );
			//tm_buf.tm_mon += 1;
			//mktime(&tm_buf);
			if (month == 12) {
				year++;
				month = 1;
			} else {
				month++;
			}

			//printf("t_m.tm_year = %d, t_m.tm_mon = %d\n", tm_buf.tm_year + 1900, tm_buf.tm_mon + 1);
			month_index = fmon_get_datablock_index_by_time(year, month);
			//printf("month_index: %d\n", month_index);
			if (month_index >= 0) {
				// valid and continue
			} else {
				continue; // fail and continue
			}

			if (!fmon_get_data(month_index, meter_index, 0x901F, &di_data[1])) {
				continue;
			} else {
				// valid and logic continue;
			}
		} //get end flux
		end_flux = reverse_byte_array2bcd(di_data[1].di_data.flux,
				sizeof(di_data[1].di_data.flux)) * 0.1; // end flux float
		delta_flux = end_flux - start_flux; // delta flux

		lcd_clean_workspace();
		snprintf(buf, 21, "%s%0.1f m3", c_month_flux_str, delta_flux);
		//lcd_show_string(2, 1, strlen(c_month_flux_str),c_month_flux_str );
		lcd_show_string(2, 1, strlen(buf), buf);
		snprintf(buf, 21, "%s%0.1f m3", c_month_initiate_flux_str, start_flux);
		lcd_show_string(3, 1, strlen(buf), buf);
		tt = di_data[0].read_tt;
		localtime_r(&tt, &t_m);
		snprintf(buf, 21, "%04d/%02d/%02d %02d:%02d", t_m.tm_year + 1900,
				t_m.tm_mon + 1, t_m.tm_mday, t_m.tm_hour, t_m.tm_min); /// 显示时间
		lcd_show_string(4, 3, strlen(buf), buf);
		snprintf(buf, 21, "%s%0.1f m3", c_next_month_initiate_flux_str,
				end_flux);
		lcd_show_string(5, 1, strlen(buf), buf);
		tt = di_data[1].read_tt;
		localtime_r(&tt, &t_m);
		snprintf(buf, 21, "%04d/%02d/%02d %02d:%02d", t_m.tm_year + 1900,
				t_m.tm_mon + 1, t_m.tm_mday, t_m.tm_hour, t_m.tm_min); /// 显示时间
		lcd_show_string(6, 3, strlen(buf), buf);

		key = getch_timeout();
		if (key == KEY_ESC) {
			break;
		} else if (key == KEY_ENTER) {
			continue;
		} else if (key == KEY_UP || key == KEY_DOWN || key == KEY_RIGHT
				|| key == KEY_LEFT) {
			continue;
		}
	}
}

// 日数据查询
static void daily_data_query(BYTE flag, void *para, const char *info) {

	char meter_id[7 * 2 + 1]; // 23051606000006
	char year_month_day[8 + 1]; // 20161018
	unsigned char meter_id_byte[7]; /* BYTE */
	time_t tt;
	struct tm t_m, tm_buf;
	unsigned char key;
	int day_idx, mtidx;
	GASMETER_CJT188_901F di_data[2];
	float start_flux = -1, end_flux = -1, delta_flux = -1;
	char buf[20 + 1];
	///int j;
	///bool current_day;
	int year, month, day;
	key = 0;
	/*
	 buf = (char**)malloc(3 * sizeof(char *));
	 for(j = 0; j < 3; j++){
	 buf[j] = (char *)malloc(20 * sizeof(char));
	 } // create buf[3][20]
	 */

	memcpy(meter_id, "23051606000007", 15); // get meter id str
	time(&tt);
	localtime_r(&tt, &t_m);
	snprintf(year_month_day, sizeof(year_month_day), "%04d%02d%02d",
			t_m.tm_year + 1900, t_m.tm_mon + 1, t_m.tm_mday); // get year_month_day str

	while (1) {

		lcd_clean_workspace();
		lcd_show_string(9, 1, strlen(c_allspace_str), c_allspace_str);
		lcd_show_string(9, 1, strlen(menu_name1_3_2), menu_name1_3_2);

		lcd_show_string(2, 1, strlen(c_history_meter_id_str),
				c_history_meter_id_str); /// 表号字符串
		lcd_show_string(3, 4, strlen(meter_id), meter_id);
		lcd_show_string(4, 1, strlen(c_history_day_str), c_history_day_str); /// 日时标 
		lcd_show_string(5, 4, strlen(year_month_day), year_month_day); /// 20161019

		// 输入year_month_day和meter_id
		if (input_string(3, "   ", fmt_num, meter_id, strlen(meter_id))) {
			lcd_show_string(3, 4, strlen(meter_id), meter_id);
		} else {
			break;
		}
		if (input_string(5, "   ", fmt_num, year_month_day,
				strlen(year_month_day))) {
			lcd_show_string(5, 4, strlen(year_month_day), year_month_day);
		} else {
			break;
		}

		hexstr_to_str(meter_id_byte, meter_id, 14);
		mtidx = -1;
		mtidx = fgasmeter_getidx_by_gasmeter(meter_id_byte);
		if (mtidx >= 0) {
			// valid mtidx, continue
		} else {
			// invalid mtidx
			printf("no day idx found str 1?\n");
			lcd_show_string(9, 1, strlen(c_allspace_str), c_allspace_str);
			lcd_show_string(9, 1, strlen(c_invalid_meter_id_str),
					c_invalid_meter_id_str);
			sleep(1);
			continue;
		}

		//printf("year_month_day = %s\n", year_month_day);
		if (if_date_str_is_valid(year_month_day)) {
			// valid, continue
			//printf("data_str is valid\n");
		} else {
			lcd_show_string(9, 1, strlen(c_allspace_str), c_allspace_str);
			lcd_show_string(9, 1, strlen(c_year_month_day_invalid_str),
					c_year_month_day_invalid_str);
			sleep(1);
			continue;
		}

		year = atoi(year_month_day) / 10000;
		month = atoi(year_month_day) % 10000 / 100;
		day = atoi(year_month_day) % 100;

		day_idx = -1;
		//printf("%04d - %02d - %02d\n",tm_buf.tm_year + 1900, tm_buf.tm_mon + 1, tm_buf.tm_mday);
		day_idx = fday_get_datablock_index_by_time(year, month, day);
		//day_idx = fday_get_datablock_index_by_time(tm_buf.tm_year + 1900, tm_buf.tm_mon + 1, atoi(year_month_day)%100);
		//day_idx = fday_get_datablock_index_by_time(atoi(year_month_day)/10000,  atoi(year_month_day)%10000/100, atoi(year_month_day)%100);
		if (day_idx < 0) {
			// printf(" day_idx = %d\n", day_idx);
			// printf("no day idx found str 2?\n");
			lcd_show_string(9, 1, strlen(c_allspace_str), c_allspace_str);
			lcd_show_string(9, 1, strlen(c_no_day_idx_found_str),
					c_no_day_idx_found_str);
			sleep(1);
			continue; // invalid continue
		} else {
			// valid continue;
		}

		if (fday_get_data(day_idx, mtidx, 0x901F, &di_data[0])) {
			// valid //printf("get start data\n"); // OK
		} else {
			// invalid,
			continue;
		}
		start_flux = reverse_byte_array2bcd(di_data[0].di_data.flux,
				sizeof(di_data[0].di_data.flux)) * 0.1;
		//snprintf(buf, 20, "    %7.1f m3", start_flux); // start flux

		//current_day = false;
		if (t_m.tm_year + 1900 == year && t_m.tm_mon + 1 == month
				&& t_m.tm_mday == day) {
			//current_day = true;
			if (!fcurrent_get_data(mtidx, 0x901F, &di_data[1])) {
				continue; // invalid
			} else {
				//printf("get current_data\n");// valid
			}
		} else {
			//current_day = false;
			///printf("year_month_day = %s\n", year_month_day);

			if (a_day_later(year_month_day) == 0) {
				continue; /// not valid time;
			}

			year = a_day_later(year_month_day) / 10000;
			month = a_day_later(year_month_day) % 10000 / 100;
			day = a_day_later(year_month_day) % 100;
			///printf("year_month_day = %d\n", a_day_later(year_month_day)); //// debug
			day_idx = fday_get_datablock_index_by_time(year, month, day);
			if (day_idx < 0) {
				lcd_show_string(9, 1, strlen(c_allspace_str), c_allspace_str);
				lcd_show_string(9, 1, strlen(c_no_next_day_data_str),
						c_no_next_day_data_str);
				sleep(1);
				continue; // invalid
			} else {
				// valid, continue
			}

			if (!fday_get_data(day_idx, mtidx, 0x901F, &di_data[1])) {
				lcd_show_string(9, 1, strlen(c_allspace_str), c_allspace_str);
				lcd_show_string(9, 1, strlen(c_no_next_day_data_str),
						c_no_next_day_data_str);
				sleep(1);
				continue;
			} else {
				// valid
			}
		} // end flux
		end_flux = reverse_byte_array2bcd(di_data[1].di_data.flux,
				sizeof(di_data[1].di_data.flux)) * 0.1; // end flux float
		//snprintf(buf[1], 20, "    %7.1f m3", end_flux);

		delta_flux = end_flux - start_flux; // delta flux
		//snprintf(buf[2], 20, "    %7.1f m3", delta_flux);

		lcd_clean_workspace();
		snprintf(buf, 21, "%s%.1f m3", c_daily_flux_str, delta_flux);
		lcd_show_string(2, 1, strlen(buf), buf);
		snprintf(buf, 21, "%s%.1f m3", c_daily_initiate_flux_str, start_flux);
		lcd_show_string(3, 1, strlen(buf), buf);
		tt = di_data[0].read_tt;
		localtime_r(&tt, &tm_buf);
		snprintf(buf, 21, "%04d/%02d/%02d %02d:%02d", tm_buf.tm_year + 1900,
				tm_buf.tm_mon + 1, tm_buf.tm_mday, tm_buf.tm_hour,
				tm_buf.tm_min); /// 显示时间
		lcd_show_string(4, 3, strlen(buf), buf);
		snprintf(buf, 21, "%s%.1f m3", c_next_day_initiate_flux_str, end_flux);
		lcd_show_string(5, 1, strlen(buf), buf);
		tt = di_data[1].read_tt;
		localtime_r(&tt, &tm_buf);
		snprintf(buf, 20 + 1, "%04d/%02d/%02d %02d:%02d", tm_buf.tm_year + 1900,
				tm_buf.tm_mon + 1, tm_buf.tm_mday, tm_buf.tm_hour,
				tm_buf.tm_min); /// 显示时间
		lcd_show_string(6, 3, strlen(buf), buf);

		key = getch_timeout();
		if (key == KEY_ESC) {
			break;
		} else if (key == KEY_ENTER) {
			continue;
		} else if (key == KEY_UP || key == KEY_DOWN || key == KEY_RIGHT
				|| key == KEY_LEFT) {
			continue;
		}
	}
}

/*
 // get valid meter data in variable
 static void get_valid_meter_test(BYTE flag, void *para, const char *info)
 {
 WORD idx, i; // 0-65525, WORD
 struct{
 BYTE address[7];
 } valid_meters[100];
 BYTE meter_address[7];
 BYTE meter_collector[5];

 for(idx = 0; idx < 1000; idx++){
 if(fgasmeter_getgasmeter(idx, meter_address, meter_collector)){
 i++;
 memcpy(valid_meters[i].address, meter_address, sizeof(meter_address));
 PRINTB("find valid meter: ", meter_address, sizeof(meter_address));
 printf("which idx is: %d\n", idx);
 }else{
 continue;
 }
 }

 ///fmon_get_datablock_index();
 ///fday_get_datablock_index();
 }
 */

/*
 static void get_valid_day_block_from_variable_test(BYTE flag, void *para, const char *info)
 {
 FDAY_DATA_BLOCK fday_blocks[MAX_GASMETER_DAY_CNT];
 int size, count, idx, fd;
 struct tm tm;

 size = 0;
 size = sizeof(FDAY_DATA_BLOCK) * MAX_GASMETER_DAY_CNT;

 memset(fday_blocks, 0, size);

 /// initiate
 for(idx = 0; idx< MAX_GASMETER_DAY_CNT; idx++){
 fday_block_init_out_use(&fday_blocks[idx]);
 }
 count = 0;
 for(idx = 0; idx< MAX_GASMETER_DAY_CNT; idx++){
 if(fday_blocks[idx].valid){
 count++;
 }else{
 }
 }
 printf("valid day block after initiating: %d\n", count); 
 /// buf block
 read_fday_blocks(fday_blocks); /// buf block
 count = 0;
 for(idx = 0; idx< MAX_GASMETER_DAY_CNT; idx++){
 ///if(fday_blocks[idx].valid){
 count++;
 printf("block index: %d ", idx);
 printf("valid: %s ",  fday_blocks[idx].valid ? "valid":"invalid");
 localtime_r(&fday_blocks[idx].tt, &tm);
 printf("date(month-day): %0d-%0d\n", tm.tm_mon+1, tm.tm_mday);
 ///}else{
 ///}
 }
 printf("valid day block after reading variable fday_info: %d\n", count);

 }
 */

/*
 static void get_valid_day_block_from_file_test(BYTE flag, void *para, const char *info)
 {	

 FDAY_DATA_BLOCK fday_blocks[MAX_GASMETER_DAY_CNT];
 int size, count, idx, fd;
 int buf = 0;
 struct tm tm;

 size = 0;
 size = sizeof(FDAY_DATA_BLOCK) * MAX_GASMETER_DAY_CNT;

 memset(fday_blocks, 0, size);

 /// initiate
 for(idx = 0; idx< MAX_GASMETER_DAY_CNT; idx++){
 fday_block_init_out_use(&fday_blocks[idx]);
 }
 count = 0;
 for(idx = 0; idx< MAX_GASMETER_DAY_CNT; idx++){
 if(fday_blocks[idx].valid){
 count++;
 }else{
 }
 }
 printf("valid day block after initiating: %d\n", count); 

 /// read
 fd = open("f_day.dat", O_RDWR); /// fd stands for a space.
 if(fd >= 0){
 /// return;
 printf("read f_day.dat successfully\n");
 }else if(fd<0){
 return;
 }	

 //FDAY_DATA data;
 //off_t offset;
 //memset(&data, 0, sizeof(FDAY_DATA));
 //offset = 2 * sizeof(FDAY_DATA_BLOCK) + offsetof(FDAY_DATA_BLOCK, daydata) + 0 * sizeof(FDAY_DATA); /// offset
 //lseek(fd,offset, SEEK_SET);
 //safe_read(fd, &data, sizeof(FDAY_DATA));
 //PRINTB("save data meter address: ",data.address, 7);
 //PRINTB("valid? %d\n", &data.valid, 1);
 //PRINTB("data: ", &data.u.data.data_901f.di_data.flux[0], 5);
 //PRINTB("data: ", data.u.data.data_901f.di_data.flux, 5);

 lseek(fd, (off_t)0, SEEK_SET);
 buf = safe_read(fd, &fday_blocks[0], size);

 if(buf > 0){
 printf("read file size: %d\n", buf);
 } else {
 printf("read file failed\n");
 }
 /// get the count 
 count = 0;
 for(idx = 0; idx< MAX_GASMETER_DAY_CNT; idx++){
 ///if(fday_blocks[idx].valid){
 count++;
 printf("block index: %d ", idx);
 printf("valid: %s ",  fday_blocks[idx].valid ? "valid":"invalid");
 localtime_r(&fday_blocks[idx].tt, &tm);
 printf("date(month-day): %0d-%0d\n", tm.tm_mon+1, tm.tm_mday);
 ///}else{
 ///}
 }
 printf("valid day block after reading file f_day.dat: %d\n", count);

 close(fd);

 }
 */

/*
 static void write_variable_into_file(BYTE flag, void *para, const char *info)
 {
 FDAY_DATA_BLOCK fday_blocks[MAX_GASMETER_DAY_CNT];
 int size, idx, fd;
 int buf = 0;
 ///struct tm tm;

 size = sizeof(FDAY_DATA_BLOCK) * MAX_GASMETER_DAY_CNT;
 memset(fday_blocks, 0, size);

 read_fday_blocks(fday_blocks); 
 fd = open("f_day.dat", O_RDWR); 
 safe_write(fd, fday_blocks, size);
 ///printf("assuming write properly\n");
 fdatasync(fd);
 close(fd); /// why close fd, clear, because we need open downward
 }
 */

/*
 static void menu_data_test(BYTE flag, void *para, const char *info)
 {

 ITEMS_MENU items_menu;
 int idx = 0;
 init_menu(&items_menu.menu);
 items_menu.cur_line = 1;

 items_menu.menu.str[idx] = c_get_valid_meter_str;
 items_menu.func[idx++] = get_valid_meter_test;
 items_menu.menu.str[idx] = c_get_valid_day_block_from_variable_str;
 items_menu.func[idx++] = get_valid_day_block_from_variable_test;
 items_menu.menu.str[idx] = c_get_valid_day_block_from_file_str;
 items_menu.func[idx++] = get_valid_day_block_from_file_test;
 ///items_menu.menu.str[idx] = c_write_variable_into_file_str;
 ///items_menu.func[idx++] = write_variable_into_file;
 ///items_menu.menu.str[idx] = c_update_day_block_from_varieble_into_file_str;
 ///items_menu.func[idx++] = update_day_block_into_file_test;
 ///idx++;

 items_menu.menu.line_num = idx;
 process_items(&items_menu, info, FALSE);

 }
 */

/*
 #include "gpio.h"
 static void power_off_test(BYTE flag, void *para, const char *info)
 {
 modem_hard_reset();
 }
 */

static void history_data_display(BYTE flag, void *para, const char *info) {
	ITEMS_MENU items_menu;
	int idx = 0;
	init_menu(&items_menu.menu);
	items_menu.cur_line = 1;

	items_menu.menu.str[idx] = menu_name1_3_1;
	items_menu.func[idx++] = month_data_query;
	items_menu.menu.str[idx] = menu_name1_3_2;
	items_menu.func[idx++] = daily_data_query;
	//items_menu.menu.str[idx] = menu_name_test;
	//items_menu.func[idx++] = menu_data_test;
	///items_menu.menu.str[idx] = menu_name_test;
	///items_menu.func[idx++] = power_off_test;

	items_menu.menu.line_num = idx;
	process_items(&items_menu, info, FALSE);
}

int gasmeter_read_di(const BYTE *address, const BYTE *collector, WORD di,
		BYTE *buf, int max_len);
static void realtime_data_display(BYTE flag, void *para, const char *info) /// 实时数据显示
{
	char meter_id[7 * 2 + 1];
	///bool forward;
	unsigned char key;
	//char *prompt = "   "; /// tips
	///int row;
	BYTE meter_id_byte[7];
	BYTE collector[5];
	WORD di = 0x901F; //// warning /// WORD di;
	BYTE resp_buf[512];
	int resp_len;
	int index_of_meter;
	///BYTE meter_status;

	double value_current;
	char buf[21];
	double value_current_balance;
	long read_tt;

	///memset(meter_id, 0, sizeof(meter_id));
	memcpy(meter_id, "23051606000007", 15); // tips meter id
	meter_id[14] = '\0';
	memset(collector, 0, sizeof(collector)); // tips collector id

	while (1) {
		lcd_clean_workspace();
		lcd_show_string(9, 1, strlen(c_allspace_str), c_allspace_str);
		lcd_show_string(9, 1, strlen(menu_name1_4), menu_name1_4); // 实时数据显示

		lcd_show_string(2, 1, strlen(c_history_meter_id_str),
				c_history_meter_id_str);

		if (input_string(3, "   ", fmt_num, meter_id, strlen(meter_id))) {
			lcd_show_string(3, 4, strlen(meter_id), meter_id);

		} else {
			return;
		}

		///printf("meter_id: %s\n",meter_id); /// 23 05 16 06 00 00 01
		hexstr_to_str(meter_id_byte, meter_id, 14); /// address <- input // 23 05 16 06 00 00 01 
		index_of_meter = fgasmeter_getidx_by_gasmeter(meter_id_byte);
		//printf("index_of_meter:");
		if (index_of_meter < 0) {
			lcd_clean_workspace();
			lcd_show_string(2, 1, strlen(c_not_find_meter_in_concentrator),
					c_not_find_meter_in_concentrator);
			sleep(1);
			continue; /// 显示刚才输入的表号
		} else if (index_of_meter >= 0) {
			lcd_clean_workspace();
			lcd_show_string(2, 1, strlen(c_reading_meter_str),
					c_reading_meter_str);

			time(&read_tt);
			resp_len = gasmeter_read_di(meter_id_byte, collector, di, resp_buf,
					sizeof(resp_buf));
			// save data
			if (resp_len <= 0) {
				fcurrent_set_status(fgasmeter_getidx_by_gasmeter(meter_id_byte),
						0x901F, GASMETER_READ_STATUS_ABORT);
				continue;
			} else {
				save_data(meter_id_byte, collector, 0x901F, read_tt, resp_buf,
						resp_len);
			}

			///PRINTB("RESP_BUF: %d\n", resp_buf, resp_len);
			if (resp_len > 0) {
				lcd_clean_workspace();
				lcd_show_string(2, 1, strlen(c_history_meter_id_str),
						c_history_meter_id_str);
				lcd_show_string(2, strlen(c_history_meter_id_str) + 1,
						strlen(meter_id), meter_id);
				lcd_show_string(3, 1, strlen(c_current_flux_display_value_str),
						c_current_flux_display_value_str);

				//PRINTB("buff is: ", resp_buf + 3, 5);/* fuck */ /// 累计值 index 3-7
				value_current = reverse_byte_array2bcd(resp_buf + 3, 5) * 0.1; /// resp_buf + 3
				//printf("value is: %f\n", value_current);
				snprintf(buf, 15, "%11.1f m3", value_current); /// 开阀才可以写表的底数, 用掌机开阀
				//printf("%s\n", buf);
				lcd_show_string(4, 1, strlen(buf), buf); // 显示信息

				lcd_show_string(5, 1, strlen(c_balance_flux_amount_str),
						c_balance_flux_amount_str);
				/// value_current_balance = reverse_byte_array2bcd(resp_buf + 8, 7) * 0.1;
				value_current_balance = reverse_byte_array2bcd(resp_buf + 8, 5)
						* 0.1;
				snprintf(buf, 15, "%11.1f m3", value_current_balance);
				lcd_show_string(6, 1, strlen(buf), buf);

				/// 结算日累计值 index 8-14
				lcd_show_string(7, 1, strlen(c_valve_status_str),
						c_valve_status_str);
				/// PRINTB("resp_buf: ", resp_buf, 22);  
				// 1F 90 06 
				// 40 05 00 00 00 累计
				// 00 00 00 00 00 结算日累计
				// 00 00 12 01 01 20 16 日期
				// 21 00
				if (!(resp_buf[20] & 0x01)) { /// 2100关阀，2000开阀
					lcd_show_string(7, strlen(c_valve_status_str) + 1,
							strlen(c_valve_status_open_str),
							c_valve_status_open_str); /// 关阀
				} else {
					lcd_show_string(7, strlen(c_valve_status_str) + 1,
							strlen(c_valve_status_closed_str),
							c_valve_status_closed_str);  /// 开阀
				}

				key = getch_timeout();
				if (key == KEY_ESC) {
					break;
				} else if (key == KEY_ENTER || key == KEY_DOWN
						|| key == KEY_RIGHT || key == KEY_LEFT || key == KEY_UP) {
					continue;
				}
			}
		} else if (resp_len <= 0) {
			return;
		}
	}
}
/** 数据形式
 * 表号：23051606000006
 * 当前流量示值
 * fffffff.fff m3 
 * 结算日流量累计值
 * fffffff.fff m3 
 * 阀门状态：异常
 */

static void read_meter_assembly_function(BYTE flag, void *para,
		const char *info) {
	int meters_amount, valid_num, failed_num, idx;
	int resp_len; /// merely is a space
	BYTE address[7], collector[5], repeater[2];
	BYTE resp_buf[512];
	char buf[21], meter_address_str[15], collector_str[11], repeater_str[5];
	long read_tt;
	///unsigned char key;
	memset(address, 0, sizeof(address));
	memset(collector, 0, sizeof(collector));
	memset(repeater, 0, sizeof(repeater));

	lcd_clean_workspace();
	meters_amount = get_valid_meter_amount_in_database();
	valid_num = 0;
	failed_num = 0;
	for (idx = 0; idx < MAX_GASMETER_NUMBER; idx++) {

		if (fgasmeter_getgasmeter(idx, address, collector)) {

			snprintf(buf, 21, "%s%d\x2F%d", c_reading_meter_str, idx + 1,
					meters_amount);
			lcd_show_string(2, 1, strlen(c_allspace_str), c_allspace_str);
			lcd_show_string(2, 1, strlen(buf), buf);  /// 正在抄表...i/valid_meters

			snprintf(buf, 21, "%s%d\x2F%d", c_reading_success_str, valid_num,
					meters_amount);
			lcd_show_string(3, 1, strlen(c_allspace_str), c_allspace_str); // 成功数：8/20
			lcd_show_string(3, 1, strlen(buf), buf);

			snprintf(buf, 21, "%s%d\x2F%d", c_reading_failure_str, failed_num,
					meters_amount);
			lcd_show_string(4, 1, strlen(c_allspace_str), c_allspace_str); // 失败数：8/20
			lcd_show_string(4, 1, strlen(buf), buf);

			//get the collector index
			//get the collector address
			snprintf(buf, 21, "%s%s", c_current_collector_str,
					hex_to_str(collector_str, sizeof(collector_str), collector,
							5, FALSE));
			lcd_show_string(5, 1, strlen(c_allspace_str), c_allspace_str);
			lcd_show_string(5, 1, strlen(buf), buf); // 采集器

			if (fgasmeter_get_repeater(address, repeater)) { // get the repeater address
				; // get repeater address and continue
			} else {
				memset(repeater, 0, sizeof(repeater));
			}

			snprintf(buf, 21, "%s%s", c_current_repeater_str,
					hex_to_str(repeater_str, sizeof(repeater_str), repeater, 2,
							FALSE));
			lcd_show_string(6, 1, strlen(c_allspace_str), c_allspace_str);
			lcd_show_string(6, 1, strlen(buf), buf); // 当前表

			snprintf(buf, 21, "%s%s", c_current_reading_meter_str,
					hex_to_str(meter_address_str, sizeof(meter_address_str),
							address, 7, FALSE));
			lcd_show_string(7, 1, strlen(c_allspace_str), c_allspace_str);
			lcd_show_string(7, 1, strlen(buf), buf); // 当前表

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
						resp_len); /// save_data
				//continue;
			}				/// save data

			snprintf(buf, 21, "%s%d/%d", c_reading_success_str, valid_num,
					meters_amount);
			lcd_show_string(3, 1, strlen(c_allspace_str), c_allspace_str);// 成功数：8/20
			lcd_show_string(3, 1, strlen(buf), buf);

			snprintf(buf, 21, "%s%d/%d", c_reading_failure_str, failed_num,
					meters_amount);
			lcd_show_string(4, 1, strlen(c_allspace_str), c_allspace_str);// 失败数：8/20
			lcd_show_string(4, 1, strlen(buf), buf);

			continue; /// complete control flow
		} else {
			continue;
		} /// index has no meter_id for use
	}

	//snprintf(buf, 21, "%s(%d\x2F%d)",c_complete_str, idx+1, meters_amount);
	lcd_show_string(2, 1, strlen(c_allspace_str), c_allspace_str);
	lcd_show_string(2, 1, strlen(c_complete_str), c_complete_str); /// 正在抄表...i/valid_meters

	getch_timeout();
	return;
}

static void read_meter_assembly(BYTE flag, void *para, const char *info) {
	if (verify_password() != 0)
		return;

	ITEMS_MENU items_menu;
	int idx = 0;

	init_menu(&items_menu.menu);
	items_menu.cur_line = 1;

	items_menu.menu.str[idx] = c_ensure_str;
	items_menu.func[idx++] = read_meter_assembly_function;
	items_menu.menu.line_num = idx;
	process_items(&items_menu, info, FALSE);

}

/*
 static void repeter_query(BYTE flag, void *para, const char *info)
 {

 }

 static void repeater_enable(BYTE flag, void *para, const char *info)
 {

 }
 */

static void meter_repeater_setting_new(BYTE flag, void *para, const char *info) /// repeater != collector
{
	// 显示已有repeater, 没有显示0000
	char repeater_str[5], address_str[15], buff[MAX_SCREEN_COL + 1];
			//char enable_str[2];
	BYTE repeater[2], address[7];
	//int idx;
	//bool enable;
	int i;

	//idx = 0;
	/*idx = */fgasmeter_getidx_by_gasmeter(address); /// init the display meter address

	lcd_clean_workspace();
	memcpy(address_str, "23051606000007", 15); // 表号:23051606000102 //input
	if (!input_string(2, c_history_meter_id_str, fmt_num, address_str, 14))
		return;  // 表号:23051606000102 // input  // failed

	for (i = 0; i < sizeof(address); i++) { /// sizeof(address) = 7
		address[i] = bin_to_bcd(
				(address_str[2 * i] - '0') * 10 + address_str[2 * i + 1] - '0');
	}
	///PRINTB("address: ", address, sizeof(address));

	snprintf(buff, sizeof(buff), "%s%s", c_history_meter_id_str, address_str);
	lcd_show_string(2, 1, strlen(buff), buff);  // 表号:23051606000102

	if (!fgasmeter_get_repeater(address, repeater)) {
		//enable_str[0] = 'N'; enable_str[1] = '\0';
		memcpy(repeater_str, "0000", 5);
	} else {
		//enable_str[0] = 'Y'; enable_str[1] = '\0';
		hex_to_str(repeater_str, sizeof(repeater_str), repeater, 2, FALSE); /// 中继复位 /// 全部成为0000
	} /// repeater invalid or no repeater

	snprintf(buff, sizeof(buff), "%s", c_repeater_str);
	/*
	 snprintf(buff, sizeof(buff), "%s%s", c_repeater_str, repeater_str);
	 lcd_show_string(4, 1, strlen(buff), buff);
	 if(!input_string(3, c_enable_str, yes_or_no, enable_str,1)){
	 return;
	 }

	 snprintf(buff, sizeof(buff), "%s%s", c_enable_str, enable_str);
	 lcd_show_string(3, 1, strlen(buff), buff); // 使能:Y/N // input
	 */

	if (!input_string(3, c_repeater_str, fmt_num, repeater_str, 4)) {
		return;
	}
	snprintf(buff, sizeof(buff), "%s%s", c_repeater_str, repeater_str);
	lcd_show_string(3, 1, strlen(buff), buff); // 中继:000000 // input

	for (i = 0; i < sizeof(repeater); i++) {
		repeater[i] = bin_to_bcd(
				(repeater_str[2 * i] - '0') * 10 + repeater_str[2 * i + 1]
						- '0');
	}

	if (!fgasmeter_set_repeater(address, repeater)) // setting enable and the repeater_id;
			{
		lcd_show_string(4, 1, strlen("FAILED!"), "FAILED!");
	} else {
		lcd_show_string(4, 1, strlen("SUCCESS!"), "SUCCESS!");
	}
	sleep(1);
	return; // ok and return 
}

/*
 static void meter_repeater_setting(BYTE flag, void *para, const char *info)
 {
 ITEMS_MENU items_menu;
 int idx = 0;

 init_menu(&items_menu.menu);
 items_menu.cur_line = 2;

 items_menu.menu.str[idx] = c_repeater_query_str;
 items_menu.func[idx++] = repeter_query;
 items_menu.menu.str[idx] = c_repeater_setting_str;
 items_menu.func[idx++] = repeater_enable;

 items_menu.menu.line_num = idx;
 process_items(&items_menu, info, FALSE);
 }
 */

/// #include <unistd.h>
/// #include <fcntl.h>
/// int open(const char *pathname, int oflag, ... /* mode_t mode */ );
/// ssize_t read(int filedes, void *buf, size_t nbytes);
void import_meters_into_the_fgasmeter_structure(const char *filename) {

	/// read a file and get every line to change the meters structure
	/// no <regex.h>, cannot use regex.h
	/// find if exist regex.h // find /opt/linux-devkit/sysroots/x86_64-arago-linux/ -name "regex.h" -ls /// find /opt/linux-devkit/sysroots/x86_64-arago-linux/ -name "stdio.h" -ls
	/// include <sys/types.h>
	/// include <regex.h>
	// grep check the file

	FILE *fp;
	///memset(buff, 0, sizeof(buff));
	///char line[LINE_LENGTH];
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
					(a_meter_id[2 * i] - '0') * 10 + a_meter_id[2 * i + 1]
							- '0');
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
				printf("success adding a gasmeter: %s\n", a_meter_id);// add, continue to add the next gasmeter;
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
	// int i;

	///printf("sizeof(suffix) = %d\n",sizeof(suffix));
	memset(suffix, 0, sizeof(suffix));
	///printf("successfully init the suffix array\n");

	lcd_clean_workspace();

	file_exist_flag = false;
	if (stat(filename, &buf) == 0) {
		file_exist_flag = true;
	} else {
		file_exist_flag = false;
	} // get file exist flag

	if (!file_exist_flag) {
		lcd_show_string(2, 1, strlen(c_meters_file_not_exist_str),
				c_meters_file_not_exist_str);
		sleep(1);
		return;
	}

	lcd_show_string(2, 1, strlen(c_meters_file_importing_str),
			c_meters_file_importing_str);
	import_meters_into_the_fgasmeter_structure(filename); /// start use file data save into the data
	lcd_show_string(3, 1, strlen(c_meters_import_success_str),
			c_meters_import_success_str);
	sleep(1);

	return; // return function
}

static void import_meters_in_bunch(BYTE flag, void *para, const char *info) {
	if (verify_password() != 0)
		return;

	//MENU menu;

	int idx = 0;
	ITEMS_MENU items_menu;

	init_menu(&items_menu.menu);
	items_menu.cur_line = 1;
	items_menu.menu.str[idx] = c_ensure_str;
	items_menu.func[idx++] = import_meters_in_bunch_function;
	items_menu.menu.line_num = idx;
	process_items(&items_menu, info, FALSE);
}

static void query_data(BYTE flag, void *para, const char *info) {

	ITEMS_MENU items_menu;
	int idx = 0;

	init_menu(&items_menu.menu);
	items_menu.cur_line = 1;

	items_menu.menu.str[idx] = menu_name1_1;
	items_menu.func[idx++] = read_meter_data;
	items_menu.menu.str[idx] = menu_name1_2;
	items_menu.func[idx++] = alarm_events;
	items_menu.menu.str[idx] = menu_name1_3;
	items_menu.func[idx++] = history_data_display;
	items_menu.menu.str[idx] = menu_name1_4;
	items_menu.func[idx++] = realtime_data_display;
	items_menu.menu.str[idx] = menu_name1_5;
	items_menu.func[idx++] = read_meter_assembly;

	items_menu.menu.line_num = idx;
	process_items(&items_menu, info, FALSE);

	//menu_ongoing(flag, para, info);
}

static void set_comm_channel(BYTE flag, void *para, const char *info) // 通讯讯道
{
	// TODO set the communication channel: rf485, ethernet, gprs/cdma
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

	fparam_get_value(FPARAMID_COMM_HOST_IP_PRI, host_ip, sizeof(host_ip)); // 获得当前port
	fparam_get_value(FPARAMID_COMM_HOST_PORT_PRI, host_port, sizeof(host_port)); // 获得当前port 

	snprintf(ip_addr, sizeof(ip_addr), "%u.%u.%u.%u", host_ip[0], host_ip[1],
			host_ip[2], host_ip[3]); /// 获取当前port字符串
	ip_port = (host_port[0] << 8) + host_port[1];
	sprintf(port_buf, "%d", ip_port); /// get port_str 

	init_menu(&param_set.menu); /// 
	param_set.menu.line_num = 3;
	param_set.menu.str[0] = c_ip_address_str;
	param_set.menu.str[1] = NULL;
	param_set.menu.str[2] = c_port_str;
	param_set.group_num = 2;
	param_set.group_idx = 0;
	idx = 0;
	init_input_set(&param_set.input[idx], 2, 1, strlen(ip_addr), ip_addr, 15); /// initiate the input set.
	param_set.keyboard_type[idx++] = _num_keyboard;
	init_input_set(&param_set.input[idx], 3, 6, strlen(port_buf), port_buf, 5); ///
	param_set.keyboard_type[idx++] = _num_keyboard;
	process_param_set(&param_set, info);
	strcpy(ip_addr, param_set.input[0].str);
	ip = inet_addr(ip_addr);
	host_ip[0] = ip;
	host_ip[1] = ip >> 8;
	host_ip[2] = ip >> 16;
	host_ip[3] = ip >> 24;
	strcpy(port_buf, param_set.input[1].str);
	ip_port = atoi(port_buf); /// char -> int "1000" -> 1000
	host_port[0] = ip_port >> 8;
	host_port[1] = ip_port;
	fparam_set_value(FPARAMID_COMM_HOST_IP_PRI, host_ip, sizeof(host_ip)); /// set ip
	fparam_set_value(FPARAMID_COMM_HOST_PORT_PRI, host_port, sizeof(host_port)); /// set port
}

static void set_minor_host_ip_and_port(BYTE flag, void *para, const char *info) {
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

	fparam_get_value(FPARAMID_COMM_HOST_IP_MINOR, host_ip, sizeof(host_ip)); // 获得当前port
	fparam_get_value(FPARAMID_COMM_HOST_PORT_MINOR, host_port,
			sizeof(host_port)); // 获得当前port 

	snprintf(ip_addr, sizeof(ip_addr), "%u.%u.%u.%u", host_ip[0], host_ip[1],
			host_ip[2], host_ip[3]); /// 获取当前port字符串
	ip_port = (host_port[0] << 8) + host_port[1];
	sprintf(port_buf, "%d", ip_port); /// get port_str 

	init_menu(&param_set.menu); /// 
	param_set.menu.line_num = 3;
	param_set.menu.str[0] = c_ip_address_str;
	param_set.menu.str[1] = NULL;
	param_set.menu.str[2] = c_port_str;
	param_set.group_num = 2;
	param_set.group_idx = 0;
	idx = 0;
	init_input_set(&param_set.input[idx], 2, 1, strlen(ip_addr), ip_addr, 15); /// 
	param_set.keyboard_type[idx++] = _num_keyboard;
	init_input_set(&param_set.input[idx], 3, 6, strlen(port_buf), port_buf, 5); ///
	param_set.keyboard_type[idx++] = _num_keyboard;
	process_param_set(&param_set, info);
	strcpy(ip_addr, param_set.input[0].str);
	ip = inet_addr(ip_addr);
	host_ip[0] = ip;
	host_ip[1] = ip >> 8;
	host_ip[2] = ip >> 16;
	host_ip[3] = ip >> 24;
	strcpy(port_buf, param_set.input[1].str);
	ip_port = atoi(port_buf); /// char -> int "1000" -> 1000
	host_port[0] = ip_port >> 8;
	host_port[1] = ip_port;
	fparam_set_value(FPARAMID_COMM_HOST_IP_MINOR, host_ip, sizeof(host_ip)); /// set ip
	fparam_set_value(FPARAMID_COMM_HOST_PORT_MINOR, host_port,
			sizeof(host_port)); /// set port
}

static void set_apn(BYTE flag, void *para, const char *info) {
	if (verify_password() != 0)
		return;

	PARAM_SET param_set;
	char apn_id[APN_LENGTH], apn_user_id[APN_LENGTH],
			apn_user_password[APN_LENGTH];
	int idx;

	memset(&param_set, 0, sizeof(param_set)); /// incompatible type for argument 1 of 'memset'
	memset(apn_id, 0, sizeof(apn_id));
	memset(apn_user_id, 0, sizeof(apn_user_id));
	memset(apn_user_password, 0, sizeof(apn_user_password));

	/// SET apn and use it later
	fparam_get_value(FPARAMID_APN_ID, apn_id, sizeof(apn_id));
	fparam_get_value(FPARAMID_APN_USER_ID, apn_user_id, sizeof(apn_user_id));
	fparam_get_value(FPARAMID_APN_USER_PASSWD, apn_user_password,
			sizeof(apn_user_password)); /// 获得当前VPN值 /// read the data //printf("APN_ID: %s\n", apn_id); // OK

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
			apn_user_id, APN_LENGTH - 1); ///
	param_set.keyboard_type[idx++] = _upper_letter_keyboard;
	init_input_set(&param_set.input[idx], 6, 1, strlen(apn_user_password),
			apn_user_password, APN_LENGTH - 1); ///
	param_set.keyboard_type[idx++] = _upper_letter_keyboard;

	process_param_set(&param_set, info);

	/// break the paramset and save the data;
	strcpy(apn_id, param_set.input[0].str);
	strcpy(apn_user_id, param_set.input[1].str);
	strcpy(apn_user_password, param_set.input[2].str);
	fparam_set_value(FPARAMID_APN_ID, apn_id, 32);
	fparam_set_value(FPARAMID_APN_USER_ID, apn_user_id, 32);
	fparam_set_value(FPARAMID_APN_USER_PASSWD, apn_user_password, 32);

	/*
	 * APN: 
	 * UNINET
	 * APN用户名：
	 * CMNET
	 * APN用户密码：
	 * CMNET
	 */
}

static void set_heartbeat_to_mainstation(BYTE flag, void *para,
		const char *info) {

	if (verify_password() != 0)
		return;

	PARAM_SET param_set;
	BYTE heart_beat[2];
	char hb_s[4 + 1];
	int buf, idx;

	memset(&param_set, 0, sizeof(param_set));
	memset(heart_beat, 0, sizeof(heart_beat));
	fparam_get_value(FPARAMID_HEARTBEAT_CYCLE, heart_beat, sizeof(heart_beat));
	snprintf(hb_s, sizeof(hb_s), "%u", (heart_beat[0] << 8) + heart_beat[1]);

	init_menu(&param_set.menu);
	param_set.menu.line_num = 2;
	param_set.menu.str[0] = c_heart_beat_min_str;

	param_set.menu.str[1] = NULL;
	param_set.group_num = 1;
	param_set.group_idx = 0;
	idx = 0;

	init_input_set(&param_set.input[idx], 2, 1, strlen(hb_s), hb_s, 5);
	param_set.keyboard_type[idx++] = _num_keyboard;
	process_param_set(&param_set, info);

	strcpy(hb_s, param_set.input[0].str); /// hb_s

	buf = atoi((const char*) hb_s);

	if (buf > 65525) {
		return; /// set value is too big
	} else if (buf <= 65525) {
		// seconds = buf * 60;
		heart_beat[0] = buf / (0x01 << 8);
		heart_beat[1] = buf % (0x01 << 8);
		fparam_set_value(FPARAMID_HEARTBEAT_CYCLE, heart_beat, 2); /// len
		/// tip set OK
	}

	return;
}

static void set_con_addr(BYTE flag, void *para, const char *info) //// byte-flag
{

	ITEMS_MENU items_menu;
	int idx = 0;

	init_menu(&items_menu.menu);
	items_menu.cur_line = 1;

	items_menu.menu.str[idx] = menu_name2_1_1;
	items_menu.func[idx++] = set_comm_channel;
	items_menu.menu.str[idx] = menu_name2_1_2;
	items_menu.func[idx++] = set_prior_host_ip_and_port;  /// 主IP与端口
	items_menu.menu.str[idx] = menu_name2_1_3;
	items_menu.func[idx++] = set_minor_host_ip_and_port; /// 备用IP与端口
	items_menu.menu.str[idx] = menu_name2_1_4;
	items_menu.func[idx++] = set_apn;
	items_menu.menu.str[idx] = menu_name2_1_5;
	items_menu.func[idx++] = set_heartbeat_to_mainstation;

	items_menu.menu.line_num = idx;
	process_items(&items_menu, info, FALSE);

	/* 通讯通道
	 * 主IP与端口
	 * 备用IP与端口
	 * APN与APN用户及密码
	 * 心跳周期
	 */
}
/*
static void set_host_param(BYTE flag, void *para, const char *info) {
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

	fparam_get_value(FPARAMID_COMM_HOST_IP_PRI, host_ip, sizeof(host_ip)); // 获得当前port
	fparam_get_value(FPARAMID_COMM_HOST_PORT_PRI, host_port, sizeof(host_port)); // 获得当前port 

	snprintf(ip_addr, sizeof(ip_addr), "%u.%u.%u.%u", host_ip[0], host_ip[1],
			host_ip[2], host_ip[3]); /// 获取当前port字符串
	ip_port = (host_port[0] << 8) + host_port[1];
	sprintf(port_buf, "%d", ip_port); /// get port_str 

	init_menu(&param_set.menu); /// 
	param_set.menu.line_num = 3;
	param_set.menu.str[0] = c_ip_address_str;
	param_set.menu.str[1] = NULL;
	param_set.menu.str[2] = c_port_str;
	param_set.group_num = 2;
	param_set.group_idx = 0;
	idx = 0;
	init_input_set(&param_set.input[idx], 2, 1, strlen(ip_addr), ip_addr, 15); /// 
	param_set.keyboard_type[idx++] = _num_keyboard;
	init_input_set(&param_set.input[idx], 3, 6, strlen(port_buf), port_buf, 5); ///
	param_set.keyboard_type[idx++] = _num_keyboard;
	process_param_set(&param_set, info);
	strcpy(ip_addr, param_set.input[0].str);
	ip = inet_addr(ip_addr);
	host_ip[0] = ip;
	host_ip[1] = ip >> 8;
	host_ip[2] = ip >> 16;
	host_ip[3] = ip >> 24;
	strcpy(port_buf, param_set.input[1].str);
	ip_port = atoi(port_buf); /// char -> int "1000" -> 1000
	host_port[0] = ip_port >> 8;
	host_port[1] = ip_port;
	fparam_set_value(FPARAMID_COMM_HOST_IP_PRI, host_ip, sizeof(host_ip)); /// set ip
	fparam_set_value(FPARAMID_COMM_HOST_PORT_PRI, host_port, sizeof(host_port)); /// set port
}
*/

static void reconstruct_network(BYTE flag, void *para, const char *info) { // no details
	if (verify_password() != 0)
		return;
	menu_ongoing(flag, para, info);
}

static void set_param(BYTE flag, void *para, const char *info) // 参数查询与设置
{
	int idx = 0;
	ITEMS_MENU items_menu;

	init_menu(&items_menu.menu);
	items_menu.cur_line = 1;
	items_menu.menu.str[idx] = menu_name2_1;
	items_menu.func[idx++] = set_con_addr;
	///items_menu.menu.str[idx] = menu_name2_2;
	///items_menu.func[idx ++] = set_host_param;
	items_menu.menu.str[idx] = menu_name1_6;
	items_menu.func[idx++] = meter_repeater_setting_new;
	items_menu.menu.str[idx] = menu_name1_7;
	items_menu.func[idx++] = import_meters_in_bunch;
	items_menu.menu.str[idx] = menu_name2_3;
	items_menu.func[idx++] = reconstruct_network;
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
	sprintf(string[2], "%s:", c_con_compiletime_str);
	sprintf(string[3], "%s", g_release_time);
	sprintf(string[4], "%s:", c_con_address_str);
	fparam_get_value(FPARAMID_CON_ADDRESS, addr, sizeof(addr));
	sprintf(string[5], "  %s",
			hex_to_str(addr_str, sizeof(addr_str), addr, 7, FALSE));
	process_menu(&menu, info); /// stop
}

static void query_network_ip(BYTE flag, void *para, const char *info) /// 查询网络IP
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

/*
 static void view_con_info(BYTE flag, void *para, const char *info)  /// 集中器信息
 {
 ITEMS_MENU items_menu;
 int idx = 0;

 init_menu(&items_menu.menu);
 items_menu.cur_line = 1;
 items_menu.menu.str[idx] = menu_name3_4;
 items_menu.func[idx++] = view_version_addr;
 items_menu.menu.str[idx] = menu_name3_2;
 items_menu.func[idx++] = query_network_ip;
 items_menu.menu.line_num = idx;
 process_items(&items_menu, info, FALSE);
 }
 */

void modem_gprs_shutdown(void);
static void restart_terminal_function(BYTE flag, void *para, const char *info) {
	//char buf[30];
	//signed char sec = 10;
	int ret;

	lcd_clean_workspace();
	lcd_show_string(2, 1, strlen(c_restarting_terminal_str),
			c_restarting_terminal_str);
	g_terminated = 1; // reboot before making sure other threads exits normally
	//led_fade();
	modem_gprs_shutdown();
	sleep(10);
	lcd_clean_workspace();

	printf("reboot\n");
	LOG_PRINTF_ALLOCATE(
			"Reboot in file:%s, in func: %s, line: %d, allocation size: %d\n",
			__FILE__, __func__, __LINE__);
	ret = system("/sbin/reboot");

	return;
}

static void restart_terminal_in(BYTE flag, void *para, const char *info) {

	ITEMS_MENU items_menu;
	int idx = 0;

	init_menu(&items_menu.menu);
	items_menu.cur_line = 1;
	items_menu.menu.str[idx] = c_ensure_str;
	items_menu.func[idx++] = restart_terminal_function;
	items_menu.menu.line_num = idx;
	process_items(&items_menu, info, FALSE);

	/*
	 char buf[30];
	 signed char sec = 10, idx;
	 unsigned char key;
	 int row;

	 lcd_clean_workspace();// clean screen
	 lcd_show_string(2, 1, strlen(c_continue_to_restart_terminal_press_enter_str), c_continue_to_restart_terminal_press_enter_str);
	 lcd_show_string(3, 1, strlen(c_cancel_restart_terminal_press_esc_str), c_cancel_restart_terminal_press_esc_str);
	 
	 while(1)
	 {
	 key = getch_timeout();
	 if(key == KEY_ENTER){
	 break; // continue;
	 }else if(key == KEY_ESC){
	 return;
	 }else if(key == KEY_NONE){
	 return;
	 }else{
	 continue;
	 }
	 }

	 lcd_clean_workspace();
	 lcd_show_string(2, 1, strlen(c_restarting_terminal_str), c_restarting_terminal_str);

	 g_terminated = 1;// 等待关掉其他线程 // 设置g_terminal // 确保其他线程在等待此线程关掉； //system("/sbin/shutdown -r 1"); // 程序保证自己的运行，需要开机启动，且只发送重启而非关机命令。
	 for(sec; sec > 0; sec--)
	 {
	 sleep(1);
	 snprintf(buf, 30, "%s%d%s", c_restart_terminal_1_str, sec, c_restart_terminal_2_str);
	 lcd_show_string( 9, 1, strlen(c_allspace_str), c_allspace_str);
	 lcd_show_string( 9, 1, strlen(buf), buf); // 实时数据显示
	 PRINTF("th_hmisys thread will exit in %d seconds later\n", sec-1);
	 }
	 lcd_clear_screen();
	 sleep(5);
	 system("/sbin/reboot");
	 
	 pthread_exit(NULL); // th_hmisys线程关掉自己
	 */

}

static void reset_day_data(BYTE flag, void *para, const char *info) {
	lcd_clean_workspace();
	lcd_show_string(2, 1, strlen(c_resetting_str), c_resetting_str);
	if (reset_fday_data() == 0) {
		lcd_clean_workspace();
		lcd_show_string(2, 1, strlen(c_success_reset_str), c_success_reset_str);

	} else {

	}
	sleep(1);
	return;
}

static void reset_month_data(BYTE flag, void *para, const char *info) {
	lcd_clean_workspace();
	lcd_show_string(2, 1, strlen(c_resetting_str), c_resetting_str);
	if (reset_fmonth_data() == 0) {
		lcd_clean_workspace();
		lcd_show_string(2, 1, strlen(c_success_reset_str), c_success_reset_str);
	} else {

	}
	sleep(1);
	return;
}

static void reset_gas_alarm_data(BYTE flag, void *para, const char *info) {
	lcd_clean_workspace();
	lcd_show_string(2, 1, strlen(c_resetting_str), c_resetting_str);
	if (reset_fgasmeteralm_data() == 0) {
		lcd_clean_workspace();
		lcd_show_string(2, 1, strlen(c_success_reset_str), c_success_reset_str);
	} else {
		/// if not success, but cannot detect failed flag
	}
	sleep(1);
	return;
}

static void meter_data_reset(BYTE flag, void *para, const char *info) {

	ITEMS_MENU items_menu;
	int idx = 0;

	init_menu(&items_menu.menu);
	items_menu.cur_line = 1;
	items_menu.menu.str[idx] = c_reset_day_data_str;
	items_menu.func[idx++] = reset_day_data;
	items_menu.menu.str[idx] = c_reset_month_data_str;
	items_menu.func[idx++] = reset_month_data;
	items_menu.menu.str[idx] = c_reset_gas_alarm_data_str;
	items_menu.func[idx++] = reset_gas_alarm_data;
	items_menu.menu.line_num = idx;
	process_items(&items_menu, info, FALSE);

	/*
	 char buff[30];
	 unsigned char key;
	 int row;

	 lcd_clean_workspace(); // clean the content
	 lcd_show_string(2, 1, strlen(c_continue_to_reset_data_press_enter_str), c_continue_to_reset_data_press_enter_str);
	 lcd_show_string(3, 1, strlen(c_cancel_reset_data_press_esc_str), c_cancel_reset_data_press_esc_str);

	 while(1)
	 {
	 //tips
	 key = getch_timeout();
	 if(key == KEY_ENTER){
	 break; // continue to reset data press enter
	 }else if(key == KEY_ESC){
	 return;// otherwise esc to exit resetting data 
	 }else if(key == KEY_NONE){
	 break;
	 }else{
	 continue;
	 }
	 } // stop to wait for command

	 lcd_clean_workspace();
	 lcd_show_string(2, 1, strlen(c_resetting_str), c_resetting_str);
	 if(reset_fmonth_data() == 0 && reset_fday_data() == 0 && reset_fgasmeteralm_data() == 0){
	 lcd_show_string(2, 1, strlen(c_allspace_str), c_allspace_str);
	 memcpy(buff, c_success_reset_month_data_str, strlen(c_success_reset_month_data_str)+1);
	 lcd_show_string(2, 1, strlen(buff), buff); // erase month data

	 lcd_show_string(3, 1, strlen(c_allspace_str), c_allspace_str);
	 memcpy(buff, c_success_reset_day_data_str, strlen(c_success_reset_day_data_str)+1);
	 lcd_show_string(3, 1, strlen(buff), buff); // erase day data

	 lcd_show_string(4, 1, strlen(c_allspace_str), c_allspace_str);
	 memcpy(buff, c_success_reset_gasalarm_data_str, strlen(c_success_reset_gasalarm_data_str)+1);
	 lcd_show_string(4, 1, strlen(buff), buff); // erase gasmeter_alarm data
	 }else{
	 // tips, reset failure
	 sleep(1);
	 return;
	 }

	 while(1){
	 //tips
	 key = getch_timeout();
	 if(key == KEY_ENTER){
	 break; // continue to reset data press enter
	 }else if(key == KEY_ESC){
	 return;// otherwise esc to exit resetting data 
	 }else if(key == KEY_NONE){
	 break;
	 }else{
	 continue;
	 }
	 }

	 return;
	 */
}

static void reset_param_data(BYTE flag, void *para, const char *info) {
	lcd_clean_workspace();
	lcd_show_string(2, 1, strlen(c_resetting_str), c_resetting_str);
	if (reset_fparam_data() == 0)
		lcd_show_string(2, 1, strlen(c_success_reset_str), c_success_reset_str);
	sleep(1);
}

static void param_data_reset(BYTE flag, void *para, const char *info) {
	//MENU menu;

	int idx = 0;
	ITEMS_MENU items_menu;

	init_menu(&items_menu.menu);
	items_menu.cur_line = 1;
	items_menu.menu.str[idx] = c_ensure_str;
	items_menu.func[idx++] = reset_param_data;
	items_menu.menu.line_num = idx;
	process_items(&items_menu, info, FALSE);

}

static void usb_update_function(BYTE flag, void *para, const char *info) {
	const char *filename = "/media/usb/gascon";
	bool file_exist_flag;
	struct stat buf;
	int ret;

	lcd_clean_workspace();

	file_exist_flag = false;
	if (stat(filename, &buf) == 0) {
		file_exist_flag = true;
	} else {
		file_exist_flag = false;
	} // get file exist flag

	if (!file_exist_flag) {
		lcd_show_string(2, 1, strlen(c_no_update_file_str),
				c_no_update_file_str);
		sleep(1);
		return;
	}
	lcd_show_string(2, 1, strlen(c_find_update_file_and_reboot_then_str),
			c_find_update_file_and_reboot_then_str);
#ifdef AM335X
	ret = system("cp /media/usb/concentrator-am335x /opt/concentrator/bin/concentrator-am335x.bak");
#elif defined IMX28
	ret = system("cp /media/usb/concentrator-am335x /opt/concentrator/bin/concentrator-am335x");
#endif

	sleep(2);
	restart_terminal_function( false, NULL, NULL); // reboot  //and //  return

	return; // return function

}

static void usb_update(BYTE flag, void *para, const char *info) {
	//MENU menu;

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
 static void usb_update(BYTE flag, void *para, const char *info)
 {
 unsigned char key;
 int row;
 bool file_exist_flag;
 struct stat buf;
 

 lcd_clean_workspace();
 lcd_show_string( 9, 1, strlen(c_allspace_str), c_allspace_str);
 lcd_show_string( 9, 1, strlen(menu_name3_1_4), menu_name3_1_4);// tips 

 while(1){

 file_exist_flag = false;
 if(stat(filename, &buf) == 0){
 file_exist_flag = true;
 }else{
 file_exist_flag = false;
 } // get file exist flag

 lcd_show_string( 2, 1, strlen(c_allspace_str), c_allspace_str); // clean
 lcd_show_string( 3, 1, strlen(c_allspace_str), c_allspace_str);
 if(file_exist_flag){
 lcd_show_string( 2, 1, strlen(c_find_update_file_and_reboot_then_str), c_find_update_file_and_reboot_then_str);
 lcd_show_string( 3, 1, strlen(c_please_press_enter_and_try_to_update), c_please_press_enter_and_try_to_update);//press_enter_to_update_tips;
 }else{
 lcd_show_string( 2, 1, strlen(c_no_update_file_str), c_no_update_file_str);
 lcd_show_string( 3, 1, strlen(c_please_press_esc_to_cancle_update_str), c_please_press_esc_to_cancle_update_str); //enter_cancle_to_return_tips;
 }// refresh tips

 key = getch_timeout();
 if(key == KEY_ESC){
 return;// return;
 }else if(key == KEY_ENTER){
 if(!file_exist_flag){
 continue;
 }else if(file_exist_flag){
 break; // break to continue
 }
 }else{
 continue;// continue to refresh tips //merging will cause vague-understanding. all for understanding and maintaincing
 }
 }

 system("cp /media/usb/gascon /home/root/gascon.bak"); // update, copy file
 restart_terminal_function( false, NULL, NULL); // reboot  //and //  return
 
 return; // return function
 }
 */

static void restart_terminal(BYTE flag, void *para, const char *info) /// 重启终端
{
	if (verify_password() != 0)
		return;

	ITEMS_MENU items_menu;
	int idx = 0;

	init_menu(&items_menu.menu);
	items_menu.cur_line = 1;

	items_menu.menu.str[idx] = menu_name3_1_1;
	items_menu.func[idx++] = restart_terminal_in;
	items_menu.menu.str[idx] = menu_name3_1_2;
	items_menu.func[idx++] = meter_data_reset;
	items_menu.menu.str[idx] = menu_name3_1_3;
	items_menu.func[idx++] = param_data_reset;
	items_menu.menu.str[idx] = menu_name3_1_4;
	items_menu.func[idx++] = usb_update;

	items_menu.menu.line_num = idx;
	process_items(&items_menu, info, FALSE);
}

/*
 static void liquid_switch(BYTE flag, void *para, const char *info)
 {
 menu_ongoing(flag, para, info);
 }
 */

static void terminal_id(BYTE flag, void *para, const char *info) /// show terminal id
{

	MENU menu;
	BYTE addr[7];
	char string[2][MAX_SCREEN_COL + 1]; /// MAX_SCREEN_COL + 1
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
	process_menu(&menu, info); /// stop
	///menu_ongoing(flag, para, info);

}

/*
 static void terminal_version(BYTE flag, void *para, const char *info)
 {
 menu_ongoing(flag, para, info);
 }
 */

bool if_time_str_is_valid(const char *time_str) {
	char buf[7];
	unsigned int i = 0;

	memcpy(buf, time_str, 7);
	i = atoi(buf);
	if (i / 10000 >= 24 || i / 10000 < 0)
		return false;
	if (i % 10000 / 100 >= 60)
		return false;
	if (i % 100 >= 60)
		return false;

	return true;
}

static void set_terminal_time(BYTE flag, void *para, const char *info) {

	if (verify_password() != 0)
		return;

	//PARAM_SET param_set;
	char terminal_date[8 + 1];
	char terminal_time[6 + 1];
	//int idx, i;
	time_t tt;
	struct tm tm;
	struct timeval tv;
	//unsigned char key;

	memset(terminal_date, 0, sizeof(terminal_date));
	memset(terminal_time, 0, sizeof(terminal_time));

	time(&tt);
	localtime_r(&tt, &tm);
	snprintf(terminal_date, sizeof(terminal_date), "%04d%02d%02d",
			tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
	snprintf(terminal_time, sizeof(terminal_time), "%02d%02d%02d", tm.tm_hour,
			tm.tm_min, tm.tm_sec);

	while (1) {
		lcd_clean_workspace();
		lcd_show_string(2, 1, strlen(c_date_re_str), c_date_re_str);
		lcd_show_string(3, 4, strlen(terminal_date), terminal_date);
		lcd_show_string(4, 1, strlen(c_time_str), c_time_str);
		lcd_show_string(5, 4, strlen(terminal_time), terminal_time);
		lcd_show_string(9, 1, strlen(c_allspace_str), c_allspace_str);
		lcd_show_string(9, 1, strlen(menu_name3_5), menu_name3_5); // 终端时间设置

		if (input_string(3, "   ", fmt_num, terminal_date,
				strlen(terminal_date))) {
			lcd_show_string(3, 4, strlen(terminal_date), terminal_date);
			// valid continue;	
		} else {
			break;
		}

		if (input_string(5, "   ", fmt_num, terminal_time,
				strlen(terminal_time))) {
			lcd_show_string(5, 4, strlen(terminal_time), terminal_time);
			// valid continue;
		} else {
			break;
		}

		// TODO: not certain with time invalidness
		if (true/*if_date_str_is_valid(terminal_date, 2)*/) {
			// valid continue;
		} else {
			// invalid continue; // tips
			lcd_show_string(9, 1, strlen(c_allspace_str), c_allspace_str);
			lcd_show_string(9, 1, strlen(c_year_month_day_invalid_str),
					c_year_month_day_invalid_str);
			sleep(1);
			break;
		}

		if (1/*if_time_str_is_valid(terminal_time)*/) {
			// valid continue
		} else {
			// invalid continue
			lcd_show_string(9, 1, strlen(c_allspace_str), c_allspace_str);
			lcd_show_string(9, 1, strlen(c_time_invalid_str),
					c_time_invalid_str); // 时间不正确
			sleep(1);
			break;
		}

		tm.tm_isdst = -1;
		tm.tm_year = atoi(terminal_date) / 10000 - 1900;
		tm.tm_mon = atoi(terminal_date) % 10000 / 100 - 1;
		tm.tm_mday = atoi(terminal_date) % 100;
		tm.tm_hour = atoi(terminal_time) / 10000;
		tm.tm_min = atoi(terminal_time) % 10000 / 100;
		tm.tm_sec = atoi(terminal_time) % 100;
		tv.tv_sec = mktime(&tm);
		tv.tv_usec = 0;
		settimeofday(&tv, NULL); // ptl_gasup_fn_2015();
		set_rtc();

		lcd_show_string(9, 1, strlen(c_allspace_str), c_allspace_str);
		lcd_show_string(9, 1, strlen(c_time_setting_sucess_str),
				c_time_setting_sucess_str); // 时间不正确
		sleep(1);

		continue;
	}
	return;
}

static void password_setting(BYTE flag, void *para, const char *info) {

	if (verify_password() != 0)
		return;

	PARAM_SET param_set;
	BYTE password_byte[3];
	char buf_s[7];
	int idx; // DWORD buf;

	memset(password_byte, 0, sizeof(password_byte));
	///memcpy(password_str, "000000", 7); // see the password
	memset(&param_set, 0, sizeof(param_set));

	if (!fparam_get_value(FPARAMID_CON_VERIFY_PASSWD, password_byte,
			sizeof(password_byte)))
		return; // failed
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
	process_param_set(&param_set, info);  // 密码只能用数字

	strcpy(buf_s, param_set.input[0].str); /// 换input
	if (strlen(buf_s) < 6)
		return;
	password_byte[0] = bin_to_bcd(atoi(buf_s) / 10000);
	password_byte[1] = bin_to_bcd(atoi(buf_s) % 10000 / 100);
	password_byte[2] = bin_to_bcd(atoi(buf_s) % 100); // 123456(buf_s) -> 0x12, 0x34, 0x56(password_byte)

	if (!fparam_set_value(FPARAMID_CON_VERIFY_PASSWD, password_byte,
			sizeof(password_byte))) {
		printf("set password failed\n");
		////return;
	} else {
		printf("set password ok\n");
	}

	return;
}

/*
static void local_module_status(BYTE flag, void *para, const char *info) {
	menu_ongoing(flag, para, info);
}

static void signal_intensity_and_battery_status(BYTE flag, void *para,
		const char *info) {
	menu_ongoing(flag, para, info);
}

*/

//#include "gpio.h"
/*
static void modem_soft_reset_hmisys(BYTE flag, void *para, const char *info) {
	///menu_ongoing(flag, para, info);
	modem_soft_reset();
}
*/

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
	///sprintf(string[i++], "   %dMb, %dKb", j/1024, j%1024);
	sprintf(string[i++], (j / (1024 * 1024)) ? "   %8.2f MB" : "   %8.2f KB",
			(j / (1024 * 1024)) ? (j / (1024.0 * 1024)) : (j / 1024.0));
	process_menu(&menu, info); /// stop
}

static void terminal_management(BYTE flag, void *para, const char *info) {
	ITEMS_MENU items_menu;
	int idx = 0;

	init_menu(&items_menu.menu);
	items_menu.cur_line = 1;
	items_menu.menu.str[idx] = menu_name3_1;
	items_menu.func[idx++] = restart_terminal;
	//items_menu.menu.str[idx] = menu_name3_2;
	//items_menu.func[idx++] = liquid_switch;
	items_menu.menu.str[idx] = menu_name3_3;
	items_menu.func[idx++] = terminal_id;
	items_menu.menu.str[idx] = menu_name3_4;
	//items_menu.func[idx++] = view_con_info;
	items_menu.func[idx++] = view_version_addr;
	items_menu.menu.str[idx] = menu_name3_10;
	items_menu.func[idx++] = query_network_ip;
	//items_menu.func[idx++] = view_version_addr;
	items_menu.menu.str[idx] = menu_name3_5;
	items_menu.func[idx++] = set_terminal_time;
	items_menu.menu.str[idx] = menu_name3_6;
	items_menu.func[idx++] = password_setting;
	///items_menu.menu.str[idx] = "modem soft reset";
	///items_menu.func[idx++] = modem_soft_reset_hmisys;
	//items_menu.menu.str[idx] = menu_name3_7;
	//items_menu.func[idx++] = channel_setting; // a duplicate
	//items_menu.menu.str[idx] = menu_name3_8;
	//items_menu.func[idx++] = local_module_status;
	//items_menu.menu.str[idx] = menu_name3_9;
	//items_menu.func[idx++] = signal_intensity_and_battery_status; // we just know items part details
	items_menu.menu.str[idx] = modem_flux_sum_menu;
	items_menu.func[idx++] = get_modem_flux_in_sum;

	items_menu.menu.line_num = idx;
	process_items(&items_menu, info, FALSE);
}

void main_menu_init(ITEMS_MENU *items_menu) {
	int idx = 0;

	///printf("utf8中文输入与打印\n"); /// can not print normal Chinese character
	init_menu(&items_menu->menu);
	items_menu->cur_line = 1;
	items_menu->menu.str[idx] = menu_name1; /// 数据查询
	items_menu->func[idx++] = query_data;
	items_menu->menu.str[idx] = menu_name2; /// 参数查询与设置
	items_menu->func[idx++] = set_param;
	items_menu->menu.str[idx] = menu_name3; /// 终端管理与维护
	items_menu->func[idx++] = terminal_management;
	items_menu->menu.line_num = idx;
}

/*
 void get_valid_day_block_from_file_test_out(void)
 {
 get_valid_day_block_from_file_test(TRUE, NULL, NULL);
 }
 */

int verify_password(void) {
	int tries_times;
	BYTE password_byte[3];
	char buf[7];
	char buf_save[7];

	memcpy(buf, "000000", sizeof(buf));
	tries_times = 3;

	while (1) {
		lcd_clean_workspace();
		lcd_show_string(2, 1, strlen(c_please_input_password_str),
				c_please_input_password_str);
		lcd_show_string(9, 1, strlen(c_allspace_str), c_allspace_str);
		lcd_show_string(9, 1, strlen(c_password_setting_str),
				c_password_setting_str); // 请输入密码
		if (input_string(3, "   ", fmt_num, buf, strlen(buf))) {
			lcd_show_string(3, 4, strlen(buf), buf); // valid continue the verification
		} else {
			return -1; // not secure if just break, here should return -1;
		}

		fparam_get_value(FPARAMID_CON_VERIFY_PASSWD, password_byte,
				sizeof(password_byte));
		snprintf(buf_save, 7, "%02d%02d%02d", bcd_to_bin(password_byte[0]),
				bcd_to_bin(password_byte[1]), bcd_to_bin(password_byte[2]));
		PRINTF("SECURITY! PASSWORD SAVED IN: %s\n", buf_save); /// show password for test
		if (memcmp(buf, buf_save, 6) == 0)
			return 0; // success in verifying
		tries_times--;
		if (!(tries_times > 0))
			return -1; // tries times is out of use.
	}
}

/// write into log
void test_process(void) {
	///
	return;
}

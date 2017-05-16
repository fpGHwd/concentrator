/*
 * language.c
 *
 *  Created on: 2015-8-11
 *      Author: Johnnyzhang
 */

#include "language.h"

const char *c_numeric_keyboard_str[] = {
	" 退出 删除 确认 ",
	"  1    2    3   ",
	"  4    5    6   ",
	"  7    8    9   ",
	"  *    0    #   ",
	"  ,    .   字母 ",
};

const char *c_lower_letter_keyboard_str[] = {
	" 退出 删除 确认 ",
	" a b c d e f g  ",
	" h i j k l m n  ",
	" o p q r s t u  ",
	" v w x y z 空格 ",
	" 大写 符号 数字 ",
};

const char *c_upper_letter_keyboard_str[] = {
	" 退出 删除 确认 ",
	" A B C D E F G  ",
	" H I J K L M N  ",
	" O P Q R S T U  ",
	" V W X Y Z 空格 ",
	" 小写 符号 数字 ",
};

const char *c_direction_keyboard_str[] = {
	"                ",
	"       ↑       ",
	"     ←  →     ",
	"       ↓       ",
	"                ",
	"                ",
};

const char *c_arrow_str[] = {"↑", "↓", "←", "→"};

const char *c_year_str = "年";
const char *c_month_str = "月";
const char *c_weeks_chinese_str = "日 一 二 三 四 五 六";
const char *c_calendar_select_str = "日期选择";

const char *c_info_main_menu_str = "主菜单";
const char *c_enterinto_menu_str = "进入菜单";
const char *c_exit_menu_str = "退出菜单";

const char *menu_name1 = "数据查询";
const char *menu_name1_1 = "抄表统计数据";
const char *menu_name1_2 = "告警事件";
const char *menu_name1_3 = "历史数据显示";
const char *menu_name1_3_1 = "月数据查询";
const char *menu_name1_3_2 = "日数据查询";
const char *menu_name1_4 = "实时数据显示";
const char *menu_name1_5 = "集中抄表";
const char *menu_name1_6 = "中继设置";
const char *menu_name1_7 = "表批量导入";
const char *menu_name2 = "参数查询与设置";
const char *menu_name2_1 = "通讯通道查询与设置";
const char *menu_name2_1_1 = "通讯通道设置";
const char *menu_name2_1_2 = "主IP与端口";
const char *menu_name2_1_3 = "备用IP与端口";
const char *menu_name2_1_4 = "APN与APN用户及密码";
const char *menu_name2_1_5 = "心跳周期";
const char *menu_name2_2 = "终端IP及端口设置";
const char *menu_name2_3 = "重新组网";
const char *menu_name3 = "终端管理与维护";
const char *menu_name3_1 = "重启终端";
const char *menu_name3_1_1 = "重启终端";
const char *menu_name3_1_2 = "数据区复位";
const char *menu_name3_1_3 = "参数数据区复位";
const char *menu_name3_1_4 = "USB程序升级";
const char *menu_name3_2 = "液晶调节";
const char *menu_name3_3 = "终端编号";
const char *menu_name3_4 = "终端版本信息";
const char *menu_name3_5 = "终端时间设置";
const char *menu_name3_6 = "界面密码设置";
const char *menu_name3_7 = "通道设置";
const char *menu_name3_8 = "本地模块状态";
const char *menu_name3_9 = "信号强度和电池电量";
const char *menu_name3_10 = "终端IP";

const char *c_ethernet_ip_str = "以太网IP";
const char *c_con_model_str = "集中器型号";
const char *c_con_version_str = "集中器版本";
const char *c_con_compiletime_str = "编译时间";
const char *c_con_address_str = "集中器通讯地址";
const char *c_login_ok_str = "注册成功!";
const char *c_ip_address_str = "IP地址:";
const char *c_port_str = "端口:";
const char *c_allspace_str = "                    ";
const char *c_menu_ongoing_str = "正在开发..."; 
const char *c_date_str = "日时标:";
const char *c_meter_sum = "表档案总数:";
const char *c_day_success = "日数据成功数:";
const char *c_month_success = "月数据成功数:";
const char *c_time_invalid = "时间不正确";
const char *c_alarm_name_str = "告警名称:";
const char *c_electricity_invalid_str = "停电事件告警";
const char *c_alarm_meter_id_str = "测量点表号:";
const char *c_alarm_occur_time_str = "发生时间:";
const char *c_alarm_last_record_str1 = "最近第";
const char *c_alarm_last_record_str2 = "条记录";
const char *c_alarm_already_get_the_last_record_str = "已经是最后一条记录";
const char *c_alarm_already_get_the_first_record_str = "已经是第一条记录";
const char *c_history_meter_id_str = "表号:";
const char *c_history_month_date_str = "月时标:";
const char *c_history_day_str = "日时标:";
const char *c_not_find_meter_in_concentrator = "没有表的信息";
const char *c_reading_meter_str = "正在抄表..."; // 7/20
const char *c_complete_str = "完成!";
const char *c_reading_success_str = "成功数:"; // 2/20
const char *c_reading_failure_str = "失败数:"; // 4/20
const char *c_current_reading_meter_str = "表号:"; // 2035301550086
const char *c_current_collector_str = "采集器:";
const char *c_current_repeater_str = "中继:";
const char *c_estmated_time_str = "预计时长:"; // 
const char *c_current_flux_display_value_str = "当前流量示值:";
const char *c_balance_flux_amount_str = "结算日流量累计值:";
const char *c_valve_status_str = "阀门状态:";
const char *c_valve_status_open_str = "打开";
const char *c_valve_status_closed_str = "关闭";
const char *c_apn_str = "APN:";
const char *c_apn_id_str = "APN用户名:";
const char *c_apn_password_str = "APN用户密码:";
const char *c_heart_beat_min_str = "心跳周期(0~65525s):";
const char *c_time_is_out_of_limited_str = "时间超前";
const char *c_year_month_invalid_str = "不正确的日期";
const char *c_year_month_day_invalid_str = "日期不正确";
const char *c_daily_flux_str = "日流量:";
const char *c_daily_initiate_flux_str = "初  值:";
const char *c_next_day_initiate_flux_str = "末  值:";
const char *c_month_initiate_flux_str = "初  值:";
const char *c_next_month_initiate_flux_str = "末  值:";
const char *c_month_end_flux_str = "末流量:"; 
const char *c_month_flux_str = "月流量:";
const char *c_current_flux_str = "当前值:";
const char *c_no_data_found = "无该月数据";
const char *menu_name_test = "测试";
const char *c_reading_meter_time_str = "时间:";
const char *c_next_day_flux_str = "后一日初始流量:";
const char *c_invalid_meter_id_str = "无此表";
const char *c_invalid_year_month_str = "时间不正确";
const char *c_no_data_str = "无数据";
const char *c_no_day_idx_found_str = "无该天数据";
const char *c_no_next_day_data_str = "无下一日数据";
const char *c_date_re_str = "日期:";
const char *c_time_str = "时间:";
const char *c_time_invalid_str = "时间不正确";
const char *c_time_setting_sucess_str = "时间设置成功";
const char *c_restart_terminal_str = "系统将在10秒后重启";
const char *c_restart_terminal_1_str = "系统将在";
const char *c_restart_terminal_2_str = "秒后重启";
const char *c_restart_terminal_3_str = "系统重启中";
const char *c_restart_terminal_4_str = ".";
const char *c_restart_terminal_5_str = "..";
const char *c_restart_terminal_6_str = "...";
const char *c_password_verify_str = "新密码:";
const char *c_please_input_password_str = "请输入验证密码:";
const char *c_password_setting_str = "密码验证";
const char *c_usb_is_plugged_in_str = "USB已插入";
const char *c_usb_is_not_plugged_in_str = "USB未插入";
const char *c_no_update_file_str = "未找到更新文件GASCON";
const char *c_find_update_file_and_reboot_then_str = "找到更新文件GASCON";
const char *c_please_press_enter_and_try_to_update = "请按ENTER键升级";
const char *c_please_press_esc_to_cancle_update_str = "请按ESC退出升级";
const char *c_check_update_condition_str = "请检查升级条件";
const char *c_meter_information_erase_success_str = "清除表数据成功";
const char *c_meter_information_erase_failure_str = "清除表数据失败";
const char *c_continue_to_reset_data_press_enter_str = "复位数据按ENTER";
const char *c_cancel_reset_data_press_esc_str = "取消复位数据按ESC";
const char *c_reset_month_data_now_str = "正在复位月数据";
const char *c_success_reset_month_data_str = "月数据已复位";
const char *c_reset_day_data_now_str = "正在复位日数据";
const char *c_success_reset_day_data_str = "日数据已复位";
const char *c_success_reset_gasalarm_data_str = "表告警数据已复位";
const char *c_resetting_str = "复位中...";
const char *c_continue_to_restart_terminal_press_enter_str = "确认重启按ENTER";
const char *c_cancel_restart_terminal_press_esc_str = "取消重启按ESC";
const char *c_restarting_terminal_str = "重启中...";
const char *c_ensure_str = "确认";
const char *c_cancle_str = "取消";
const char *c_success_reset_str = "复位成功";
const char *c_reset_day_data_str = "日数据复位";
const char *c_reset_month_data_str = "月数据复位";
const char *c_reset_gas_alarm_data_str = "表告警数据复位";
const char *c_rthc_tech_str = "瑞泰恒创科技";
const char *c_gasmeter_alarm_str = "表告警事件";
const char *c_concentrator_alarm_str = "集中器告警事件";
const char *c_repeater_query_str = "中继查询";
const char *c_repeater_setting_str = "中继设置";
const char *c_repeater_str = "中继:";
const char *c_enable_str = "使能:";
const char *c_meters_file_not_exist_str = "未找到文件METERS.TXT";
const char *c_meters_file_importing_str = "导入中...";
const char *c_meters_import_success_str = "导入成功!";
const char *modem_flux_sum_str = "GPRS/CDMA总流量:";
const char *modem_flux_sum_menu = "流量查询";
const char *c_invalid_value = "无效值！";


// 数据库中的数据调试
const char *c_get_valid_meter_str = "表号及状态";
const char *c_get_valid_day_block_from_variable_str = "变量中的有效天";
const char *c_get_valid_day_block_from_file_str = "文件中的有效天";
const char *c_write_variable_into_file_str = "把变量写入文件";
const char *c_update_day_block_from_varieble_into_file_str = "update day_block in file";



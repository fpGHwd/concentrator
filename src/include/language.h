/*
 * language.h
 *
 *  Created on: 2015-8-11
 *      Author: Johnnyzhang
 */

#ifndef LANGUAGE_H_
#define LANGUAGE_H_

//#include "typedef.h"

extern const char *c_numeric_keyboard_str[];

extern const char *c_lower_letter_keyboard_str[];

extern const char *c_upper_letter_keyboard_str[];

extern const char *c_direction_keyboard_str[];

extern const char *c_arrow_str[];

extern const char *c_year_str;
extern const char *c_month_str;
extern const char *c_weeks_chinese_str;
extern const char *c_calendar_select_str;

extern const char *c_info_main_menu_str;
extern const char *c_enterinto_menu_str;
extern const char *c_exit_menu_str;

extern const char *menu_name1;
extern const char *menu_name1_1;
extern const char *menu_name1_2;
extern const char *menu_name1_3;
extern const char *menu_name1_3_1;
extern const char *menu_name1_3_2;
extern const char *menu_name1_4;
extern const char *menu_name1_5;
extern const char *menu_name1_6;
extern const char *menu_name1_7;
extern const char *menu_name2;
extern const char *menu_name2_1;
extern const char *menu_name2_1_1;
extern const char *menu_name2_1_2;
extern const char *menu_name2_1_3;
extern const char *menu_name2_1_4;
extern const char *menu_name2_1_5;
extern const char *menu_name2_2;
extern const char *menu_name2_3;
extern const char *menu_name3;
extern const char *menu_name3_1;
extern const char *menu_name3_1_1;
extern const char *menu_name3_1_2;
extern const char *menu_name3_1_3;
extern const char *menu_name3_1_4;
extern const char *menu_name3_2;
extern const char *menu_name3_3;
extern const char *menu_name3_4;
extern const char *menu_name3_5;
extern const char *menu_name3_6;
extern const char *menu_name3_7;
extern const char *menu_name3_8;
extern const char *menu_name3_9;
extern const char *menu_name3_10;

extern const char *c_ethernet_ip_str;
extern const char *c_con_model_str;
extern const char *c_con_version_str;
extern const char *c_con_compiletime_str;
extern const char *c_con_address_str;
extern const char *c_login_ok_str;
extern const char *c_ip_address_str;
extern const char *c_port_str;
extern const char *c_allspace_str;
extern const char *c_menu_ongoing_str;
extern const char *c_date_str;
extern const char *c_meter_sum;
extern const char *c_day_success;
extern const char *c_month_success;
extern const char *c_time_invalid;
extern const char *c_alarm_name_str;
extern const char *c_electricity_invalid_str;
extern const char *c_alarm_meter_id_str;
extern const char *c_reading_success_str;
extern const char *c_reading_failure_str;
extern const char *c_current_reading_meter_str;
extern const char *c_current_collector_str;
extern const char *c_current_repeater_str;
extern const char *c_estmated_time_str;
extern const char *c_alarm_occur_time_str;
extern const char *c_alarm_last_record_str1;
extern const char *c_alarm_last_record_str2;
extern const char *c_alarm_already_get_the_last_record_str;
extern const char *c_alarm_already_get_the_first_record_str;
extern const char *c_history_meter_id_str;
extern const char *c_history_month_date_str;
extern const char *c_history_day_str;
extern const char *c_not_find_meter_in_concentrator;
extern const char *c_reading_meter_str;
extern const char *c_complete_str;
extern const char *c_current_flux_display_value_str;
extern const char *c_balance_flux_amount_str;
extern const char *c_valve_status_str;
extern const char *c_valve_status_open_str;
extern const char *c_valve_status_closed_str;
extern const char *c_apn_str;
extern const char *c_apn_id_str;
extern const char *c_apn_password_str;
extern const char *c_heart_beat_min_str;
//extern const char *c_heart_beat_tips_str;
extern const char *c_time_is_out_of_limited_str;
extern const char *c_year_month_invalid_str;
extern const char *c_year_month_day_invalid_str;
extern const char *c_daily_flux_str;
extern const char *c_daily_initiate_flux_str;
extern const char *c_next_day_initiate_flux_str;
extern const char *c_month_initiate_flux_str;
extern const char *c_next_month_initiate_flux_str;
extern const char *c_month_end_flux_str;
extern const char *c_month_flux_str;
extern const char *c_current_flux_str;
extern const char *c_no_data_found;
extern const char *menu_name_test;
extern const char *c_reading_meter_time_str;
extern const char *c_next_day_flux_str;
extern const char *c_invalid_meter_id_str;
extern const char *c_invalid_year_month_str;
extern const char *c_no_data_str;
extern const char *c_no_day_idx_found_str;
extern const char *c_no_next_day_data_str;
extern const char *c_date_re_str;
extern const char *c_time_str;
extern const char *c_time_invalid_str;
extern const char *c_time_setting_sucess_str;
extern const char *c_restart_terminal_str;
extern const char *c_restart_terminal_1_str;
extern const char *c_restart_terminal_2_str;
extern const char *c_restart_terminal_3_str;
extern const char *c_restart_terminal_4_str;
extern const char *c_restart_terminal_5_str;
extern const char *c_restart_terminal_6_str;
extern const char *c_password_verify_str;
extern const char *c_please_input_password_str;
extern const char *c_password_setting_str;
extern const char *c_usb_is_plugged_in_str;
extern const char *c_usb_is_not_plugged_in_str;
extern const char *c_no_update_file_str;
extern const char *c_find_update_file_and_reboot_then_str;
extern const char *c_please_press_enter_and_try_to_update;
extern const char *c_check_update_condition_str;
extern const char *c_please_press_esc_to_cancle_update_str;
extern const char *c_meter_information_erase_success_str;
extern const char *c_meter_information_erase_failure_str;
extern const char *c_continue_to_reset_data_press_enter_str;
extern const char *c_cancel_reset_data_press_esc_str;
extern const char *c_reset_month_data_now_str;
extern const char *c_success_reset_month_data_str;
extern const char *c_reset_day_data_now_str;
extern const char *c_success_reset_day_data_str;
extern const char *c_success_reset_gasalarm_data_str;
extern const char *c_resetting_str;
extern const char *c_continue_to_restart_terminal_press_enter_str;
extern const char *c_cancel_restart_terminal_press_esc_str;
extern const char *c_restarting_terminal_str;
extern const char *c_ensure_str;
extern const char *c_cancle_str;
extern const char *c_success_reset_str;
extern const char *c_reset_data_str;
extern const char *c_reset_day_data_str;
extern const char *c_reset_month_data_str;
extern const char *c_reset_gas_alarm_data_str;
extern const char *c_reset_data_suc_str;
extern const char *c_reset_day_data_suc_str;
extern const char *c_reset_month_data_suc_str;
extern const char *c_reset_gas_alarm_data_suc_str;
extern const char *c_rthc_tech_str;
extern const char *c_cqrl_str;
extern const char *c_gasmeter_alarm_str;
extern const char *c_concentrator_alarm_str;
extern const char *c_repeater_query_str;
extern const char *c_repeater_setting_str;
extern const char *c_repeater_str;
extern const char *c_enable_str;
extern const char *c_meters_file_not_exist_str;
extern const char *c_meters_file_importing_str;
extern const char *c_meters_import_success_str;
extern const char *modem_flux_sum_str;
extern const char *modem_flux_sum_menu;
extern const char *c_invalid_value;

extern const char *c_get_valid_meter_str;
extern const char *c_get_valid_day_block_from_variable_str;
extern const char *c_get_valid_day_block_from_file_str;
extern const char *c_write_variable_into_file_str;
extern const char *c_update_day_block_from_varieble_into_file_str;



extern const char *date_test;
extern const char *invalid_meter;
extern const char *read_no_data;
extern const char *inner_error;
extern const char *please_input_meter_id;
extern const char *read_meter_ok;
extern const char *please_select_date;
extern const char *meter_sum;
extern const char *no_meter;
extern const char *no_data;
extern const char *day_data;
extern const char *no_date_index;
extern const char *date_string;
extern const char *time_string;
extern const char *none;
extern const char *month_data;
extern const char *current_data;
extern const char *set_successfully;
extern const char *set_unsuccessfully;
extern const char *meter_info;

extern const char *clearing;
extern const char *c_reset_meters_str;
extern const char *clear_successfully;
extern const char *clear_unsuccessfully;

extern const char *c_abort_str;

#endif /* LANGUAGE_H_ */

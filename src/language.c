/*
 * language.c
 *
 *  Created on: 2015-8-11
 *      Author: Johnnyzhang
 */

#include "language.h"

const char *c_numeric_keyboard_str[] = {
	" �˳� ɾ�� ȷ�� ",
	"  1    2    3   ",
	"  4    5    6   ",
	"  7    8    9   ",
	"  *    0    #   ",
	"  ,    .   ��ĸ ",
};

const char *c_lower_letter_keyboard_str[] = {
	" �˳� ɾ�� ȷ�� ",
	" a b c d e f g  ",
	" h i j k l m n  ",
	" o p q r s t u  ",
	" v w x y z �ո� ",
	" ��д ���� ���� ",
};

const char *c_upper_letter_keyboard_str[] = {
	" �˳� ɾ�� ȷ�� ",
	" A B C D E F G  ",
	" H I J K L M N  ",
	" O P Q R S T U  ",
	" V W X Y Z �ո� ",
	" Сд ���� ���� ",
};

const char *c_direction_keyboard_str[] = {
	"                ",
	"       ��       ",
	"     ��  ��     ",
	"       ��       ",
	"                ",
	"                ",
};

const char *c_arrow_str[] = {"��", "��", "��", "��"};

const char *c_year_str = "��";
const char *c_month_str = "��";
const char *c_weeks_chinese_str = "�� һ �� �� �� �� ��";
const char *c_calendar_select_str = "����ѡ��";

const char *c_info_main_menu_str = "���˵�";
const char *c_enterinto_menu_str = "����˵�";
const char *c_exit_menu_str = "�˳��˵�";

const char *menu_name1 = "���ݲ�ѯ";
const char *menu_name1_1 = "����ͳ������";
const char *menu_name1_2 = "�澯�¼�";
const char *menu_name1_3 = "��ʷ������ʾ";
const char *menu_name1_3_1 = "�����ݲ�ѯ";
const char *menu_name1_3_2 = "�����ݲ�ѯ";
const char *menu_name1_4 = "ʵʱ������ʾ";
const char *menu_name1_5 = "���г���";
const char *menu_name1_6 = "�м�����";
const char *menu_name1_7 = "����������";
const char *menu_name2 = "������ѯ������";
const char *menu_name2_1 = "ͨѶͨ����ѯ������";
const char *menu_name2_1_1 = "ͨѶͨ������";
const char *menu_name2_1_2 = "��IP��˿�";
const char *menu_name2_1_3 = "����IP��˿�";
const char *menu_name2_1_4 = "APN��APN�û�������";
const char *menu_name2_1_5 = "��������";
const char *menu_name2_2 = "�ն�IP���˿�����";
const char *menu_name2_3 = "��������";
const char *menu_name3 = "�ն˹�����ά��";
const char *menu_name3_1 = "�����ն�";
const char *menu_name3_1_1 = "�����ն�";
const char *menu_name3_1_2 = "��������λ";
const char *menu_name3_1_3 = "������������λ";
const char *menu_name3_1_4 = "USB��������";
const char *menu_name3_2 = "Һ������";
const char *menu_name3_3 = "�ն˱��";
const char *menu_name3_4 = "�ն˰汾��Ϣ";
const char *menu_name3_5 = "�ն�ʱ������";
const char *menu_name3_6 = "������������";
const char *menu_name3_7 = "ͨ������";
const char *menu_name3_8 = "����ģ��״̬";
const char *menu_name3_9 = "�ź�ǿ�Ⱥ͵�ص���";
const char *menu_name3_10 = "�ն�IP";

const char *c_ethernet_ip_str = "��̫��IP";
const char *c_con_model_str = "�������ͺ�";
const char *c_con_version_str = "�������汾";
const char *c_con_compiletime_str = "����ʱ��";
const char *c_con_address_str = "������ͨѶ��ַ";
const char *c_login_ok_str = "ע��ɹ�!";
const char *c_ip_address_str = "IP��ַ:";
const char *c_port_str = "�˿�:";
const char *c_allspace_str = "                    ";
const char *c_menu_ongoing_str = "���ڿ���..."; 
const char *c_date_str = "��ʱ��:";
const char *c_meter_sum = "��������:";
const char *c_day_success = "�����ݳɹ���:";
const char *c_month_success = "�����ݳɹ���:";
const char *c_time_invalid = "ʱ�䲻��ȷ";
const char *c_alarm_name_str = "�澯����:";
const char *c_electricity_invalid_str = "ͣ���¼��澯";
const char *c_alarm_meter_id_str = "��������:";
const char *c_alarm_occur_time_str = "����ʱ��:";
const char *c_alarm_last_record_str1 = "�����";
const char *c_alarm_last_record_str2 = "����¼";
const char *c_alarm_already_get_the_last_record_str = "�Ѿ������һ����¼";
const char *c_alarm_already_get_the_first_record_str = "�Ѿ��ǵ�һ����¼";
const char *c_history_meter_id_str = "���:";
const char *c_history_month_date_str = "��ʱ��:";
const char *c_history_day_str = "��ʱ��:";
const char *c_not_find_meter_in_concentrator = "û�б����Ϣ";
const char *c_reading_meter_str = "���ڳ���..."; // 7/20
const char *c_complete_str = "���!";
const char *c_reading_success_str = "�ɹ���:"; // 2/20
const char *c_reading_failure_str = "ʧ����:"; // 4/20
const char *c_current_reading_meter_str = "���:"; // 2035301550086
const char *c_current_collector_str = "�ɼ���:";
const char *c_current_repeater_str = "�м�:";
const char *c_estmated_time_str = "Ԥ��ʱ��:"; // 
const char *c_current_flux_display_value_str = "��ǰ����ʾֵ:";
const char *c_balance_flux_amount_str = "�����������ۼ�ֵ:";
const char *c_valve_status_str = "����״̬:";
const char *c_valve_status_open_str = "��";
const char *c_valve_status_closed_str = "�ر�";
const char *c_apn_str = "APN:";
const char *c_apn_id_str = "APN�û���:";
const char *c_apn_password_str = "APN�û�����:";
const char *c_heart_beat_min_str = "��������(0~65525s):";
const char *c_time_is_out_of_limited_str = "ʱ�䳬ǰ";
const char *c_year_month_invalid_str = "����ȷ������";
const char *c_year_month_day_invalid_str = "���ڲ���ȷ";
const char *c_daily_flux_str = "������:";
const char *c_daily_initiate_flux_str = "��  ֵ:";
const char *c_next_day_initiate_flux_str = "ĩ  ֵ:";
const char *c_month_initiate_flux_str = "��  ֵ:";
const char *c_next_month_initiate_flux_str = "ĩ  ֵ:";
const char *c_month_end_flux_str = "ĩ����:"; 
const char *c_month_flux_str = "������:";
const char *c_current_flux_str = "��ǰֵ:";
const char *c_no_data_found = "�޸�������";
const char *menu_name_test = "����";
const char *c_reading_meter_time_str = "ʱ��:";
const char *c_next_day_flux_str = "��һ�ճ�ʼ����:";
const char *c_invalid_meter_id_str = "�޴˱�";
const char *c_invalid_year_month_str = "ʱ�䲻��ȷ";
const char *c_no_data_str = "������";
const char *c_no_day_idx_found_str = "�޸�������";
const char *c_no_next_day_data_str = "����һ������";
const char *c_date_re_str = "����:";
const char *c_time_str = "ʱ��:";
const char *c_time_invalid_str = "ʱ�䲻��ȷ";
const char *c_time_setting_sucess_str = "ʱ�����óɹ�";
const char *c_restart_terminal_str = "ϵͳ����10�������";
const char *c_restart_terminal_1_str = "ϵͳ����";
const char *c_restart_terminal_2_str = "�������";
const char *c_restart_terminal_3_str = "ϵͳ������";
const char *c_restart_terminal_4_str = ".";
const char *c_restart_terminal_5_str = "..";
const char *c_restart_terminal_6_str = "...";
const char *c_password_verify_str = "������:";
const char *c_please_input_password_str = "��������֤����:";
const char *c_password_setting_str = "������֤";
const char *c_usb_is_plugged_in_str = "USB�Ѳ���";
const char *c_usb_is_not_plugged_in_str = "USBδ����";
const char *c_no_update_file_str = "δ�ҵ������ļ�GASCON";
const char *c_find_update_file_and_reboot_then_str = "�ҵ������ļ�GASCON";
const char *c_please_press_enter_and_try_to_update = "�밴ENTER������";
const char *c_please_press_esc_to_cancle_update_str = "�밴ESC�˳�����";
const char *c_check_update_condition_str = "������������";
const char *c_meter_information_erase_success_str = "��������ݳɹ�";
const char *c_meter_information_erase_failure_str = "���������ʧ��";
const char *c_continue_to_reset_data_press_enter_str = "��λ���ݰ�ENTER";
const char *c_cancel_reset_data_press_esc_str = "ȡ����λ���ݰ�ESC";
const char *c_reset_month_data_now_str = "���ڸ�λ������";
const char *c_success_reset_month_data_str = "�������Ѹ�λ";
const char *c_reset_day_data_now_str = "���ڸ�λ������";
const char *c_success_reset_day_data_str = "�������Ѹ�λ";
const char *c_success_reset_gasalarm_data_str = "��澯�����Ѹ�λ";
const char *c_resetting_str = "��λ��...";
const char *c_continue_to_restart_terminal_press_enter_str = "ȷ��������ENTER";
const char *c_cancel_restart_terminal_press_esc_str = "ȡ��������ESC";
const char *c_restarting_terminal_str = "������...";
const char *c_ensure_str = "ȷ��";
const char *c_cancle_str = "ȡ��";
const char *c_success_reset_str = "��λ�ɹ�";
const char *c_reset_day_data_str = "�����ݸ�λ";
const char *c_reset_month_data_str = "�����ݸ�λ";
const char *c_reset_gas_alarm_data_str = "��澯���ݸ�λ";
const char *c_rthc_tech_str = "��̩�㴴�Ƽ�";
const char *c_gasmeter_alarm_str = "��澯�¼�";
const char *c_concentrator_alarm_str = "�������澯�¼�";
const char *c_repeater_query_str = "�м̲�ѯ";
const char *c_repeater_setting_str = "�м�����";
const char *c_repeater_str = "�м�:";
const char *c_enable_str = "ʹ��:";
const char *c_meters_file_not_exist_str = "δ�ҵ��ļ�METERS.TXT";
const char *c_meters_file_importing_str = "������...";
const char *c_meters_import_success_str = "����ɹ�!";
const char *modem_flux_sum_str = "GPRS/CDMA������:";
const char *modem_flux_sum_menu = "������ѯ";
const char *c_invalid_value = "��Чֵ��";


// ���ݿ��е����ݵ���
const char *c_get_valid_meter_str = "��ż�״̬";
const char *c_get_valid_day_block_from_variable_str = "�����е���Ч��";
const char *c_get_valid_day_block_from_file_str = "�ļ��е���Ч��";
const char *c_write_variable_into_file_str = "�ѱ���д���ļ�";
const char *c_update_day_block_from_varieble_into_file_str = "update day_block in file";



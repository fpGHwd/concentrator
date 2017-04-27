/*
 * main.h
 *
 *  Created on: 2015-8-10
 *      Author: Johnnyzhang
 */

#ifndef MAIN_H_
#define MAIN_H_

#include "typedef.h"
#include "up_comm.h"

#define CONCENTRATOR_MODEL "GB-0001"

extern const char* g_release_time;
extern int g_terminated;
extern int g_silent;
extern const int g_retry_times;
extern UP_COMM_SOCKET_TYPE g_socket_type;
extern const int rf_id;

struct debug{
	bool gasmeter_test;
	bool watchdog_enable;
	bool repeater_enable;
	bool sqlite_enable;
	bool led_enable;
	bool gpio_enable;
	bool gprs_display_back_enable;
};
extern const struct debug debug_ctrl;




#endif /* MAIN_H_ */

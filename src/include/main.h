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

#endif /* MAIN_H_ */

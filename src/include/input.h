/*
 * input.h
 *
 *  Created on: 2015-8-11
 *      Author: Johnnyzhang
 */

#ifndef INPUT_H_
#define INPUT_H_

#include "typedef.h"

#include "lcd.h"

#define KEY_NONE	0
#define KEY_UP		5
#define KEY_DOWN	24
#define KEY_LEFT	19
#define KEY_RIGHT	4
#define KEY_ENTER	13
#define KEY_ESC		27
#define KEY_NUM		7

#define MAX_DIR_NUM 4
#define DIR_UP		0
#define DIR_DOWN	1
#define DIR_LEFT	2
#define DIR_RIGHT	3

#define MAX_DAY_CNT_IN_WEEK 7
#define SECONDS_IN_DAY (24 * 60 * 60)
#define SECONDS_IN_WEEK (SECONDS_IN_DAY * MAX_DAY_CNT_IN_WEEK)

typedef struct {
	int idx;
	int round_idx[MAX_DIR_NUM];
	int row;
	int col;
	int str_len;
} KEY_ATTR;

typedef struct {
	int year;
	BYTE month;
	BYTE day_cnt; // day number of the month
	BYTE cur_day; // 1 - 31
	// week of the first day in the month 0 - 6, 0 for Sunday
	BYTE first_day_week;
} CALENDAR_T; /// useful, i can use when using, all are data (abstract-)structure.

BYTE key_getch(int flag);

BYTE getch_timeout();

BYTE getch_wait_ms(long msec);

int get_input(keyboard_t type, int maxlen, char *buf, BYTE ispasswd,
		int select_idx);

void reset_input_calendar(void);

int input_calendar(struct tm *tm);

#endif /* INPUT_H_ */

/*
 * input.c
 *
 *  Created on: 2015-8-11
 *      Author: Johnnyzhang
 */

#include "input.h"
#include "main.h"
#include "threads.h"
#include "language.h"
#include "common.h"
#include "menu.h"

#define KEYBOARD_START_ROW	2
#define KEYBOARD_START_COL	3
#define KEYBOARD_HEIGHT		7
#define KEYBOARD_WIDTH		16
#define CALENDAR_START_ROW	2
#define CALENDAR_START_COL	1
#define CALENDAR_HEIGHT		8
#define CALENDAR_WIDTH		20

#define SWITCH_LOWER_LETTER	128
#define SWITCH_UPPER_LETTER	129
#define SWITCH_NUMERIC		130
#define SWITCH_SYMBOL		131

static const unsigned char c_numeric_key[] = { 27, '\b', '\r', '1', '2', '3',
		'4', '5', '6', '7', '8', '9', '*', '0', '#', ',', '.',
		SWITCH_LOWER_LETTER, };

static const KEY_ATTR c_num_keyboard_attr[] =
		{ { 0, { 15, 3, 2, 1 }, KEYBOARD_START_ROW + 1, KEYBOARD_START_COL + 1,
				4 }, { 1, { 16, 4, 0, 2 }, KEYBOARD_START_ROW + 1,
				KEYBOARD_START_COL + 6, 4 }, { 2, { 17, 5, 1, 0 },
				KEYBOARD_START_ROW + 1, KEYBOARD_START_COL + 11, 4 }, { 3, { 0,
				6, 5, 4 }, KEYBOARD_START_ROW + 2, KEYBOARD_START_COL + 2, 1 },
				{ 4, { 1, 7, 3, 5 }, KEYBOARD_START_ROW + 2, KEYBOARD_START_COL
						+ 7, 1 }, { 5, { 2, 8, 4, 3 }, KEYBOARD_START_ROW + 2,
						KEYBOARD_START_COL + 12, 1 }, { 6, { 3, 9, 8, 7 },
						KEYBOARD_START_ROW + 3, KEYBOARD_START_COL + 2, 1 }, {
						7, { 4, 10, 6, 8 }, KEYBOARD_START_ROW + 3,
						KEYBOARD_START_COL + 7, 1 }, { 8, { 5, 11, 7, 6 },
						KEYBOARD_START_ROW + 3, KEYBOARD_START_COL + 12, 1 }, {
						9, { 6, 12, 11, 10 }, KEYBOARD_START_ROW + 4,
						KEYBOARD_START_COL + 2, 1 }, { 10, { 7, 13, 9, 11 },
						KEYBOARD_START_ROW + 4, KEYBOARD_START_COL + 7, 1 }, {
						11, { 8, 14, 10, 9 }, KEYBOARD_START_ROW + 4,
						KEYBOARD_START_COL + 12, 1 }, { 12, { 9, 15, 14, 13 },
						KEYBOARD_START_ROW + 5, KEYBOARD_START_COL + 2, 1 }, {
						13, { 10, 16, 12, 14 }, KEYBOARD_START_ROW + 5,
						KEYBOARD_START_COL + 7, 1 }, { 14, { 11, 17, 13, 12 },
						KEYBOARD_START_ROW + 5, KEYBOARD_START_COL + 12, 1 }, {
						15, { 12, 0, 17, 16 }, KEYBOARD_START_ROW + 6,
						KEYBOARD_START_COL + 2, 1 }, { 16, { 13, 1, 15, 17 },
						KEYBOARD_START_ROW + 6, KEYBOARD_START_COL + 7, 1 }, {
						17, { 14, 2, 16, 15 }, KEYBOARD_START_ROW + 6,
						KEYBOARD_START_COL + 11, 4 }, };

static const unsigned char c_lower_letter_key[] = { 27, '\b', '\r', 'a', 'b',
		'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p',
		'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', ' ',
		SWITCH_UPPER_LETTER, SWITCH_SYMBOL, SWITCH_NUMERIC, };

static const unsigned char c_upper_letter_key[] = { 27, '\b', '\r', 'A', 'B',
		'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
		'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', ' ',
		SWITCH_LOWER_LETTER, SWITCH_SYMBOL, SWITCH_NUMERIC, };

static const KEY_ATTR c_letter_keyboard_attr[] =
		{ { 0, { 30, 4, 2, 1 }, KEYBOARD_START_ROW + 1, KEYBOARD_START_COL + 1,
				4 }, { 1, { 31, 6, 0, 2 }, KEYBOARD_START_ROW + 1,
				KEYBOARD_START_COL + 6, 4 }, { 2, { 32, 9, 1, 0 },
				KEYBOARD_START_ROW + 1, KEYBOARD_START_COL + 11, 4 }, { 3, { 0,
				10, 9, 4 }, KEYBOARD_START_ROW + 2, KEYBOARD_START_COL + 1, 1 },
				{ 4, { 0, 11, 3, 5 }, KEYBOARD_START_ROW + 2, KEYBOARD_START_COL
						+ 3, 1 }, { 5, { 1, 12, 4, 6 }, KEYBOARD_START_ROW + 2,
						KEYBOARD_START_COL + 5, 1 }, { 6, { 1, 13, 5, 7 },
						KEYBOARD_START_ROW + 2, KEYBOARD_START_COL + 7, 1 }, {
						7, { 1, 14, 6, 8 }, KEYBOARD_START_ROW + 2,
						KEYBOARD_START_COL + 9, 1 }, { 8, { 2, 15, 7, 9 },
						KEYBOARD_START_ROW + 2, KEYBOARD_START_COL + 11, 1 }, {
						9, { 2, 16, 8, 3 }, KEYBOARD_START_ROW + 2,
						KEYBOARD_START_COL + 13, 1 }, { 10, { 3, 17, 16, 11 },
						KEYBOARD_START_ROW + 3, KEYBOARD_START_COL + 1, 1 }, {
						11, { 4, 18, 10, 12 }, KEYBOARD_START_ROW + 3,
						KEYBOARD_START_COL + 3, 1 }, { 12, { 5, 19, 11, 13 },
						KEYBOARD_START_ROW + 3, KEYBOARD_START_COL + 5, 1 }, {
						13, { 6, 20, 12, 14 }, KEYBOARD_START_ROW + 3,
						KEYBOARD_START_COL + 7, 1 }, { 14, { 7, 21, 13, 15 },
						KEYBOARD_START_ROW + 3, KEYBOARD_START_COL + 9, 1 }, {
						15, { 8, 22, 14, 16 }, KEYBOARD_START_ROW + 3,
						KEYBOARD_START_COL + 11, 1 }, { 16, { 9, 23, 15, 10 },
						KEYBOARD_START_ROW + 3, KEYBOARD_START_COL + 13, 1 }, {
						17, { 10, 24, 23, 18 }, KEYBOARD_START_ROW + 4,
						KEYBOARD_START_COL + 1, 1 }, { 18, { 11, 25, 17, 19 },
						KEYBOARD_START_ROW + 4, KEYBOARD_START_COL + 3, 1 }, {
						19, { 12, 26, 18, 20 }, KEYBOARD_START_ROW + 4,
						KEYBOARD_START_COL + 5, 1 }, { 20, { 13, 27, 19, 21 },
						KEYBOARD_START_ROW + 4, KEYBOARD_START_COL + 7, 1 }, {
						21, { 14, 28, 20, 22 }, KEYBOARD_START_ROW + 4,
						KEYBOARD_START_COL + 9, 1 }, { 22, { 15, 29, 21, 23 },
						KEYBOARD_START_ROW + 4, KEYBOARD_START_COL + 11, 1 }, {
						23, { 16, 29, 22, 17 }, KEYBOARD_START_ROW + 4,
						KEYBOARD_START_COL + 13, 1 }, { 24, { 17, 30, 29, 25 },
						KEYBOARD_START_ROW + 5, KEYBOARD_START_COL + 1, 1 }, {
						25, { 18, 30, 24, 26 }, KEYBOARD_START_ROW + 5,
						KEYBOARD_START_COL + 3, 1 }, { 26, { 19, 31, 25, 27 },
						KEYBOARD_START_ROW + 5, KEYBOARD_START_COL + 5, 1 }, {
						27, { 20, 31, 26, 28 }, KEYBOARD_START_ROW + 5,
						KEYBOARD_START_COL + 7, 1 }, { 28, { 21, 31, 27, 29 },
						KEYBOARD_START_ROW + 5, KEYBOARD_START_COL + 9, 1 }, {
						29, { 23, 32, 28, 24 }, KEYBOARD_START_ROW + 5,
						KEYBOARD_START_COL + 11, 4 }, { 30, { 25, 0, 32, 31 },
						KEYBOARD_START_ROW + 6, KEYBOARD_START_COL + 1, 4 }, {
						31, { 27, 1, 30, 32 }, KEYBOARD_START_ROW + 6,
						KEYBOARD_START_COL + 6, 4 }, { 32, { 29, 2, 31, 30 },
						KEYBOARD_START_ROW + 6, KEYBOARD_START_COL + 11, 4 }, };

static const unsigned char c_direction_key[] = {
KEY_UP, KEY_LEFT, KEY_RIGHT, KEY_DOWN, };

static const KEY_ATTR c_direction_keyboard_attr[] =
		{
				{ 0, { 3, 3, 1, 2 }, KEYBOARD_START_ROW + 2, KEYBOARD_START_COL
						+ 7, 2 }, { 1, { 0, 3, 2, 2 }, KEYBOARD_START_ROW + 3,
						KEYBOARD_START_COL + 5, 2 }, { 2, { 0, 3, 1, 1 },
						KEYBOARD_START_ROW + 3, KEYBOARD_START_COL + 9, 2 }, {
						3, { 0, 0, 1, 2 }, KEYBOARD_START_ROW + 4,
						KEYBOARD_START_COL + 7, 2 }, };

typedef struct {
	keyboard_t type;
	const unsigned char *key;
	int key_str_len;
	const KEY_ATTR *key_attr;
	const char **key_str;
} virtual_keyboard_t;

static virtual_keyboard_t virtual_keyboard[] = { { _num_keyboard, c_numeric_key,
		sizeof(c_numeric_key), c_num_keyboard_attr, c_numeric_keyboard_str }, {
		_lower_letter_keyboard, c_lower_letter_key, sizeof(c_lower_letter_key),
		c_letter_keyboard_attr, c_lower_letter_keyboard_str }, {
		_upper_letter_keyboard, c_upper_letter_key, sizeof(c_upper_letter_key),
		c_letter_keyboard_attr, c_upper_letter_keyboard_str }, {
		_symbol_keyboard, c_numeric_key, sizeof(c_numeric_key),
		c_num_keyboard_attr }, { _direction_keyboard, c_direction_key,
		sizeof(c_direction_key), c_direction_keyboard_attr,
		c_direction_keyboard_str }, };

static unsigned char idx_to_key(keyboard_t type, int idx) {
	int i;

	for (i = 0; i < sizeof(virtual_keyboard) / sizeof(virtual_keyboard_t);
			i++) {
		if (type == virtual_keyboard[i].type) {
			if (idx < 0 || idx >= virtual_keyboard[i].key_str_len)
				return 0;
			return virtual_keyboard[i].key[idx];
		}
	}
	return 0;
}

static int get_next_keyidx(keyboard_t type, int dir, int idx) {
	int i;

	for (i = 0; i < sizeof(virtual_keyboard) / sizeof(virtual_keyboard_t);
			i++) {
		if (type == virtual_keyboard[i].type) {
			if (idx < 0 || idx >= virtual_keyboard[i].key_str_len)
				return -1;
			if (dir < 0 || dir >= MAX_DIR_NUM)
				return -1;
			return (virtual_keyboard[i].key_attr + idx)->round_idx[dir];
		}
	}
	return -1;
}

static void keyboard_reverse(keyboard_t type, int *idx) {
	int i, row, col1, col2;

	for (i = 0; i < sizeof(virtual_keyboard) / sizeof(virtual_keyboard_t);
			i++) {
		if (type == virtual_keyboard[i].type) {
			if (*idx > virtual_keyboard[i].key_str_len - 1) {
				*idx = virtual_keyboard[i].key_str_len - 1;
			}
			row = (virtual_keyboard[i].key_attr + *idx)->row;
			col1 = (virtual_keyboard[i].key_attr + *idx)->col;
			col2 = col1 + (virtual_keyboard[i].key_attr + *idx)->str_len - 1;
			lcd_reverse_string(row, col1, col2);
			break;
		}
	}
}

static void show_virtual_keyboard(keyboard_t type) {
	int i, j, row, col, len;

	row = KEYBOARD_START_ROW;
	col = KEYBOARD_START_COL;
	len = KEYBOARD_WIDTH;
	for (i = 0; i < sizeof(virtual_keyboard) / sizeof(virtual_keyboard_t);
			i++) {
		if (type == virtual_keyboard[i].type) {
			for (j = 1; j < KEYBOARD_HEIGHT; j++) {
				lcd_show_string(row + j, col, len,
						*(virtual_keyboard[i].key_str + j - 1));
			}
			break;
		}
	}
}

BYTE getch_timeout(void) /// interface 
{
	static int keywait_time = 60; /// 60s key wait time;
	long idle_time = 0;
	BYTE key = KEY_NONE;
	//int i = 0;

	idle_time = uptime();
	while (!g_terminated) {
		notify_watchdog();
		if (lcd_mode_get() == LCD_MODE_TEST /*|| lcd_mode_get() == LCD_MODE_WAIT*/)
			return KEY_ESC;
		key = key_getch(1);
		if (key != KEY_NONE)
			return key;
		else if (uptime() - idle_time >= keywait_time)
			return KEY_NONE;
		msleep(50);
	}
	return KEY_NONE;
}

BYTE getch_wait_ms(long msec) /// 500ms get a value, but the value may be KEY(27)
{
	struct timeval tv1, tv2;
	BYTE key = KEY_NONE;

	gettimeofday(&tv1, NULL);
	while (!g_terminated) {
		notify_watchdog();

		if (lcd_mode_get() == LCD_MODE_TEST) {
			return KEY_ESC;
		}
		key = key_getch(1); /// get 27, 0, 0, 0 /// stop here for value; /// turn to kernel state for the key value;
		if (key != KEY_NONE) {
			////printf("!!!!!!return key(%d)\n", key);
			return key; // return 27
		}

		gettimeofday(&tv2, NULL);
		////printf("(tv1 - tv2) - msec = %d - %d = %d\n", get_diff_ms(&tv2, &tv1), msec, get_diff_ms(&tv2, &tv1) - msec );
		/// -500, - 346, -335
		if (get_diff_ms(&tv2, &tv1) >= msec) {
			////printf("!!!!!!return  KEY_NONE(get_diff_ms(&tv2, &tv1) >= msec)\n");
			return KEY_NONE;
		}
		msleep(10);
	}
	////printf("!!!!!!return  KEY_NONE(function return KEY_NONE)\n");
	return KEY_NONE; /// when g_terminated = 1, runs here;
}

/*
 KEY_VALUE get_wait_ms(void)
 {
 struct timeval time1, time2;
 BYTE key = KEY_NONE;

 gettimeofday(&time1, NULL);
 while(!g_terminated){

 if(lcd_mode_get() == LCD_MODE_TEST)
 return KEY_ESC;

 key_value = key_getch(1);
 if(key_value != KEY_NONE)
 return key_value;

 gettimeofday(&time2, NULL);
 if(time1 - time2 > msec){
 return KEY_NONE;

 continue;
 }
 return KEY_NONE;
 }
 */

/* ret input value from virtual keyboard, all result is saved as ascii */
int get_input(keyboard_t type, int maxlen, char *buf, BYTE ispasswd,
		int select_idx) {
	int ret = -1, cur_idx = 1, len = 0;
	int cursor_row = KEYBOARD_START_ROW, cursor_col = KEYBOARD_START_COL + 1;
	unsigned char keybuf[MAX_INPUT_STRING_LEN], virtual_key;
	BYTE key, refresh = 1, refresh_reverse = 1, exit = 0;
	void *save_ptr;
	int save_len;

	if (select_idx < 0) {
		cur_idx = 1;
	} else {
		cur_idx = select_idx;
	}
	memset(keybuf, 0, sizeof(keybuf));
	lcd_save_window(KEYBOARD_START_ROW, KEYBOARD_START_COL,
	KEYBOARD_START_ROW + KEYBOARD_HEIGHT - 1,
	KEYBOARD_START_COL + KEYBOARD_WIDTH - 1, &save_ptr, &save_len);
	len = min(maxlen, strlen(buf));
	memcpy(keybuf, buf, len);
	lcd_show_string(cursor_row, cursor_col, len, keybuf);
	cursor_col += len;
	if (type != _direction_keyboard)
		lcd_show_cursor(cursor_row, cursor_col, 0);
	while (!g_terminated) {
		if (refresh == 1) {
			show_virtual_keyboard(type);
			keyboard_reverse(type, &cur_idx);
			refresh = 0;
			refresh_reverse = 0;
		} else if (refresh_reverse == 1) {
			refresh_reverse = 0;
			keyboard_reverse(type, &cur_idx);
		}
		key = getch_timeout();
		switch (key) {
		case KEY_UP:
			keyboard_reverse(type, &cur_idx);
			cur_idx = get_next_keyidx(type, DIR_UP, cur_idx);
			refresh_reverse = 1;
			break;
		case KEY_DOWN:
			keyboard_reverse(type, &cur_idx);
			cur_idx = get_next_keyidx(type, DIR_DOWN, cur_idx);
			refresh_reverse = 1;
			break;
		case KEY_LEFT:
			keyboard_reverse(type, &cur_idx);
			cur_idx = get_next_keyidx(type, DIR_LEFT, cur_idx);
			refresh_reverse = 1;
			break;
		case KEY_RIGHT:
			keyboard_reverse(type, &cur_idx);
			cur_idx = get_next_keyidx(type, DIR_RIGHT, cur_idx);
			refresh_reverse = 1;
			break;
		case KEY_ENTER:
			virtual_key = idx_to_key(type, cur_idx);
			switch (virtual_key) {
			case '\r':
				memcpy(buf, keybuf, len);
				ret = len;
				exit = 1;
				break;
			case '\b':
				if (len > 0 && cursor_col > KEYBOARD_START_COL + 1) {
					keybuf[len - 1] = 0;
					len--;
					lcd_show_string(cursor_row, cursor_col - 1, 1, " ");
					lcd_show_cursor(cursor_row, cursor_col, 0);
					cursor_col--;
					lcd_show_cursor(cursor_row, cursor_col, 0);
				}
				break;
			case 27:
				ret = -1;
				exit = 1;
				break;
			case SWITCH_NUMERIC:
				type = _num_keyboard;
				refresh = 1;
				break;
			case SWITCH_LOWER_LETTER:
				type = _lower_letter_keyboard;
				refresh = 1;
				break;
			case SWITCH_UPPER_LETTER:
				type = _upper_letter_keyboard;
				refresh = 1;
				break;
			case SWITCH_SYMBOL:
				type = _num_keyboard;
				refresh = 1;
				break;
			default:
				if (type != _direction_keyboard && len < maxlen) {
					if (!ispasswd) {
						lcd_show_string(cursor_row, cursor_col, 1,
								&virtual_key);
					} else {
						lcd_show_string(cursor_row, cursor_col, 1, "*");
					}
					keybuf[len] = virtual_key;
					len++;
					cursor_col++;
					lcd_show_cursor(cursor_row, cursor_col, 0);
				} else if (type == _direction_keyboard) {
					sprintf(buf, "%c", virtual_key);
					ret = len = 1;
					exit = 1;
				}
				break;
			} /* end of switch virtual_key */
			break;
		case KEY_ESC:
			ret = -1;
			exit = 1;
			break;
		case KEY_NONE:
			lcd_mode_set(0);
			ret = -1;
			exit = 1;
			break;
		default:
			break;
		}
		if (exit) {
			lcd_restore_window(KEYBOARD_START_ROW, KEYBOARD_START_COL,
			KEYBOARD_START_ROW + KEYBOARD_HEIGHT - 1,
			KEYBOARD_START_COL + KEYBOARD_WIDTH - 1, save_ptr, save_len);
			return ret;
		}
	}
	lcd_restore_window(KEYBOARD_START_ROW, KEYBOARD_START_COL,
	KEYBOARD_START_ROW + KEYBOARD_HEIGHT - 1,
	KEYBOARD_START_COL + KEYBOARD_WIDTH - 1, save_ptr, save_len);
	return -1;
}

static void init_calendar(CALENDAR_T *calendar, struct tm *tm) {
	static const BYTE month_day[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31,
			30, 31 };
	time_t tt;

	calendar->year = 1900 + tm->tm_year;
	calendar->month = tm->tm_mon + 1;
	calendar->day_cnt = month_day[calendar->month - 1];
	if (calendar->month == 2 && is_leap_year(calendar->year)) {
		calendar->day_cnt++;
	}
	calendar->cur_day = tm->tm_mday;
	tm->tm_mday = 1;
	tt = mktime(tm);
	localtime_r(&tt, tm);
	calendar->first_day_week = tm->tm_wday;
}

static void calendar_to_tm(struct tm *tm, const CALENDAR_T *calendar) {
	time_t tt;

	tm->tm_year = calendar->year - 1900;
	tm->tm_mon = calendar->month - 1;
	tm->tm_mday = calendar->cur_day;
	tm->tm_hour = 0;
	tm->tm_min = 0;
	tm->tm_sec = 0;
	tt = mktime(tm);
	localtime_r(&tt, tm);
}

static void show_calendar(const CALENDAR_T *calendar) {
	MENU menu;
	char string[MAX_SCREEN_ROW][MAX_SCREEN_COL + 1];
	char date_str[MAX_SCREEN_COL + 1];
	int i, k, row_num, len;
	short day_num;

	row_num = 1
			+ (calendar->first_day_week + calendar->day_cnt)
					/ MAX_DAY_CNT_IN_WEEK;
	if (row_num > MAX_SCREEN_ROW) {
		PRINTF("The calendar row is too many\n");
		return;
	}
	init_menu(&menu);
	sprintf(date_str, "%s %d%s%d%s", c_calendar_select_str, calendar->year,
			c_year_str, calendar->month, c_month_str);
	menu.str[0] = c_weeks_chinese_str;
	memset(string, 0, sizeof(string));
	day_num = 1;
	for (i = 0; i < row_num; i++) {
		for (k = 0; k < MAX_DAY_CNT_IN_WEEK; k++) {
			len = strlen(string[i]);
			if ((i == 0 && k < calendar->first_day_week)
					|| day_num > calendar->day_cnt) {
				if (k == 0) {
					sprintf(&string[i][len], "  ");
				} else {
					sprintf(&string[i][len], "   ");
				}
			} else {
				if (k == 0) {
					sprintf(&string[i][len], "%2d", day_num++);
				} else {
					sprintf(&string[i][len], "%3d", day_num++);
				}
			}
		}
		menu.str[i + 1] = string[i];
	}
	menu.line_num = i + 1;
	lcd_show_arrow(0, 0, 0, 0);
	show_menu(&menu, 1, date_str);
}

static void reverse_calendar(const CALENDAR_T *calendar) {
	int row, col, len;

	row = CALENDAR_START_ROW + 1
			+ (calendar->first_day_week + calendar->cur_day - 1)
					/ MAX_DAY_CNT_IN_WEEK;
	col = (MAX_SCREEN_COL - CALENDAR_WIDTH) / 2
			+ (((calendar->first_day_week + calendar->cur_day - 1)
					% MAX_DAY_CNT_IN_WEEK) + 1) * 3 - 1;
	if (calendar->cur_day >= 10) {
		len = 2;
	} else {
		len = 1;
	}
	lcd_reverse_string(row, col - len + 1, col);
}

static struct CALE_ATTR {
	BYTE key;
	long int seconds;
	short days;
} cale_mod_attrs[] = { { KEY_UP, -SECONDS_IN_WEEK, -MAX_DAY_CNT_IN_WEEK }, {
		KEY_DOWN, SECONDS_IN_WEEK, MAX_DAY_CNT_IN_WEEK }, { KEY_LEFT,
		-SECONDS_IN_DAY, -1 }, { KEY_RIGHT, SECONDS_IN_DAY, 1 }, };

static time_t last_cale_tt;

void reset_input_calendar(void) {
	last_cale_tt = 0;
}

int input_calendar(struct tm *out_tm) {
	CALENDAR_T calendar;
	time_t tt;
	struct tm tm;
	BYTE key, refresh = 1, refresh_reverse = 1;
	short day_num;
	void *save_ptr;
	int save_len;
	int i, ret = FALSE, exit = 0;

	if (last_cale_tt == 0) {
		sys_time(&tm);
	} else {
		localtime_r(&last_cale_tt, &tm);
	}
	init_calendar(&calendar, &tm);
	lcd_save_window(CALENDAR_START_ROW, CALENDAR_START_COL,
	CALENDAR_START_ROW + CALENDAR_HEIGHT - 1,
	CALENDAR_START_COL + CALENDAR_WIDTH - 1, &save_ptr, &save_len);
	while (!g_terminated) {
		if (refresh) {
			show_calendar(&calendar);
			refresh_reverse = 1;
			refresh = 0;
		}
		if (refresh_reverse) {
			reverse_calendar(&calendar);
			refresh_reverse = 0;
		}
		key = getch_timeout();
		switch (key) {
		case KEY_UP:
		case KEY_DOWN:
		case KEY_LEFT:
		case KEY_RIGHT:
			for (i = 0; i < sizeof(cale_mod_attrs) / sizeof(struct CALE_ATTR);
					i++) {
				if (key == cale_mod_attrs[i].key) {
					day_num = calendar.cur_day + cale_mod_attrs[i].days;
					if (day_num < 1 || day_num > calendar.day_cnt) {
						calendar_to_tm(&tm, &calendar);
						tt = mktime(&tm);
						tt += cale_mod_attrs[i].seconds;
						localtime_r(&tt, &tm);
						init_calendar(&calendar, &tm);
						refresh = 1;
					} else {
						reverse_calendar(&calendar); // del reverse
						calendar.cur_day += cale_mod_attrs[i].days;
						refresh_reverse = 1;
					}
				}
			}
			break;
		case KEY_ENTER:
			calendar_to_tm(out_tm, &calendar);
			last_cale_tt = mktime(out_tm);
			ret = TRUE;
			exit = 1;
			break;
		case KEY_ESC:
			ret = FALSE;
			exit = 1;
			break;
		case KEY_NONE:
			ret = FALSE;
			exit = 1;
			break;
		default:
			break;
		}
		if (exit) {
			lcd_restore_window(CALENDAR_START_ROW, CALENDAR_START_COL,
			CALENDAR_START_ROW + CALENDAR_HEIGHT - 1,
			CALENDAR_START_COL + CALENDAR_WIDTH - 1, save_ptr, save_len);
			return ret;
		}
	}
	lcd_restore_window(CALENDAR_START_ROW, CALENDAR_START_COL,
	CALENDAR_START_ROW + CALENDAR_HEIGHT - 1,
	CALENDAR_START_COL + CALENDAR_WIDTH - 1, save_ptr, save_len);
	return ret;
}

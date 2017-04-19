/*
 * menu.c
 *
 *  Created on: 2015-8-11
 *      Author: Johnnyzhang
 */

#include "menu.h"
#include "main.h"
#include "common.h"
#include "threads.h"
#include "language.h"

static int lcd_mode = LCD_MODE_SCROLL;
//static int lcd_mode = LCD_MODE_TEST;

int lcd_mode_get(void) {
	return lcd_mode;
}

void lcd_mode_set(int mode) {
	lcd_mode = mode;
}

void init_menu(MENU *menu) {
	memset(menu, 0, sizeof(menu));
	menu->row = STATUS_BAR_LINE + 1;
	menu->col = 1;
	menu->vrow_num = MAX_WORKSPACE_LINE;
	menu->vcol_num = MAX_SCREEN_COL;
	menu->start_line = 1;
}

void init_input_set(INPUT_STRING *input, int row, int col, int len,
		const char *str, int maxlen) {
	input->row = row;
	input->col = col;
	len = min(len, maxlen);
	input->len = len;
	memcpy(input->str, str, len);
	input->maxlen = maxlen;
}

void show_menu(MENU *menu, int align_center, const char *info) {
	int len, offset = 0, vwidth, row;
	const char *ptr;
	char buf[MAX_SCREEN_COL + 1]; /// show buf
	int i, left_spaces = 0;

	/// check argument if is valid, menu for instance
	/// instant message, for instance(for example)

	if (menu->col > 0&& menu->col <= MAX_SCREEN_COL
	&& menu->row > STATUS_BAR_LINE
	&& menu->row <= MAX_SCREEN_ROW - INFO_BAR_LINE) { /// row is the menu essential message
		vwidth = menu->vcol_num; /// 
		for (i = menu->start_line - 1;
				i < menu->start_line - 1 + menu->vrow_num; i++) {
			row = menu->row + i - (menu->start_line - 1); /// 
			memset(buf, ' ', sizeof(buf));
			ptr = menu->str[i];
			if (ptr != NULL && i <= menu->line_num - 1) { /// ptr is 
				len = strlen(ptr);
				if (len <= vwidth) {
					if (align_center == 1)
						left_spaces = (menu->vcol_num - len) / 2;
					else
						left_spaces = 0; /// if align mode is center, change the
					// string(buf) showing localtion
					memcpy(buf + left_spaces, ptr, len);
				} else {
					memcpy(buf, ptr, vwidth);
				}
			}
			lcd_show_string(row + offset, menu->col, vwidth, buf);/// show buff
		}
		lcd_update_info(info);// show content and update info /// botton tips info	
	}
}

static struct timeval last_enter_tv;
static int last_is_enter = 0;

BYTE process_menu(MENU *menu, const char *info) {
	unsigned char key;
	int need_refresh = 1;
	struct timeval cur_tv;

	while (key_getch(1) != KEY_NONE)
		/// a pause
		;
	while (!g_terminated) {
		if (need_refresh == 1) {
			show_menu(menu, 0, info);
			lcd_show_arrow(menu->start_line > 1,
					menu->line_num - menu->start_line >= menu->vrow_num, 0, 0); /// show the arrow
			need_refresh = 0;
		}
		key = getch_timeout();
		if (key != KEY_ENTER) {
			last_is_enter = 0;
		}
		switch (key) {
		case KEY_UP:
			if (menu->start_line > 1) {
				menu->start_line--;
				need_refresh = 1;
			}
			break;
		case KEY_DOWN:
			if (menu->line_num - menu->start_line >= menu->vrow_num) {
				menu->start_line++;
				need_refresh = 1;
			}
			break;
		case KEY_ESC:
		case KEY_ENTER:
			if (key == KEY_ENTER) {
				gettimeofday(&cur_tv, NULL);
				if (last_is_enter
						&& (get_diff_ms(&cur_tv, &last_enter_tv) < 300)) {
					gettimeofday(&last_enter_tv, NULL);
					break; /// enter < 300, break
				}
				last_is_enter = 1;
				gettimeofday(&last_enter_tv, NULL);
			}
			lcd_show_arrow(0, 0, 0, 0);
			return key; ///
		case KEY_NONE:
			lcd_mode_set(0);
			lcd_show_arrow(0, 0, 0, 0);
			return KEY_NONE;
		default:
			break;
		}
	}
	return KEY_NONE;
}

BYTE process_items(ITEMS_MENU *items, const char *info, int align_center) {
	BYTE key = KEY_ENTER;
	int reverse_row, reverse_col, reverse_len, first = 0, refresh_menu = 1;
	struct timeval cur_tv;

	while (key_getch(1) != KEY_NONE) /// key value do nothing
		msleep(10);
	while (!g_terminated) {
		notify_watchdog();
		if (lcd_mode_get() == 0 || lcd_mode_get() > 2)
			return KEY_ESC;
		if ((key == KEY_UP || key == KEY_DOWN || key == KEY_ENTER)
				&& refresh_menu) {
			show_menu(&items->menu, align_center, info);
			refresh_menu = 0;
			/* reverse display cur_line */
			reverse_len = strlen(items->menu.str[items->cur_line - 1]); /// strlen of reversed row(row chosen now)
			reverse_row = items->cur_line - items->menu.start_line /// reversed row/rank number /// this is the key to understand the reverse_row, else is not the key
			+ items->menu.row; /// calculate reverse_row
			if (align_center == 1)
				reverse_col = items->menu.col
						+ (items->menu.vcol_num - reverse_len) / 2;
			else
				reverse_col = items->menu.col; ///
			/*lcd_reverse_string(reverse_row, reverse_col,
			 reverse_col + reverse_len - 1); */
			lcd_reverse_string(reverse_row, 1, MAX_SCREEN_COL); /// reverse row displaying
			lcd_show_arrow(items->cur_line > 1,
					items->cur_line < items->menu.line_num, 0, 0);
		}
		if (info == c_info_main_menu_str) {
			if (first == 0) {
				first = 1;
				/*
				 while (getch_wait_ms(1000) != KEY_NONE) /// 500ms -> 1000ms
				 ;
				 */  /// commented by wd
			}
		}
		key = getch_timeout();
		if (key != KEY_ENTER) {
			last_is_enter = 0;
		}
		switch (key) {
		case KEY_UP:
			refresh_menu = 1; /// need refresh
			if (items->cur_line == 1) {
				items->cur_line = items->menu.line_num;
				if (items->cur_line - items->menu.start_line
						>= items->menu.vrow_num) {
					items->menu.start_line = items->cur_line
							- items->menu.vrow_num + 1;
				}
			} else {
				items->cur_line--;
				if (items->cur_line < items->menu.start_line) {
					items->menu.start_line--;
				}
			} /// get items argumentation set
			break;
		case KEY_DOWN:
			refresh_menu = 1;
			if (items->cur_line < items->menu.line_num) {
				items->cur_line++;
				if (items->cur_line - items->menu.start_line
						>= items->menu.vrow_num) {
					items->menu.start_line++;
				}
			} else {
				items->cur_line = 1;
				if (items->menu.start_line > 1) {
					items->menu.start_line = 1;
				}
			}
			break;
		case KEY_ENTER:
			gettimeofday(&cur_tv, NULL);
			if (last_is_enter && (get_diff_ms(&cur_tv, &last_enter_tv) < 300)) {
				gettimeofday(&last_enter_tv, NULL);
				break;
			}
			last_is_enter = 1;
			gettimeofday(&last_enter_tv, NULL);
			refresh_menu = 1;
			lcd_show_arrow(0, 0, 0, 0);
			if (items->func[items->cur_line - 1] != NULL) {
				LOG_PRINTF("%s -> \"%s\"\n", c_enterinto_menu_str,
						items->menu.str[items->cur_line - 1]);
				items->func[items->cur_line - 1](0,
						items->para[items->cur_line - 1],
						items->menu.str[items->cur_line - 1]);
				break;
			}
			return KEY_ENTER;
		case KEY_NONE:
			lcd_mode_set(0);
			break;
		case KEY_ESC:
			if (info == c_info_main_menu_str) /// add wd
				return KEY_ESC;
			///printf("process_menu case KEY_ESC\n");
			///printf("info != c_info_main_menu: %s, lcd_mode_get() == LCD_MODE_TEST: %s\n", (info != c_info_main_menu_str)?"TRRE":"FALSE", (lcd_mode_get() == LCD_MODE_TEST)?"TRUE":"FALSE");
			if (info != c_info_main_menu_str || lcd_mode_get() == LCD_MODE_TEST) {
				lcd_show_arrow(0, 0, 0, 0);
				LOG_PRINTF("%s <- \"%s\"\n", c_exit_menu_str, info);
				return KEY_ESC;
			} else {
				break; /// do not comment this, for that the exit of this function is return KEY_ESC;
			}

		default:
			break;
		}
	}
	return KEY_ESC;
}

static void show_param_list(PARAM_LIST *param, int set_underline) {
	int i, j, row, col, tmp, len;
	int group_idx, digit_idx;
	int start_row, end_row, start_col, end_col;
	WORD group_num, digit_num;
	BYTE value_idx;
	char buf[MAX_SCREEN_COL], *ptr;

	start_row = param->menu.row;
	start_col = param->menu.col;
	end_row = param->menu.row - 1 + param->menu.vrow_num;
	end_col = param->menu.col - 1 + param->menu.vcol_num;
	group_idx = param->group_idx;
	digit_idx = param->digit_idx;
	group_num = param->group_num;
	for (i = 0; i < group_num; i++) {
		digit_num = param->list[i].digit_num;
		ptr = buf;
		len = 0;
		for (j = 0; j < digit_num; j++) {
			value_idx = param->list[i].value_idx[j];
			tmp = strlen(param->list[i].const_list[value_idx]);
			memcpy(ptr, param->list[i].const_list[value_idx], tmp);
			ptr += tmp;
			len += tmp;
		}
		row = param->list[i].row + param->menu.row - param->menu.start_line;
		col = param->menu.col - 1 + param->list[i].col;
		if (row >= start_row && row <= end_row && col >= start_col
				&& col <= end_col) {
			lcd_show_string(row, col, len, buf);
			lcd_show_underline(row, col, col + len - 1, set_underline);
		}
	}
	if (param->flag[group_idx] == 1) {
		value_idx = param->list[group_idx].value_idx[digit_idx];
		tmp = strlen(param->list[group_idx].const_list[value_idx]);
		row = param->list[group_idx].row + param->menu.row
				- param->menu.start_line;
		col = param->menu.col - 1 + param->list[group_idx].col;
		lcd_show_underline(row, col, col + tmp - 1, 0);
		lcd_reverse_string(row, col, col + tmp - 1);
	}
}

static int check_input_list(INPUT_LIST *list, int max) {
	int i, j = 1;
	int temp = 0;

	if (list->digit_num <= 0)
		return 0;
	for (i = list->digit_num - 1; i >= 0; i--) {
		temp += list->value_idx[i] * j;
		j = j * 10;
	}
	if (temp <= max)
		return 1;
	else
		return 0;
}

BYTE process_param_list(PARAM_LIST *param, const char *info) /// process_param_list
{
	BYTE key, refresh_cursor = 1, refresh_menu = 1, refresh_list = 1;
	BYTE temp, ret_key;
	int group_idx, digit_idx, i, start_row, last_row;
	ITEMS_MENU im_pop_up;
	WORD pop_line_num, pop_col_num, max_col_num;
	void *save_ptr;
	int save_len;
	struct timeval cur_tv;

	group_idx = param->group_idx;
	digit_idx = param->digit_idx;
	while (key_getch(1) != KEY_NONE)
		;
	while (!g_terminated) {
		if (lcd_mode_get() == 0 || lcd_mode_get() > 2) {
			lcd_show_cursor(0, 0, 0);
			show_param_list(param, 0);
			return KEY_ESC;
		}
		if (refresh_menu == 1) {
			refresh_menu = 0;
			show_menu(&param->menu, 0, info);
		}
		if (refresh_list == 1) {
			refresh_list = 0;
			show_param_list(param, 1);
		}
		if (refresh_cursor == 1) {
			refresh_cursor = 0;
			lcd_show_cursor(
					param->menu.row + param->list[group_idx].row
							- param->menu.start_line,
					param->menu.col - 1 + param->list[group_idx].col
							+ digit_idx, 0);
		}
		key = getch_timeout();
		if (key != KEY_ENTER) {
			last_is_enter = 0;
		}
		switch (key) {
		case KEY_UP:
			if (param->list[group_idx].value_idx[digit_idx]
					< (param->list[group_idx].list_num - 1)) {
				temp = param->list[group_idx].value_idx[digit_idx];
				param->list[group_idx].value_idx[digit_idx]++;
				if (!check_input_list(&param->list[group_idx], /// check_input_list
						param->list[group_idx].max_value))
					param->list[group_idx].value_idx[digit_idx] = temp;
			} else {
				temp = param->list[group_idx].value_idx[digit_idx];
				param->list[group_idx].value_idx[digit_idx] = 0;
				if (!check_input_list(&param->list[group_idx],
						param->list[group_idx].max_value))
					param->list[group_idx].value_idx[digit_idx] = temp;
			}
			refresh_list = 1;
			refresh_cursor = 1;
			break;
		case KEY_DOWN:
			if (param->list[group_idx].value_idx[digit_idx] > 0) {
				temp = param->list[group_idx].value_idx[digit_idx];
				param->list[group_idx].value_idx[digit_idx]--;
				if (!check_input_list(&param->list[group_idx],
						param->list[group_idx].max_value))
					param->list[group_idx].value_idx[digit_idx] = temp;
			} else {
				temp = param->list[group_idx].value_idx[digit_idx];
				param->list[group_idx].value_idx[digit_idx] =
						param->list[group_idx].list_num - 1;
				if (!check_input_list(&param->list[group_idx],
						param->list[group_idx].max_value))
					param->list[group_idx].value_idx[digit_idx] = temp;
			}
			refresh_list = 1;
			refresh_cursor = 1;
			break;
		case KEY_LEFT:
			if (group_idx == 0 && digit_idx == 0)
				continue;
			if (digit_idx > 0) {
				digit_idx--;
				param->digit_idx = digit_idx;
			} else {
				group_idx--;
				param->group_idx = group_idx;
				digit_idx = param->list[group_idx].digit_num - 1;
				param->digit_idx = digit_idx;
			}
			refresh_list = 1;
			refresh_cursor = 1;
			while (!g_terminated) {
				start_row = param->menu.start_line;
				if (param->list[group_idx].row < start_row) {
					show_param_list(param, 0);
					param->menu.start_line--;
					refresh_menu = 1;
				} else
					break;
			}
			break;
		case KEY_RIGHT:
			if (group_idx == (param->group_num - 1)
					&& digit_idx == (param->list[group_idx].digit_num - 1))
				continue;
			if (digit_idx == (param->list[group_idx].digit_num - 1)) {
				group_idx++;
				param->group_idx = group_idx;
				digit_idx = param->digit_idx = 0;
			} else {
				digit_idx++;
				param->digit_idx = digit_idx;
			}
			refresh_list = 1;
			refresh_cursor = 1;
			while (!g_terminated) {
				last_row = param->menu.start_line - 1 + param->menu.vrow_num;
				if (param->list[group_idx].row > last_row) {
					show_param_list(param, 0);
					param->menu.start_line++;
					refresh_menu = 1;
				} else
					break;
			}
			break;
		case KEY_ENTER:
			gettimeofday(&cur_tv, NULL);
			if (last_is_enter && (get_diff_ms(&cur_tv, &last_enter_tv) < 300)) {
				gettimeofday(&last_enter_tv, NULL);
				break;
			}
			last_is_enter = 1;
			gettimeofday(&last_enter_tv, NULL);
			lcd_show_cursor(0, 0, 0);
			group_idx = param->group_idx;
			if (param->flag[group_idx] == 1) {
				pop_line_num = param->list[group_idx].list_num;
				max_col_num = 0;
				for (i = 0; i < param->list[group_idx].list_num; i++) {
					pop_col_num = strlen(param->list[group_idx].const_list[i]);
					max_col_num = max(pop_col_num, max_col_num);
				}
				im_pop_up.menu.row = (param->menu.vrow_num - pop_line_num) / 2
						+ param->menu.row;
				im_pop_up.menu.col = (param->menu.vcol_num - (max_col_num + 1))
						/ 2; /* add a space after strings */
				im_pop_up.menu.vrow_num = pop_line_num;
				im_pop_up.menu.vcol_num = max_col_num;
				if (im_pop_up.menu.vcol_num < MAX_SCREEN_COL) {
					im_pop_up.menu.vcol_num++;
				}
				im_pop_up.menu.line_num = pop_line_num;
				im_pop_up.menu.start_line = 1;
				im_pop_up.cur_line = param->list[group_idx].value_idx[0];
				for (i = 0; i < pop_line_num; i++) {
					im_pop_up.menu.str[i] =
							param->list[group_idx].const_list[i];
					im_pop_up.func[i] = NULL;
				}
				lcd_save_window(im_pop_up.menu.row, im_pop_up.menu.col,
						im_pop_up.menu.row + im_pop_up.menu.vrow_num - 1,
						im_pop_up.menu.col + im_pop_up.menu.vcol_num - 1,
						&save_ptr, &save_len);
				ret_key = process_items(&im_pop_up, NULL, 1);
				lcd_restore_window(im_pop_up.menu.row, im_pop_up.menu.col,
						im_pop_up.menu.row + im_pop_up.menu.vrow_num - 1,
						im_pop_up.menu.col + im_pop_up.menu.vcol_num - 1,
						save_ptr, save_len);
				if (ret_key == KEY_ENTER) {
					param->list[group_idx].value_idx[0] = im_pop_up.cur_line
							- 1;
					refresh_list = 1;
				}
				refresh_cursor = 1;
				break;
			}
			show_param_list(param, 0);
			lcd_update_info("");
			return key;
		case KEY_ESC:
			lcd_show_cursor(0, 0, 0);
			show_param_list(param, 0);
			lcd_update_info("");
			return key;
		case KEY_NONE:
			lcd_mode_set(0);
			break;
		default:
			break;
		}
	}
	return KEY_ESC;
}

static void show_input(PARAM_SET *param, BYTE idx, int set_underline) {
	int i, row, col;
	char string[MAX_SCREEN_COL];

	for (i = 0; i < param->group_num; i++) {
		memset(string, 0, sizeof(string));
		if (idx != 0xff)
			i = idx;
		if (param->input[i].row < param->menu.start_line)
			continue;
		row = param->menu.row + param->input[i].row - param->menu.start_line;
		if (row > (param->menu.row - 1 + param->menu.vrow_num))
			break;
		col = param->menu.col - 1 + param->input[i].col;
		if (param->input[i].len < 1)
			lcd_show_underline(row, col, col, set_underline);
		else
			lcd_show_underline(row, col, col + param->input[i].len - 1,
					set_underline);
		memcpy(string, param->input[i].str, param->input[i].len);
		lcd_show_string(row, col, param->input[i].maxlen, string);
		if (idx != 0xff)
			return;
	}
}

BYTE process_param_set(PARAM_SET *param, const char *info)  /// set
{
	int group_idx, ret, i, cursor_row, cursor_col;
	int last_cursor_row = 0, last_cursor_col = 0;
	int last_cursor_len = 0, cursor_len;
	BYTE key, refresh_menu = 1, refresh_input = 1, refresh_cursor = 1;
	BYTE ret_key, cur_input_idx = 0xff;
	ITEMS_MENU im_pop_up;
	void *save_ptr;
	int save_len;
	struct timeval cur_tv;

	while (key_getch(1) != KEY_NONE)
		;
	while (!g_terminated) {
		if (lcd_mode_get() == 0 || lcd_mode_get() > 2) {
			lcd_show_cursor(0, 0, 0);
			show_input(param, 0xff, 0);
			return KEY_ESC;
		}
		if (refresh_menu == 1) {
			refresh_menu = 0;
			show_menu(&param->menu, 0, info);
		}
		if (refresh_input == 1) {
			refresh_input = 0;
			show_input(param, cur_input_idx, 1);
			last_cursor_row = 0;
			last_cursor_col = 0;
			last_cursor_len = 0;
		}
		if (refresh_cursor == 1) {
			refresh_cursor = 0;
			if (last_cursor_row >= param->menu.row
					&& last_cursor_row < param->menu.row + param->menu.vrow_num
					&& last_cursor_col >= param->menu.col
					&& last_cursor_col
							< param->menu.col + param->menu.vcol_num) {
				lcd_show_cursor(last_cursor_row, last_cursor_col,
						last_cursor_len);
			}
			group_idx = param->group_idx;
			cursor_row = param->menu.row + param->input[group_idx].row
					- param->menu.start_line;
			;
			cursor_col = param->menu.col + param->input[group_idx].col - 1;
			cursor_len = param->input[group_idx].len;
			if (cursor_row >= param->menu.row
					&& cursor_row < param->menu.row + param->menu.vrow_num
					&& cursor_col >= param->menu.col
					&& cursor_col < param->menu.col + param->menu.vcol_num) {
				lcd_show_cursor(cursor_row, cursor_col, cursor_len);
			}
			last_cursor_row = cursor_row;
			last_cursor_col = cursor_col;
			last_cursor_len = cursor_len;
		}
		key = getch_timeout();
		if (key != KEY_ENTER) {
			last_is_enter = 0;
		}
		switch (key) {
		case KEY_UP:
			if (param->menu.start_line > 1) {
				cur_input_idx = 0xff;
				show_input(param, cur_input_idx, 0);
				param->menu.start_line--;
				refresh_menu = 1;
				refresh_input = 1;
				refresh_cursor = 1;
			}
			break;
		case KEY_DOWN:
			if (param->menu.line_num - param->menu.start_line
					>= param->menu.vrow_num) {
				cur_input_idx = 0xff;
				show_input(param, cur_input_idx, 0);
				param->menu.start_line++;
				refresh_menu = 1;
				refresh_input = 1;
				refresh_cursor = 1;
			}
			break;
		case KEY_LEFT:
			if (param->group_idx > 0) {
				param->group_idx--;
				refresh_cursor = 1;
				group_idx = param->group_idx;
				if (param->input[group_idx].row < param->menu.start_line) {
					cur_input_idx = 0xff;
					show_input(param, cur_input_idx, 0);
					param->menu.start_line--;
					refresh_menu = 1;
					refresh_input = 1;
				}
			}
			break;
		case KEY_RIGHT:
			if (param->group_idx < param->group_num - 1) {
				param->group_idx++;
				refresh_cursor = 1;
				group_idx = param->group_idx;
				if (param->input[group_idx].row - param->menu.start_line
						>= param->menu.vrow_num) {
					cur_input_idx = 0xff;
					show_input(param, cur_input_idx, 0);
					param->menu.start_line++;
					refresh_menu = 1;
					refresh_input = 1;
				}
			}
			break;
		case KEY_ENTER:
			gettimeofday(&cur_tv, NULL);
			if (last_is_enter && (get_diff_ms(&cur_tv, &last_enter_tv) < 300)) {
				gettimeofday(&last_enter_tv, NULL);
				break;
			}
			last_is_enter = 1;
			gettimeofday(&last_enter_tv, NULL);
			lcd_show_cursor(0, 0, 0);
			group_idx = param->group_idx;
			if (param->keyboard_type[group_idx] == _none_keyboard) {
				memset(&im_pop_up, 0, sizeof(im_pop_up));
				im_pop_up.menu.vcol_num = 0;
				im_pop_up.menu.line_num = param->input[group_idx].list_num;
				im_pop_up.menu.vrow_num = min(4, im_pop_up.menu.line_num);
				for (i = 0; i < im_pop_up.menu.line_num; i++) {
					im_pop_up.menu.vcol_num = max(im_pop_up.menu.vcol_num,
							strlen(param->input[group_idx].const_list[i]));
				}
				if (im_pop_up.menu.vcol_num < MAX_SCREEN_COL) {
					im_pop_up.menu.vcol_num++;
				}
				im_pop_up.menu.row = (param->menu.vrow_num
						- im_pop_up.menu.vrow_num) / 2 + param->menu.row;
				im_pop_up.menu.col = (param->menu.vcol_num
						- im_pop_up.menu.vcol_num) / 2 + 1;
				im_pop_up.cur_line = param->input[group_idx].list_idx + 1;
				if (im_pop_up.menu.line_num < im_pop_up.menu.vrow_num) {
					im_pop_up.menu.start_line = 1;
				} else {
					im_pop_up.menu.start_line =
							(im_pop_up.menu.line_num - im_pop_up.cur_line
									< im_pop_up.menu.vrow_num) ?
									im_pop_up.menu.line_num
											- im_pop_up.menu.vrow_num + 1 :
									im_pop_up.cur_line;
				}
				for (i = 0; i < im_pop_up.menu.line_num; i++) {
					im_pop_up.menu.str[i] =
							param->input[group_idx].const_list[i];
					im_pop_up.func[i] = NULL;
					im_pop_up.flag[i] = 0;
				}
				lcd_save_window(im_pop_up.menu.row, im_pop_up.menu.col,
						im_pop_up.menu.row + im_pop_up.menu.vrow_num - 1,
						im_pop_up.menu.col + im_pop_up.menu.vcol_num - 1,
						&save_ptr, &save_len);
				ret_key = process_items(&im_pop_up, NULL, 1);
				lcd_restore_window(im_pop_up.menu.row, im_pop_up.menu.col,
						im_pop_up.menu.row + im_pop_up.menu.vrow_num - 1,
						im_pop_up.menu.col + im_pop_up.menu.vcol_num - 1,
						save_ptr, save_len);
				if (ret_key != KEY_ENTER) {
					refresh_cursor = 1;
					break;
				} else {
					cur_input_idx = group_idx;
					show_input(param, cur_input_idx, 0);
					param->input[group_idx].len = strlen(
							im_pop_up.menu.str[im_pop_up.cur_line - 1]);
					param->input[group_idx].list_idx = im_pop_up.cur_line - 1;
					memcpy(param->input[group_idx].str,
							im_pop_up.menu.str[im_pop_up.cur_line - 1],
							param->input[group_idx].len);
				}
			} else {
				cur_input_idx = 0xff;
				show_input(param, cur_input_idx, 0);
				ret = get_input(param->keyboard_type[group_idx],
						param->input[group_idx].maxlen,
						param->input[group_idx].str, 0, -1);
				if (ret >= 0) {
					param->input[group_idx].len = ret;
					param->input[group_idx].str[param->input[group_idx].len] =
							0;
				}
			}
			refresh_input = 1;
			refresh_cursor = 1;
			break;
		case KEY_ESC:
			show_input(param, 0xff, 0);
			lcd_update_info("");
			return KEY_ESC;
		case KEY_NONE:
			lcd_mode_set(0);
			break;
		default:
			break;
		}
	}
	return KEY_ESC;
}

void lcd_update_comm_info(int flag)
{
	// TODO /// update_comm_info
}

/*
 * menu.h
 *
 *  Created on: 2015-8-11
 *      Author: Johnnyzhang
 */

#ifndef MENU_H_
#define MENU_H_

#include "typedef.h"
#include "lcd.h"
#include "input.h"

#define LCD_MODE_SCROLL 0
#define LCD_MODE_MENU	1
#define LCD_MODE_TEST	2
#define LCD_MODE_WAIT	3

int lcd_mode_get(void);

void lcd_mode_set(int mode);

void init_menu(MENU *menu);
void init_input_set(INPUT_STRING *input, int row, int col, int len,
		const char *str, int maxlen);

void show_menu(MENU *menu, int align_center, const char *info);

BYTE process_menu(MENU *menu, const char *info);

BYTE process_items(ITEMS_MENU *items, const char *info, int align_center);

BYTE process_param_list(PARAM_LIST *param, const char *info);

BYTE process_param_set(PARAM_SET *param, const char *info);

void lcd_update_head_enable(int flag);

void lcd_update_info_enable(int flag);

int update_head_is_enable(void);

void lcd_update_comm_info(int flag);

void lcd_update_task_info(const char *buf);

/// add by wd 
void clean_the_content_menu(void);

#endif /* MENU_H_ */

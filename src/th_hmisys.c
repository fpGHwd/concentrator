/*
 * th_hmisys.c
 *
 *  Created on: 2015-8-11
 *      Author: Johnnyzhang
 */

#include "threads.h"
#include "main.h"
#include "common.h"
#include "input.h"
#include "menu_gas.h"
#include "language.h"
#include "con_test.h"
#include "lcd.h"

void show_logo_and_date(void);
static void process_scroll_show(void) {
	BYTE key = KEY_NONE;

	lcd_clean_workspace();
	lcd_show_string(MAX_SCREEN_ROW, 1, strlen(c_allspace_str), c_allspace_str);

	while (!g_terminated) {
		msleep(300);
		show_logo_and_date();

		//key = getch_timeout(1000);
		if ((key= getch_timeout(1000)) == KEY_ENTER) {
			return;
		} else {
			continue;
		}

	}
	return;
}

void *th_hmisys(void * arg) {
	ITEMS_MENU im_main;
	int lcd_ok = 0, mode;
	//unsigned char key_value;

	while (!g_terminated) {

		notify_watchdog();
		if (!lcd_ok) {
			if (!lcd_is_ok()) {
				PRINTF("Open lcd fd error\n");
				wait_delay(1000);
				continue;
			}
			/*
			 while(key_getch(1) != KEY_NONE)
			 ;
			 */
			main_menu_init(&im_main);
		}
		mode = lcd_mode_get();

		if (mode == LCD_MODE_SCROLL) {
			process_scroll_show();
			lcd_mode_set(LCD_MODE_MENU);
		} else if (mode == LCD_MODE_TEST) {
			con_test_exec();
		} else {
			if (process_items(&im_main, c_info_main_menu_str, FALSE) == KEY_ESC) {
				if (lcd_mode_get() != LCD_MODE_TEST) {
					lcd_mode_set(LCD_MODE_SCROLL);
				}
			}
		}
	}
	lcd_clear_screen();
	return NULL;
}

void show_logo_and_date(void) {
	char buff[11];

	get_date(buff);
	lcd_show_string(4, 5, strlen(c_rthc_tech_str), c_rthc_tech_str);
	//lcd_show_string(4, 7, strlen(c_cqrl_str), c_cqrl_str);
	lcd_show_string(5, 6, strlen(buff), buff);
}

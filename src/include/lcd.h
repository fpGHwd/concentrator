/*
 * lcd.h
 *
 *  Created on: 2015-8-11
 *      Author: Johnnyzhang
 */

#ifndef LCD_H_
#define LCD_H_

#include "typedef.h"

#define MAX_SCREEN_ROW			lcd_screen_row()
#define MAX_SCREEN_COL			lcd_screen_col()
#define STATUS_BAR_LINE			1
#define INFO_BAR_LINE			1
#define MAX_WORKSPACE_LINE		(MAX_SCREEN_ROW-STATUS_BAR_LINE-INFO_BAR_LINE)

#define MAX_MENU_LINE			100
#define MAX_INPUT_GROUP_NUM		100
#define MAX_INPUT_DIGIT_NUM		16
#define MAX_INPUT_STRING_LEN	16
#define MAX_LIST_NUM			32

typedef struct {
	int row; /// current row idx
	int col; /// current col idx
	int vrow_num; /// viewable row
	int vcol_num; /// viewable col
	int line_num; /// all the line num
	int start_line; /// start line num idx
	const char *str[MAX_MENU_LINE]; /// str to show
} MENU;

/* used in ITEMS_MENU, flag: 0: view, 1: set */
typedef void (*ITEM_FUNC)(BYTE flag, void *para, const char *info);

/* used to select menu, from main menu to submenu */
typedef struct {
	MENU menu;
	int cur_line;
	ITEM_FUNC func[MAX_MENU_LINE];
	void *para[MAX_MENU_LINE];
	BYTE flag[MAX_MENU_LINE]; /* 0: can't set the item; 1: view and set. */
} ITEMS_MENU;

/* used in PARAM_LIST */
typedef struct {
	int row;
	int col;
	WORD digit_num;
	BYTE value_idx[MAX_INPUT_DIGIT_NUM];
	WORD list_num;
	const char *const_list[MAX_LIST_NUM];
	WORD max_value;
} INPUT_LIST;

/* used to list information of the concentrator */
typedef struct {
	MENU menu;
	WORD group_num;
	int group_idx;
	int digit_idx;
	INPUT_LIST list[MAX_INPUT_GROUP_NUM];
	/* flag, 1: several digits are a union, such as "645",
	 0: several digits are alone */
	BYTE flag[MAX_INPUT_GROUP_NUM];
} PARAM_LIST;

/* used to list document information */
typedef struct {
	int row;
	WORD idx_num;
	int start_idx;
	int cur_idx;
	MENU menu;
} DOC_LIST; /// document list

/* used in PARAM_SET */
typedef struct {
	int row;
	int col;
	int len;
	char str[MAX_INPUT_STRING_LEN];
	int list_idx;
	int list_num;
	const char *const_list[MAX_LIST_NUM];
	int maxlen;
} INPUT_STRING; /// input_string for param_set

typedef enum {
	_num_keyboard = 0,
	_lower_letter_keyboard,
	_upper_letter_keyboard,
	_symbol_keyboard,
	_direction_keyboard,
	_calendar_keyboard,
	_none_keyboard,
} keyboard_t; /// key board type

/* used to set parameters of concentrator */
typedef struct {
	MENU menu;
	WORD group_num;
	int group_idx;
	INPUT_STRING input[MAX_INPUT_GROUP_NUM]; /// input string
	keyboard_t keyboard_type[MAX_INPUT_GROUP_NUM];
} PARAM_SET;

void lcd_update_lock(void);

void lcd_update_unlock(void);

int lcd_is_ok(void);

void lcd_open(const char *device, int lcd_type, int font_size);

void lcd_close(void);

void lcd_position_left(int offset);

void lcd_position_right(int offset);

void lcd_show_cursor(int row, int col, BYTE mode);

void lcd_show_underline(int row, int col1, int col2, int pixel);

void lcd_show_string(int row, int col, int len, const void *buf);

void lcd_reverse_string(int row, int col1, int col2);

int lcd_save_window(int row1, int col1, int row2, int col2, void **buf,
		int *len);

int lcd_restore_window(int row1, int col1, int row2, int col2, void *buf,
		int len);

int lcd_screen_row(void);

int lcd_screen_col(void);

void lcd_clear_screen(void);

void lcd_show_screen(BYTE *buf);

void lcd_show_lines(void);

void lcd_update_head_info(void);

void lcd_update_info(const char *buf);

int lcd_key_in(int flag);

void lcd_clean_workspace(void);

void lcd_show_arrow(int up, int down, int left, int right);

void lcd_write_init_cmd(void);

enum key_type_t {
	KEY_PRESS_DOWN = 0x0001,
	KEY_PRESS_SHORT_UP = 0x0002,
	KEY_PRESS_LONG_UP = 0x0004,
};
struct key_msg_t {
	unsigned int code;
	enum key_type_t type;
};
int KeyOpen(void);
int KeyClose(void);
int KeyRead(struct key_msg_t *msg);
int KeyRead_NONE_BLOCKING(struct key_msg_t *msg);

#endif /* LCD_H_ */

/*
 * lcd.c
 *
 *  Created on: 2015-8-11
 *      Author: Johnnyzhang
 */

//#include <key_hw.h.BAK>
#include "lcd.h"
#include "main.h"
#include "common.h"
#include "language.h"
#include "gpio.h"
#include "font.h"
#include "atcmd.h"
#include "f_con_alarm.h"
#include "f_param.h"
#include "input.h"
#include "gprscdma.h"
#include "devices.h"

#define LCD_WIDTH			scr.width
#define LCD_HEIGHT			scr.height
#define LCD_MAX_ROW			scr.max_row
#define LCD_MAX_COL			scr.max_col
#define LCD_FONT_SIZE		scr.font_size
#define LCD_TOP_POS			scr.top_pos
#define LCD_LEFT_POS		scr.left_pos
#define LCD_ROW_SPACE		scr.row_space
#define LCD_ICON			scr.icon
#define LCD_BUF				scr.buf
#define LCD_FD				scr.fd
#define LCD_INIT_CMD		scr.init_cmd
#define LCD_REFRESH_CMD		scr.refresh_cmd

static void lcd_init_cmd_0(void);
static void lcd_init_cmd_1(void);
static void lcd_refresh_cmd_0(int x, int y, int w, int h);
static void lcd_refresh_cmd_1(int x, int y, int w, int h);

static const unsigned char icon_16[32 * 12] = { /// 12 icons
		/* signal sensitive 1, ICON_0 */
		0x00, 0x0E, 0x00, 0x0A, 0x00, 0x0A, 0x00, 0x0A, 0x00, 0xEA, 0x00, 0xAA,
				0x00, 0xAA, 0x00, 0xAA, 0x0E, 0xAA, 0x0A, 0xAA, 0x0A, 0xAA,
				0x0A, 0xAA, 0xEA, 0xAA, 0xEA, 0xAA, 0xEA, 0xAA, 0xEE, 0xEE,

				/* signal sensitive 2, ICON_1 */
				0x00, 0x0E, 0x00, 0x0A, 0x00, 0x0A, 0x00, 0x0A, 0x00, 0xEA,
				0x00, 0xAA, 0x00, 0xAA, 0x00, 0xAA, 0x0E, 0xAA, 0x0E, 0xAA,
				0x0E, 0xAA, 0x0E, 0xAA, 0xEE, 0xAA, 0xEE, 0xAA, 0xEE, 0xAA,
				0xEE, 0xEE,

				/* signal sensitive 3, ICON_2 */
				0x00, 0x0E, 0x00, 0x0A, 0x00, 0x0A, 0x00, 0x0A, 0x00, 0xEA,
				0x00, 0xEA, 0x00, 0xEA, 0x00, 0xEA, 0x0E, 0xEA, 0x0E, 0xEA,
				0x0E, 0xEA, 0x0E, 0xEA, 0xEE, 0xEA, 0xEE, 0xEA, 0xEE, 0xEA,
				0xEE, 0xEE,

				/* signal sensitive 4, ICON_3 */
				0x00, 0x0E, 0x00, 0x0E, 0x00, 0x0E, 0x00, 0x0E, 0x00, 0xEE,
				0x00, 0xEE, 0x00, 0xEE, 0x00, 0xEE, 0x0E, 0xEE, 0x0E, 0xEE,
				0x0E, 0xEE, 0x0E, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE,
				0xEE, 0xEE,

				/* net type G, ICON_4 */
				0xFF, 0xFF, 0x80, 0x01, 0x80, 0x01, 0x83, 0xD1, 0x84, 0x31,
				0x88, 0x11, 0x88, 0x01, 0x88, 0x01, 0x88, 0x39, 0x88, 0x11,
				0x88, 0x11, 0x84, 0x31, 0x83, 0xD1, 0x80, 0x01, 0x80, 0x01,
				0xFF, 0xFF,

				/* net type C, ICON_5 */
				0xFF, 0xFF, 0x80, 0x01, 0x80, 0x01, 0x83, 0xD1, 0x84, 0x31,
				0x88, 0x11, 0x88, 0x01, 0x88, 0x01, 0x88, 0x01, 0x88, 0x01,
				0x88, 0x09, 0x84, 0x11, 0x83, 0xE1, 0x80, 0x01, 0x80, 0x01,
				0xFF, 0xFF,

				/* connected, ICON_6 */
				0x07, 0xE0, 0x07, 0xE0, 0x07, 0xE0, 0x07, 0xE0, 0x07, 0xE0,
				0x01, 0x80, 0x01, 0x80, 0x02, 0x40, 0x04, 0x20, 0x08, 0x10,
				0x10, 0x08, 0xF8, 0x1F, 0xF8, 0x1F, 0xF8, 0x1F, 0xF8, 0x1F,
				0xF8, 0x1F,

				/* alarm, ICON_7 */
				0x07, 0xE0, 0x08, 0x10, 0x31, 0x8C, 0x23, 0xC4, 0x43, 0xC2,
				0x83, 0xC1, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,
				0x81, 0x81, 0x40, 0x02, 0x21, 0x84, 0x31, 0x8C, 0x08, 0x10,
				0x07, 0xE0,

				/* up/down arrow, ICON_8 */
				0x00, 0x80, 0x01, 0xC0, 0x01, 0xC0, 0x03, 0xE0, 0x03, 0xE0,
				0x04, 0x90, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80,
				0x04, 0x90, 0x03, 0xE0, 0x03, 0xE0, 0x01, 0xC0, 0x01, 0xC0,
				0x00, 0x80,

				/* left/right arrow, ICON_9 */
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x20,
				0x18, 0x18, 0x78, 0x1E, 0xFF, 0xFF, 0x78, 0x1E, 0x18, 0x18,
				0x04, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00,

				/* up/down/left/right arrow, ICON_10 */
				0x00, 0x80, 0x01, 0xC0, 0x03, 0xE0, 0x00, 0x80, 0x00, 0x80,
				0x10, 0x84, 0x30, 0x86, 0x7F, 0xFF, 0x30, 0x86, 0x10, 0x84,
				0x00, 0x80, 0x00, 0x80, 0x03, 0xE0, 0x01, 0xC0, 0x00, 0x80,
				0x00, 0x00,

				/* PROGRAM STATUS E, ICON_11 */
				0xFF, 0xFF, 0x80, 0x01, 0x9F, 0xF9, 0x9F, 0xF9, 0x98, 0x19,
				0x98, 0x01, 0x98, 0x01, 0x9F, 0xE1, 0x9F, 0xE1, 0x98, 0x01,
				0x98, 0x01, 0x98, 0x19, 0x9F, 0xF9, 0x9F, 0xF9, 0x80, 0x01,
				0xFF, 0xFF, };

static const unsigned char icon_12[24 * 12] =
		{
		/* signal sensitive 1, ICON_0 */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x00, 0xC0, 0x00,
				0xC0, 0x00,

				/* signal sensitive 2, ICON_1 */
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x18, 0x00, 0x18, 0x00, 0x18, 0x00, 0xD8, 0x00,
				0xD8, 0x00, 0xD8, 0x00,

				/* signal sensitive 3, ICON_2 */
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x03, 0x00,
				0x03, 0x00, 0x1B, 0x00, 0x1B, 0x00, 0x1B, 0x00, 0xDB, 0x00,
				0xDB, 0x00, 0xDB, 0x00,

				/* signal sensitive 4, ICON_3 */
				0x00, 0x60, 0x00, 0x60, 0x00, 0x60, 0x03, 0x60, 0x03, 0x60,
				0x03, 0x60, 0x1B, 0x60, 0x1B, 0x60, 0x1B, 0x60, 0xDB, 0x60,
				0xDB, 0x60, 0xDB, 0x60,

				/* net type G, ICON_4 */
				0xFF, 0xF0, 0x80, 0x10, 0x8F, 0x10, 0x99, 0x90, 0xB0, 0x90,
				0xB0, 0x10, 0xB7, 0x90, 0xB1, 0x90, 0x99, 0x90, 0x8E, 0x90,
				0x80, 0x10, 0xFF, 0xF0,

				/* net type C, ICON_5 */
				0xFF, 0xF0, 0x80, 0x10, 0x8F, 0x10, 0x99, 0x90, 0xB0, 0x90,
				0xB0, 0x10, 0xB0, 0x10, 0xB0, 0x90, 0x99, 0x90, 0x8F, 0x10,
				0x80, 0x10, 0xFF, 0xF0,

				/* connected, ICON_6 */
				0x0F, 0x00, 0x0F, 0x00, 0x0F, 0x00, 0x0F, 0x00, 0x06, 0x00,
				0x09, 0x00, 0x10, 0x80, 0x20, 0x40, 0xF0, 0xF0, 0xF0, 0xF0,
				0xF0, 0xF0, 0xF0, 0xF0,

				/* alarm, ICON_7 */
				0x1F, 0x80, 0x20, 0x40, 0x46, 0x20, 0x86, 0x10, 0x86, 0x10,
				0x86, 0x10, 0x86, 0x10, 0x80, 0x10, 0x86, 0x10, 0x46, 0x20,
				0x20, 0x40, 0x1F, 0x80,

				/* up/down arrow, ICON_8 */
				0x04, 0x00, 0x0E, 0x00, 0x1F, 0x00, 0x04, 0x00, 0x04, 0x00,
				0x04, 0x00, 0x04, 0x00, 0x04, 0x00, 0x04, 0x00, 0x1F, 0x00,
				0x0E, 0x00, 0x04, 0x00,

				/* left/right arrow, ICON_9 */
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x40, 0x60, 0x60,
				0xFF, 0xF0, 0x60, 0x60, 0x20, 0x40, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00,

				/* up/down/left/right arrow, ICON_10 */
				0x04, 0x00, 0x0E, 0x00, 0x04, 0x00, 0x04, 0x00, 0x44, 0x40,
				0xFF, 0xE0, 0x44, 0x40, 0x04, 0x00, 0x04, 0x00, 0x0E, 0x00,
				0x04, 0x00, 0x00, 0x00,

				/* PROGRAM STATUS E, ICON_11 */
				0xFF, 0xF0, 0x80, 0x10, 0x80, 0x10, 0x9F, 0x90, 0x90, 0x90,
				0x90, 0x10, 0x9F, 0x10, 0x90, 0x10, 0x90, 0x90, 0x9F, 0x90,
				0x80, 0x10, 0xFF, 0xF0, };

static struct {
	int fd;
	unsigned char *buf;
	short width, height, max_row, max_col;
	short font_size, top_pos, left_pos, row_space;
	const unsigned char *icon;
	void (*init_cmd)(void);
	void (*refresh_cmd)(int x, int y, int w, int h);
} scr = { .fd = -1, .buf = NULL, };

static sem_t sem_t_lcd, sem_t_lcd_update;

static void lcd_lock(void);
static void lcd_unlock(void);

static void lcd_cmd(unsigned char *cmd, int len) {
	int ret;

	if (LCD_FD >= 0 && len > 0) {
		lcd_lock();
		ret = write(LCD_FD, cmd, len);
		if (ret < len) {
			PRINTF("File: %s, Function: %s, Line: %d, ret: %d\n", __FILE__,
					__FUNCTION__, __LINE__, ret);
		}
		lcd_unlock();
	}
}

static void lcd_init_cmd_0(void) {
	unsigned char buf[128], *ptr;

	ptr = buf;
	*ptr++ = 0x00;
	*ptr++ = 0x30; // Ext = 0
	*ptr++ = 0x00;
	*ptr++ = 0x94; // Sleep Out
	*ptr++ = 0x00;
	*ptr++ = 0xD1; // OSC On
	*ptr++ = 0x00;
	*ptr++ = 0xCA; // Display Control, Changed for LCM
	*ptr++ = 0x01;
	*ptr++ = 0x04;
	*ptr++ = 0x01;
	*ptr++ = 0x27;
	*ptr++ = 0x01;
	*ptr++ = 0x00;
	*ptr++ = 0x00;
	*ptr++ = 0x81; // Electronic Volumn Control
	*ptr++ = 0x01;
	*ptr++ = 0x1A;
	*ptr++ = 0x01;
	*ptr++ = 0x04;
	*ptr++ = 0x00;
	*ptr++ = 0x20; // Power Control Set
	*ptr++ = 0x01;
	*ptr++ = 0x0B;
	*ptr++ = 0x00;
	*ptr++ = 0xBB; // COM Scan Direction
	*ptr++ = 0x01;
	*ptr++ = 0x02;
	*ptr++ = 0x00;
	*ptr++ = 0xA7; // Inverse Display
	*ptr++ = 0x00;
	*ptr++ = 0xBC; // Data Scan Direction, 3Byte 3pixel mode
	*ptr++ = 0x01;
	*ptr++ = 0x02;
	*ptr++ = 0x01;
	*ptr++ = 0x01;
	*ptr++ = 0x01;
	*ptr++ = 0x02;
	*ptr++ = 0x00;
	*ptr++ = 0xAF; // Display On
	lcd_cmd(buf, ptr - buf);

	ptr = buf;
	*ptr++ = 0x00;
	*ptr++ = 0x31; // Ext = 1
	*ptr++ = 0x00;
	*ptr++ = 0x32; // Analog Circuit Set, Changed for LCM
	*ptr++ = 0x01;
	*ptr++ = 0x06;
	*ptr++ = 0x01;
	*ptr++ = 0x00;
	*ptr++ = 0x01;
	*ptr++ = 0x03;
	*ptr++ = 0x00;
	*ptr++ = 0x20; // Set Odd Frame Gray PWM Set
	*ptr++ = 0x01;
	*ptr++ = 0x00;
	*ptr++ = 0x01;
	*ptr++ = 0x02;
	*ptr++ = 0x01;
	*ptr++ = 0x04;
	*ptr++ = 0x01;
	*ptr++ = 0x06;
	*ptr++ = 0x01;
	*ptr++ = 0x08;
	*ptr++ = 0x01;
	*ptr++ = 0x0A;
	*ptr++ = 0x01;
	*ptr++ = 0x0C;
	*ptr++ = 0x01;
	*ptr++ = 0x0E;
	*ptr++ = 0x01;
	*ptr++ = 0x10;
	*ptr++ = 0x01;
	*ptr++ = 0x12;
	*ptr++ = 0x01;
	*ptr++ = 0x14;
	*ptr++ = 0x01;
	*ptr++ = 0x16;
	*ptr++ = 0x01;
	*ptr++ = 0x18;
	*ptr++ = 0x01;
	*ptr++ = 0x1A;
	*ptr++ = 0x01;
	*ptr++ = 0x1C;
	*ptr++ = 0x01;
	*ptr++ = 0x1E;
	*ptr++ = 0x00;
	*ptr++ = 0x21; // Set Even Frame Gray PWM Set
	*ptr++ = 0x01;
	*ptr++ = 0x01;
	*ptr++ = 0x01;
	*ptr++ = 0x03;
	*ptr++ = 0x01;
	*ptr++ = 0x04;
	*ptr++ = 0x01;
	*ptr++ = 0x07;
	*ptr++ = 0x01;
	*ptr++ = 0x09;
	*ptr++ = 0x01;
	*ptr++ = 0x0B;
	*ptr++ = 0x01;
	*ptr++ = 0x0D;
	*ptr++ = 0x01;
	*ptr++ = 0x0F;
	*ptr++ = 0x01;
	*ptr++ = 0x11;
	*ptr++ = 0x01;
	*ptr++ = 0x13;
	*ptr++ = 0x01;
	*ptr++ = 0x15;
	*ptr++ = 0x01;
	*ptr++ = 0x17;
	*ptr++ = 0x01;
	*ptr++ = 0x19;
	*ptr++ = 0x01;
	*ptr++ = 0x1B;
	*ptr++ = 0x01;
	*ptr++ = 0x1D;
	*ptr++ = 0x01;
	*ptr++ = 0x1F;
	*ptr++ = 0x00;
	*ptr++ = 0x30; // Ext = 0
	lcd_cmd(buf, ptr - buf);
}

static void lcd_init_cmd_1(void) {
	unsigned char buf[128], *ptr;

	 /*//for test command the kernel
	 ptr = buf;
	 printf("start %s\n", __func__);
	 *ptr ++ = 0x00; *ptr ++ = 0x05;
	 *ptr ++ = 0x00; *ptr ++ = 0x12;
	 *ptr ++ = 0x00; *ptr ++ = 0x60;
	 *ptr ++ = 0x00; *ptr ++ = 0x70;
	 *ptr ++ = 0x01; *ptr ++ = 0xff;
	 *ptr ++ = 0x01; *ptr ++ = 0xff;
	 *ptr ++ = 0x01; *ptr ++ = 0xff;
	 *ptr ++ = 0x03;
	 lcd_cmd(buf, ptr - buf);
	 msleep(5000);
	 */

	/*  // cancel reset  by nayo wang, for lcd has been initiated in
	 *the kernel. Re-initiation cause inconvenience from screen.

	device_lcd_reset();
	ptr = buf;
	*ptr++ = 0x00;
	*ptr++ = 0xE2; // System Set
	*ptr++ = 0x03; // add by nayowang
	lcd_cmd(buf, ptr - buf);

	msleep(200);

	ptr = buf;
	*ptr++ = 0x00;
	*ptr++ = 0x27; // Set Temp. command
	*ptr++ = 0x00;
	*ptr++ = 0x2B; // Internal pump
	*ptr++ = 0x00;
	*ptr++ = 0xC4; // Set lcd mapping control
	*ptr++ = 0x00;
	*ptr++ = 0xA1; // Set line rate
	*ptr++ = 0x00;
	*ptr++ = 0xC8; // Set n-line
	*ptr++ = 0x00;
	*ptr++ = 0x1F;
	*ptr++ = 0x00;
	*ptr++ = 0xD1; // Set color pattern RGB
	*ptr++ = 0x00;
	*ptr++ = 0xD5; // Set 4K color mode
	*ptr++ = 0x00;
	*ptr++ = 0xE9; // Set lcd bias ratio
	*ptr++ = 0x00;
	*ptr++ = 0x81; // Set Vbias potentiometer
	*ptr++ = 0x00;
	*ptr++ = 0x99;
	*ptr++ = 0x00;
	*ptr++ = 0xDE; // Set com scan function
	*ptr++ = 0x00;
	*ptr++ = 0x88; // Set RAM address control
	*ptr++ = 0x00;
	*ptr++ = 0xAD; // Display enable
	*ptr++ = 0x03; // add by nayowang
	lcd_cmd(buf, ptr - buf);

	*/
}

static void lcd_refresh_cmd_0(int x, int y, int w, int h) {
	unsigned char buf[2048], *ptr;
	unsigned char *data;
	int i, j, addr, off, bit;

	if (LCD_BUF != NULL) {
		data = LCD_BUF;
		h = y + h - 1;
		if (h >= LCD_HEIGHT)
			h = LCD_HEIGHT - 1;
		x = 0;
		w = LCD_WIDTH - 1;
		for (i = y; i <= h; i++) {
			ptr = buf;
			*ptr++ = 0x00;
			*ptr++ = 0x15; // Column Address Set
			*ptr++ = 0x01;
			*ptr++ = 84 - (LCD_WIDTH / 3);
			*ptr++ = 0x01;
			*ptr++ = 84;
			*ptr++ = 0x00;
			*ptr++ = 0x75; // Line Address Set
			*ptr++ = 0x01;
			*ptr++ = i;
			*ptr++ = 0x01;
			*ptr++ = i + 1;
			*ptr++ = 0x00;
			*ptr++ = 0x5C; // Entry Memory Write Mode
			switch (LCD_WIDTH % 3) {
			case 1:
				*ptr++ = 0x01;
				*ptr++ = 0x00;
				*ptr++ = 0x01;
				*ptr++ = 0x00;
				break;
			case 2:
				*ptr++ = 0x01;
				*ptr++ = 0x00;
				break;
			case 0:
				break;
			}
			addr = i * LCD_WIDTH + x;
			for (j = x; j <= w; j++) {
				off = addr / 8;
				bit = addr % 8;
				*ptr++ = 0x01;
				if (data[off] & (1 << bit))
					*ptr++ = 0xf8;
				else
					*ptr++ = 0x00;
				addr++;
			}
			lcd_cmd(buf, ptr - buf);
		}
	}
}

static BYTE lcd_position = 0x05;



#ifdef IMX28

static void lcd_refresh_cmd_1(int x, int y, int w, int h) {
	unsigned char buf[2048], *ptr, val1, val2, val3;
	const unsigned char *data;
	int i, j, addr, off1, bit1, off2, bit2;

	if (LCD_BUF != NULL) {
		data = LCD_BUF;
		h = y + h - 1;
		if (h >= LCD_HEIGHT)
			h = LCD_HEIGHT - 1;
		x = 0;
		w = LCD_WIDTH - 1;
		for (i = y; i <= h; i++) {
			ptr = buf;
			*ptr++ = 0x00;
			*ptr++ = lcd_position;
			*ptr++ = 0x00;
			*ptr++ = 0x12;
			*ptr++ = 0x00;
			*ptr++ = 0x60 | (i & 0x0f);
			*ptr++ = 0x00;
			*ptr++ = 0x70 | ((i >> 4) & 0x0f);
			addr = i * LCD_WIDTH + x;
			for (j = 0; j < LCD_WIDTH / 2; j++) {
				off1 = addr / 8;
				bit1 = addr % 8;
				off2 = (addr + 1) / 8;
				bit2 = (addr + 1) % 8;
				val1 = (data[off1] & (1 << bit1)) ? 0x0f : 0x00;
				val2 = (data[off2] & (1 << bit2)) ? 0x0f : 0x00;
				val3 = (val1 << 4) | val2;
				*ptr++ = 0x01;
				*ptr++ = val3;
				addr += 2;
			}
			*ptr++ = 0x01;
			*ptr++ = 0x00;
			*ptr++ = 0x03;
			lcd_cmd(buf, ptr - buf);
		}
	}
}

#elif defined(AM335X)

#include "lcd_hw.h"
static void lcd_refresh_byte( int x, int y, unsigned char byte )
{
	int i = 0;
	//--------------------------------------------------------------
	for ( i=0; i<8; i++ ) {

		if ( byte & 0x01 ) {

			lcd_set_current_color(LCD_WHITE);
		} else {

			lcd_set_current_color(LCD_BLACK);
		}

		draw_dot( x, y, 1 );

		x++;

		byte >>= 1;

		///printf("lcd_refresh_byte called\n");


	}

	lcd_set_current_color(LCD_WHITE);
}

void lcd_clrsrn(void)
{
	int x = 0;
	int y = 0;

	for ( y=0; y<LCD_HEIGHT; y++ ) {

		for ( x=0; x<LCD_WIDTH; x++ ) {

			draw_dot( x, y, 1 );
		}
	}

}

static void lcd_refresh_cmd_1(int x, int y, int w, int h)
{

	//int i, j, k, addr, off1, bit1, off2, bit2;

	//unsigned int y1 = 0;
	unsigned int uwIndx = 0;
	unsigned int uwSize = 0;

	int x1 = 0;
	int x2 = 0;
	//int y1 = 0;
	unsigned char i = 0;
	unsigned char k = 0;
	unsigned char *p = NULL;
	unsigned char *ucLcdBuffer = NULL;
	//-----------------------------------------------------------------------------------
	//x = 0 y = 0 w = 160 h = 160

	if (LCD_BUF != NULL) {

	x1 = 0; x2 = LCD_WIDTH;

		w = (x2 - x1);

		if ( (y + h) > LCD_HEIGHT ) {

			h = (LCD_HEIGHT - y);
		}

		uwSize = ( h * (w / 8) );

		ucLcdBuffer = malloc(uwSize);

		if ( ucLcdBuffer == NULL ) {

			return;
		}

		p = ucLcdBuffer;

		for ( i=0; i<h; i++ ) {

			uwIndx = ( ( (y + i) * (LCD_WIDTH / 8) ) + ( x1 / 8 ) ); /// block buffer start, left-up conner

			for ( k=0; k<(w/8); k++ ) {

				*p++ = LCD_BUF[uwIndx + k]; ///
			}

		}

		LCD_DrawPicture_1( x1, y, w, h, ucLcdBuffer );

		free(ucLcdBuffer);

	} else {
		printf("lcd.c: LCD_BUFF = NULL\n");
	}

}

int get_screen_fd(void){
	return LCD_FD;
}

#endif /// AM335X


static void lcd_refresh(int x, int y, int w, int h) {
	(*LCD_REFRESH_CMD)(x, y, w, h);
}

static void lcd_swap(int *val1, int *val2) {
	int tmp;

	tmp = *val1;
	*val1 = *val2;
	*val2 = tmp;
}

static int lcd_check_x_y(int x, int y) {
	return x >= 0 && x < LCD_WIDTH && y >= 0 && y < LCD_HEIGHT;
}

static int lcd_check_row_col(int row, int col) {
	return row > 0 && row <= MAX_SCREEN_ROW && col > 0 && col <= MAX_SCREEN_COL;
}

static int lcd_valid_x_y(int *x1, int *y1, int *x2, int *y2) {
	if (lcd_check_x_y(*x1, *y1) && lcd_check_x_y(*x2, *y2)) {
		if (*x1 > *x2)
			lcd_swap(x1, x2);
		if (*y1 > *y2)
			lcd_swap(y1, y2);
		return 1;
	}
	return 0;
}

static int lcd_valid_row_col(int *row1, int *col1, int *row2, int *col2) {
	if (lcd_check_row_col(*row1, *col1) && lcd_check_row_col(*row2, *col2)) {
		if (*row1 > *row2)
			lcd_swap(row1, row2);
		if (*col1 > *col2)
			lcd_swap(col1, col2);
		return 1;
	}
	return 0;
}

static int col_to_x(int col) {
	return LCD_LEFT_POS + (col - 1) * (LCD_FONT_SIZE / 2);
}

static int row_to_y(int row) {
	int adjust;

	if (row == 1)
		adjust = -1;
	else if (row == MAX_SCREEN_ROW)
		adjust = 1;
	else
		adjust = 0;
	return LCD_TOP_POS + (row - 1) * (LCD_FONT_SIZE + LCD_ROW_SPACE) + adjust;
}

static void lcd_show_pixel(int x, int y, int pixel) {
	int addr, off, bit;
	unsigned char *ptr, ch;

	if (lcd_check_x_y(x, y) && LCD_BUF != NULL) {
		addr = y * LCD_WIDTH + x;
		off = addr / 8;
		bit = addr % 8;
		ptr = LCD_BUF + off;
		ch = *ptr;
		if (pixel)
			ch = ch | (1 << bit);
		else
			ch = ch & (~(1 << bit));
		*ptr = ch;
	}
}

static void lcd_show_icon(int row, int col, int icon) {
	int i, j, x, y;
	const unsigned char *ptr;
	unsigned short word;

	x = col_to_x(col);
	y = row_to_y(row);
	ptr = LCD_ICON + icon * (LCD_FONT_SIZE * 2);
	for (i = 0; i < LCD_FONT_SIZE; i++) {
		word = (ptr[0] << 8) + ptr[1];
		for (j = 0; j < LCD_FONT_SIZE; j++) {
			lcd_show_pixel(x + j, y + i, (word & 0x8000) ? 1 : 0);
			word <<= 1;
		}
		ptr += 2;
	}
}

static void lcd_hide_icon(int row, int col, int icon) {
	int i, j, x, y;

	x = col_to_x(col);
	y = row_to_y(row);
	for (i = 0; i < LCD_FONT_SIZE; i++) {
		for (j = 0; j < LCD_FONT_SIZE; j++) {
			lcd_show_pixel(x + j, y + i, 0);
		}
	}
}

static void lcd_lock(void) {
	sem_wait(&sem_t_lcd);
}

static void lcd_unlock(void) {
	int ret;

	ret = sem_post(&sem_t_lcd);
}

void lcd_update_lock(void) {
	sem_wait(&sem_t_lcd_update);
}

void lcd_update_unlock(void) {
	int ret;

	ret = sem_post(&sem_t_lcd_update);
}

void lcd_open(const char *device, int lcd_type, int font_size) {
	unsigned char *buf;
	int m, n;

	sem_init(&sem_t_lcd, 0, 1);
	sem_init(&sem_t_lcd_update, 0, 1);
	if (lcd_type == 0) {
		LCD_WIDTH = LCD_HEIGHT = 160;
		LCD_INIT_CMD = lcd_init_cmd_0;
		LCD_REFRESH_CMD = lcd_refresh_cmd_0;
	} else {
		LCD_WIDTH = LCD_HEIGHT = 160;
		LCD_INIT_CMD = lcd_init_cmd_1;
		LCD_REFRESH_CMD = lcd_refresh_cmd_1;
	}
	if (font_size == 16) {
		LCD_FONT_SIZE = 16;
		LCD_ICON = icon_16;
	} else {
		LCD_FONT_SIZE = 12;
		LCD_ICON = icon_12;
	}
	LCD_ROW_SPACE = 1;
	while (1) {
		m = LCD_HEIGHT % (LCD_FONT_SIZE + LCD_ROW_SPACE);
		n = LCD_HEIGHT / (LCD_FONT_SIZE + LCD_ROW_SPACE);
		if (m > n)
			LCD_ROW_SPACE = LCD_ROW_SPACE + 1;
		else
			break;
	}
	LCD_TOP_POS = (LCD_HEIGHT % (LCD_FONT_SIZE + LCD_ROW_SPACE)) / 2;
	LCD_LEFT_POS = (LCD_WIDTH % LCD_FONT_SIZE) / 2;
	LCD_MAX_COL = (LCD_WIDTH / LCD_FONT_SIZE) * 2;
	LCD_MAX_ROW = LCD_HEIGHT / (LCD_FONT_SIZE + LCD_ROW_SPACE);
	LOG_PRINTF_ALLOCATE(
			"Before malloc() in func: %s, line: %d, allocation size: %d\n",
			__func__, __LINE__, LCD_WIDTH * LCD_HEIGHT / 8);
	if ((buf = malloc(LCD_WIDTH * LCD_HEIGHT / 8)) != NULL) {
		LOG_PRINTF_ALLOCATE("After malloc() in func: %s, line: %d\n", __func__,
				__LINE__);
		if (font_init(LCD_FONT_SIZE))
			LCD_BUF = buf;
		else {
			LOG_PRINTF_ALLOCATE("free() in func: %s, line: %d\n", __func__,
					__LINE__);
			free(buf);
		}
	}
	if (LCD_BUF != NULL)
		PRINTF("Open font%d ok, type:%d\n", LCD_FONT_SIZE, lcd_type);
	else
		PRINTF("Open font%d fail, type:%d\n", LCD_FONT_SIZE, lcd_type);
	if ((LCD_FD = open(device, O_RDWR)) >= 0) {
		(*LCD_INIT_CMD)();
		lcd_clear_screen();
		device_lcd_light(1);
		PRINTF("Open %s ok, fd:%d font:%d row:%d col:%d left:%d top:%d\n",
				device, LCD_FD, LCD_FONT_SIZE, MAX_SCREEN_ROW, MAX_SCREEN_COL,
				LCD_LEFT_POS, LCD_TOP_POS);
	} else {
		PRINTF("Open %s fail\n", device);
	}
}

void lcd_close(void) {
	if (LCD_BUF != NULL) {
		font_destroy();
		LOG_PRINTF_ALLOCATE("free() in func: %s, line: %d\n", __func__,
				__LINE__);
		free(LCD_BUF);
		LCD_BUF = NULL;
	}
	if (LCD_FD >= 0) {
		device_lcd_light(0);
		close(LCD_FD);
		LCD_FD = -1;
	}
	sem_destroy(&sem_t_lcd_update);
	sem_destroy(&sem_t_lcd);
}

void lcd_position_left(int offset) {
	BYTE data;

	if (lcd_position - offset >= 0x03) {
		lcd_position -= offset;
		lcd_refresh(0, 0, LCD_WIDTH, LCD_HEIGHT);
		data = lcd_position;
	}
}

void lcd_position_right(int offset) {
	BYTE data;

	if (lcd_position + offset <= 0x05) {
		lcd_position += offset;
		lcd_refresh(0, 0, LCD_WIDTH, LCD_HEIGHT);
		data = lcd_position;
	}
}

int lcd_is_ok(void) {
	return LCD_FD >= 0;
}

int lcd_screen_row(void) {
	return LCD_MAX_ROW;
}

int lcd_screen_col(void) {
	return LCD_MAX_COL;
}

void lcd_show_cursor(int row, int col, BYTE mode) {
	mode = (mode == 0) ? 1 : mode;
	lcd_reverse_string(row, col, col + mode - 1);
}

static void lcd_show_line(int start_x, int start_y, int end_x, int end_y,
		int pixel) {
	int i, j;

	if (!lcd_check_x_y(start_x, start_y) || !lcd_check_x_y(end_x, end_y))
		return;
	if (start_x == end_x) { // vert line
		if (start_y > end_y) {
			j = start_y;
			start_y = end_y;
			end_y = j;
		}
		for (i = start_y; i <= end_y; i++)
			lcd_show_pixel(start_x, i, pixel);
		lcd_refresh(start_x, start_y, end_x - start_x + 1, end_y - start_y + 1);
		return;
	}
	if (start_y == end_y) { // horz line
		if (start_x > end_x) {
			j = start_x;
			start_x = end_x;
			end_x = j;
		}
		for (i = start_x; i <= end_x; i++)
			lcd_show_pixel(i, start_y, pixel);
		lcd_refresh(start_x, start_y, end_x - start_x + 1, end_y - start_y + 1);
		return;
	}
	// bias line
	if (abs(end_x - start_x) > abs(end_y - start_y)) {
		if (start_y < end_y) {
			for (i = start_y; i <= end_y; i++) {
				j = (end_x - start_x) * (i - start_y) / (end_y - start_y)
						+ start_x;
				lcd_show_pixel(j, i, pixel);
			}
		} else {
			for (i = start_y; i >= end_y; i--) {
				j = (end_x - start_x) * (i - start_y) / (end_y - start_y)
						+ start_x;
				lcd_show_pixel(j, i, pixel);
			}
		}
	} else {
		if (start_x < end_x) {
			for (i = start_x; i <= end_x; i++) {
				j = (end_y - start_y) * (i - start_x) / (end_x - start_x)
						+ start_y;
				lcd_show_pixel(i, j, pixel);
			}
		} else {
			for (i = start_x; i >= end_x; i--) {
				j = (end_y - start_y) * (i - start_x) / (end_x - start_x)
						+ start_y;
				lcd_show_pixel(i, j, pixel);
			}
		}
	}
	if (start_x > end_x) {
		j = start_x;
		start_x = end_x;
		end_x = j;
	}
	if (start_y > end_y) {
		j = start_y;
		start_y = end_y;
		end_y = j;
	}
	lcd_refresh(start_x, start_y, end_x - start_x + 1, end_y - start_y + 1);
}

static void lcd_show_rect(int start_x, int start_y, int end_x, int end_y,
		int pixel) {
	lcd_show_line(start_x, start_y, end_x, start_y, pixel);
	lcd_show_line(start_x, end_y, end_x, end_y, pixel);
	lcd_show_line(start_x, start_y, start_x, end_y, pixel);
	lcd_show_line(end_x, start_y, end_x, end_y, pixel);
}

void lcd_show_string(int row, int col, int len, const void *buf) {
	int x, y, w;

	len = min(len, MAX_SCREEN_COL);
	if (lcd_check_row_col(row, col)) {
		x = col_to_x(col);
		y = row_to_y(row);
		w = text_out(x, y, buf, len, lcd_show_pixel);
		lcd_refresh(x, y, w, LCD_FONT_SIZE);
	}
}

void lcd_show_underline(int row, int col1, int col2, int pixel) {
	int x1, x2, y1, y2;

	if (lcd_valid_row_col(&row, &col1, &row, &col2)) {
		x1 = col_to_x(col1);
		y1 = row_to_y(row);
		x2 = col_to_x(col2) + LCD_FONT_SIZE / 2 - 1;
		y2 = y1 + LCD_FONT_SIZE - 1;
		lcd_show_line(x1, y2 + 1, x2, y2 + 1, pixel);
	}
}

static void lcd_reverse_rect(int start_x, int start_y, int end_x, int end_y) {
	int i, j, addr, off, bit;

	if (lcd_valid_x_y(&start_x, &start_y, &end_x, &end_y) && LCD_BUF) {
		for (i = start_y; i <= end_y; i++) {
			for (j = start_x; j <= end_x; j++) {
				addr = i * LCD_WIDTH + j;
				off = addr / 8;
				bit = addr % 8;
				LCD_BUF[off] ^= (1 << bit);
			}
		}
		lcd_refresh(start_x, start_y, end_x - start_x + 1, end_y - start_y + 1);
	}
}

void lcd_reverse_string(int row, int col1, int col2) {
	int x1, x2, y1, y2;

	if (lcd_valid_row_col(&row, &col1, &row, &col2)) {
		x1 = col_to_x(col1);
		y1 = row_to_y(row);
		x2 = col_to_x(col2) + LCD_FONT_SIZE / 2 - 1;
		y2 = y1 + LCD_FONT_SIZE - 1;
		lcd_reverse_rect(x1, y1, x2, y2);
	}
}

int lcd_save_window(int row1, int col1, int row2, int col2, void **buf,
		int *len) {
	char *ptr, tmp[128];
	int x1, y1, x2, y2, i, j, size;

	size = LCD_WIDTH * LCD_HEIGHT / 8; // modify by Johnny on 2012.2.21
	LOG_PRINTF_ALLOCATE(
			"Before malloc() in func: %s, line: %d, allocation size: %d\n",
			__func__, __LINE__, size);
	if (!lcd_valid_row_col(&row1, &col1, &row2,
			&col2) || LCD_BUF == NULL || (ptr = malloc(size)) == NULL)
		return 0;
	LOG_PRINTF_ALLOCATE("After malloc() in func: %s, line: %d\n", __func__,
			__LINE__);
	memcpy(ptr, LCD_BUF, LCD_WIDTH * LCD_HEIGHT / 8);
	*(char **) buf = ptr;
	*len = size;
	j = col2 - col1 + 1;
	memset(tmp, ' ', j);
	for (i = row1; i <= row2; i++) {
		lcd_show_string(i, col1, j, tmp);
		if (i != row2)
			lcd_show_underline(i, col1, col2, 0);
	}
	x1 = col_to_x(col1);
	y1 = row_to_y(row1);
	x2 = col_to_x(col2) + LCD_FONT_SIZE / 2 - 1;
	y2 = row_to_y(row2) + LCD_FONT_SIZE - 1;
	if (x1 >= 1)
		x1--;
	if (x2 < LCD_WIDTH - 1)
		x2++;
	if (y1 >= 1)
		y1--;
	if (y2 < LCD_HEIGHT - 1)
		y2++;
	lcd_show_rect(x1, y1, x2, y2, 1);
	return 1;
}

int lcd_restore_window(int row1, int col1, int row2, int col2, void *buf,
		int len) {
	int size;

	size = LCD_WIDTH * LCD_HEIGHT / 8;
	if (!lcd_valid_row_col(&row1, &col1, &row2, &col2) || size != len
	|| LCD_BUF == NULL) {
		return 0;
	} else {
		memcpy(LCD_BUF, buf, LCD_WIDTH * LCD_HEIGHT / 8);
		LOG_PRINTF_ALLOCATE("free() in func: %s, line: %d\n", __func__,
				__LINE__);
		free(buf);
		lcd_refresh(0, 0, LCD_WIDTH, LCD_HEIGHT);
		return 1;
	}
}

void lcd_clear_screen(void) {
	if (LCD_BUF != NULL) {
		memset(LCD_BUF, 0, LCD_WIDTH * LCD_HEIGHT / 8);
		lcd_refresh(0, 0, LCD_WIDTH, LCD_HEIGHT);
	}
}

void lcd_show_screen(BYTE *buf) {
	BYTE *ptr;
	int i, len;

	if (LCD_BUF != NULL) {
		ptr = LCD_BUF;
		len = LCD_WIDTH / 8;
		for (i = 0; i < LCD_HEIGHT; i++) {
			memcpy(ptr, buf, len);
			ptr += len;
		}
		lcd_refresh(0, 0, LCD_WIDTH, LCD_HEIGHT);
	}
}

void lcd_clean_workspace(void) {
	int i;
	char buf[MAX_SCREEN_COL];

	memset(buf, ' ', sizeof(buf));
	for (i = 0; i < MAX_WORKSPACE_LINE; i++) {
		lcd_show_string(STATUS_BAR_LINE + i + 1, 1, MAX_SCREEN_COL, buf);
		lcd_show_underline(STATUS_BAR_LINE + i + 1, 1, MAX_SCREEN_COL, 0);
	}
}

void lcd_show_lines(void) {
	int y;

	y = row_to_y(1);
	lcd_show_line(0, y + LCD_FONT_SIZE, LCD_WIDTH - 1, y + LCD_FONT_SIZE, 1);
	y = row_to_y(MAX_SCREEN_ROW);
	lcd_show_line(0, y - 1, LCD_WIDTH - 1, y - 1, 1);
	PRINTF("function here");
}

int fep_is_connect(void);

void lcd_update_head_info(void) {
	static int month = -1, day = -1, hour = -1, min = -1, sec = -1;
	struct tm tm;
	char buf[128];
	int x, y, w;
	BYTE value, level, type;

	y = row_to_y(1);
	if (remote_module_get_netinfo(&type, &value, &level)) {
		lcd_show_icon(1, 1, level - 1);
		lcd_show_icon(1, 3, (type == 0) ? 4 : 5);
	} else {
		lcd_hide_icon(1, 1, 4);
		lcd_hide_icon(1, 3, 4);
		lcd_hide_icon(1, 3, 5);
	}
	if (fep_is_connect())
		lcd_show_icon(1, 5, 6);
	else
		lcd_hide_icon(1, 5, 6);
	if (fconalm_changed())
		lcd_show_icon(1, 7, 7);
	else
		lcd_hide_icon(1, 7, 7);
	if (fparam_get_program_status())
		lcd_show_icon(1, 7, 11);
	else
		lcd_hide_icon(1, 7, 11);
	sys_time(&tm);
	if (tm.tm_mon != month || tm.tm_mday != day || tm.tm_hour != hour
			|| tm.tm_min != min || tm.tm_sec != sec) {
		if (MAX_SCREEN_COL - 9 + 1 >= 8) {
			x = col_to_x(MAX_SCREEN_COL - 8 + 1);
			w = snprintf(buf, sizeof(buf), "%02d:%02d:%02d", tm.tm_hour,
					tm.tm_min, tm.tm_sec);
			w = text_out(x, y, buf, w, lcd_show_pixel);
		} else {
			x = col_to_x(MAX_SCREEN_COL - 5 + 1);
			w = snprintf(buf, sizeof(buf), "%02d:%02d", tm.tm_hour, tm.tm_min);
			w = text_out(x, y, buf, w, lcd_show_pixel);
		}
		month = tm.tm_mon;
		day = tm.tm_mday;
		hour = tm.tm_hour;
		min = tm.tm_min;
	}
	lcd_refresh(0, y, LCD_WIDTH, LCD_FONT_SIZE);
}

void lcd_update_info(const char *buf) {
	char str[128];
	int x, y, w, len;

	if (buf != NULL) {
		len = snprintf(str, sizeof(str), "%-*s", MAX_SCREEN_COL - 2, buf);
		x = col_to_x(1);
		y = row_to_y(MAX_SCREEN_ROW);
		w = text_out(x, y, str, len, lcd_show_pixel);
		lcd_refresh(x, y, w, LCD_FONT_SIZE);
	}
}

void lcd_show_arrow(int up, int down, int left, int right) {
	int x, y;
	const char *c_str;

	x = col_to_x(MAX_SCREEN_COL - 1);
	y = row_to_y(MAX_SCREEN_ROW);
	if (up && down) {
		if (!left && !right)
			lcd_show_icon(MAX_SCREEN_ROW, MAX_SCREEN_COL - 1, 8);
		else
			lcd_show_icon(MAX_SCREEN_ROW, MAX_SCREEN_COL - 1, 10);
	} else if (left && right) {
		if (!up && !down)
			lcd_show_icon(MAX_SCREEN_ROW, MAX_SCREEN_COL - 1, 9);
		else
			lcd_show_icon(MAX_SCREEN_ROW, MAX_SCREEN_COL - 1, 10);
	} else {
		if (up)
			c_str = c_arrow_str[0];
		else if (down)
			c_str = c_arrow_str[1];
		else if (left)
			c_str = c_arrow_str[2];
		else if (right)
			c_str = c_arrow_str[3];
		else
			c_str = "  ";
		text_out(x, y, c_str, 2, lcd_show_pixel);
	}
	lcd_refresh(x, y, LCD_FONT_SIZE, LCD_FONT_SIZE);
}

// int KeyRead(struct key_msg_t *msg);
//int KeyRead_noneblock_version(struct key_msg_t *msg, int msec)
int lcd_key_in(int flag) {
	static int lightwait_time = 60;
	static long idle_time = 0;
#ifdef AM335X
	int ch;
#elif defined IMX28
	unsigned char ch;
#endif

	struct key_msg_t msg;

	if (idle_time == 0)
		idle_time = uptime();

	//if (LCD_FD < 0 || read(LCD_FD, &ch, 1) <= 0)
	if(KeyRead_NONE_BLOCKING(&msg) == 0){
		ch = 0;
	}else{
#ifdef AM335X
		ch = (int)msg.code;
#elif defined IMX28
		ch = (unsigned char)msg.code;
#endif
	}

	if (ch) {
		idle_time = uptime();
		if (flag)
			device_lcd_light(1);
	} else if (uptime() - idle_time >= lightwait_time) {
		if (flag)
			device_lcd_light(0);
	}
	return ch;
}

void lcd_write_init_cmd(void) {
	(*LCD_INIT_CMD)();
}

static int key_hold_time[KEY_NUM];
static BYTE hold_key_value[KEY_NUM] = {
		KEY_LEFT,
		KEY_UP,
		KEY_RIGHT,
		KEY_DOWN,
		KEY_ESC,
		KEY_ENTER,
		KEY_NONE };

static int hold_key_time(BYTE key) {
	static struct timeval last_tv[KEY_NUM];
	struct timeval cur_tv;
	int i, ret = 0;

	gettimeofday(&cur_tv, NULL);
	for (i = 0; i < KEY_NUM; i++) {
		if (key == hold_key_value[i]) {
			if ((get_diff_ms(&cur_tv, &last_tv[i])) > 300) {
				key_hold_time[i] = 0;
			}
			if ((get_diff_ms(&cur_tv, &last_tv[i])) >= 200) {
				gettimeofday(&last_tv[i], NULL);
				key_hold_time[i] += 200;
			}
			ret = (key_hold_time[i] + 600) / 1000;
		} else {
			if (key != KEY_NONE) {
				key_hold_time[i] = 0;
			}
		}
	}
	return ret;
}

#ifdef IMX28

BYTE key_getch(int flag) {
	unsigned char ch;
	int hold_time;
	static int long_press_valid = 1;

	ch = lcd_key_in(flag);
	switch (ch) {
	//case 0x12:
	case 158: // default 105
		return KEY_LEFT;
	//case 0x13:
	case 108: // default 103
		return KEY_UP;
	//case 0x22:
	case 106: /// default 106
		return KEY_RIGHT;
	//case 0x23:
	case 103: //default 108
		return KEY_DOWN;
	//case 0x32:
	case 105: // default 158
		return KEY_ESC;
	//case 0x33:
	case 159: // default 159
		hold_time = hold_key_time(KEY_ENTER);
		if (long_press_valid && hold_time >= FPARAM_PROGRAM_KEY_HOLD_TIME) {
			fparam_change_program_status();
			lcd_update_head_info();
			long_press_valid = 0;
			key_hold_time[5] = 0;
		}
		return KEY_ENTER;
	default:
		hold_time = hold_key_time(KEY_NONE);
		if (hold_time >= 1) {
			long_press_valid = 1;
		}
		return KEY_NONE;
	}
}

#elif defined(AM335X)

BYTE key_getch(int flag) {
#ifdef AM335X
	int ch;
#elif defined IMX28
	unsigned char ch;
#endif
	int hold_time;
	static int long_press_valid = 1;

	ch = lcd_key_in(flag);
	switch (ch) {
	//case 0x12:
	case 352: // default 105
		return KEY_LEFT;
	//case 0x13:
	case 108: // default 103
		return KEY_UP;
	//case 0x22:
	case 105: /// default 106
		return KEY_RIGHT;
	//case 0x23:
	case 106: //default 108
		return KEY_DOWN;
	//case 0x32:
	case 103: // default 158
		return KEY_ESC;
	//case 0x33:
	case 223: // default 159
		hold_time = hold_key_time(KEY_ENTER);
		if (long_press_valid && hold_time >= FPARAM_PROGRAM_KEY_HOLD_TIME) {
			fparam_change_program_status();
			lcd_update_head_info();
			long_press_valid = 0;
			key_hold_time[5] = 0;
		}
		return KEY_ENTER;
	default:
		hold_time = hold_key_time(KEY_NONE);
		if (hold_time >= 1) {
			long_press_valid = 1;
		}
		return KEY_NONE;
	}
}

#endif


#include <linux/input.h>
#include "lcd.h"
int key_fd = -1;
int KeyOpen(void) {
	if (key_fd == -1) {
		key_fd = open(key_device, O_RDWR);
		return key_fd;
	}
	return 0;
}

int KeyClose(void) {
	if (key_fd > 0) {
		close(key_fd);
		key_fd = -1;
	}
	return 1;
}

int KeyRead_NONE_BLOCKING(struct key_msg_t *msg){

	static struct input_event data;
	int ret;

	msg->code = 0;
	msg->type = 0;

	if(key_fd < 0)
		return 0;

	if(wait_for_ready(key_fd, 1, 0)>0){
		ret = read(key_fd, &data, sizeof(data));
		//PRINTF("key code: %d,key value: %d, key type:%d \n", data.code, data.value, data.type);
		if (data.type == EV_KEY)
		{
			msg->code = data.code;
			memset(&data, 0x0, sizeof(data));
			ret = read(key_fd, &data, sizeof(data));
			//PRINTF("key code: %d,key value: %d, key type:%d \n", data.code, data.value, data.type);
			ret = read(key_fd, &data, sizeof(data));
			//PRINTF("key code: %d,key value: %d, key type:%d \n", data.code, data.value, data.type);
			if (data.value == 1)
			{
				msg->type = (enum key_type_t) data.type;
				//PRINTF("msg code: %d, key msg:%d \n", msg->code, msg->type); // commented by wd 20170608
				return 1;
			}else if(data.value == 0){

			}
		}
	}
	return 0;
}

void key_exit(void) {
	KeyClose();
}

void key_initiate(void) {
	int ret;
	ret = KeyOpen();
	if (ret != 0)
		PRINTF("WARNNING: key initiated error\n");
}

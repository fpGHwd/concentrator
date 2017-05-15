#ifndef __LCD_HW_H__
#define __LCD_HW_H__



enum LCD_LINE_STYLE {
    LINE_HORIZONTAL = 0,
    LINE_VERTICAL,
    LINE_MAX_NUM,
};

// size of CN character
enum LCD_GB2312_FONT_SIZE {
    GB_FONT_12X12 = 0,
    GB_FONT_16X16,
    GB_FONT_24X24,
    GB_FONT_32X32,
    GB_FONT_MAX,
};

// size of ENGLISH character
enum LCD_ASCII_FONT_SIZE {
    ASCII_FONT_8X8 = 0,
    ASCII_FONT_12X12,
    ASCII_FONT_16X16,
    ASCII_FONT_20X20,
    ASCII_FONT_24X24,
    ASCII_FONT_32X32,
    ASCII_FONT_MAX,
};

//lcd color
enum LCD_COLOR {
    LCD_WHITE = 0,
    LCD_BLACK,
    LCD_COLOR_MAX,
};

struct Gui_FontSize {
    int width;
    int height;
};


struct Gui_Point {
    int x; //column
    int y; //row
};

struct GUI_Draw {
    int (*set_current_color)(enum LCD_COLOR color);
    int (*draw_dot)(struct Gui_Point *point,unsigned char dot_size);
    int (*draw_line)(struct Gui_Point *point,unsigned char length,unsigned char line_width,enum LCD_LINE_STYLE style);
    int (*draw_rect)(struct Gui_Point *point,unsigned char width,unsigned char height,unsigned char line_width);
    int (*draw_text)(struct Gui_Point *point,unsigned char *text,enum LCD_GB2312_FONT_SIZE font);
    int (*draw_string)(struct Gui_Point *point,unsigned char *str,enum LCD_ASCII_FONT_SIZE font);
    int (*draw_picture)(struct Gui_Point *point,int width,int height,unsigned char *picture);
    int (*set_background)(enum LCD_COLOR color);
    int (*clear_erea)(struct Gui_Point *point,int width,int height);
    int (*fix_erea)(struct Gui_Point *point,int width,int height,enum LCD_COLOR color);
};

struct GUI_Draw *getGuiDrawContext(void);

//extern void lcd_test(void);
void lcd_api_test(void);
int lcd_set_current_color(enum LCD_COLOR color);
int draw_dot(int x, int y,unsigned char dot_size);
int draw_line(int x, int y,unsigned char length,unsigned char line_width,enum LCD_LINE_STYLE style);
int draw_rect(int x, int y,unsigned char width,unsigned char height,unsigned char line_width);
int draw_text(int x, int y,unsigned char *text,enum LCD_GB2312_FONT_SIZE font);
int draw_string(int x, int y,unsigned char *str,enum LCD_ASCII_FONT_SIZE font);
int draw_picture(int x, int y,int width,int height,unsigned char *picture);
int clear_screen_erea(int x, int y,int width,int height);
int fix_screen_erea(int x, int y, int width,int height,enum LCD_COLOR color);
int lcd_set_background(enum LCD_COLOR color);
int draw_textbox(int x, int y, int width, int height, enum LCD_COLOR color, unsigned char *text ,enum LCD_GB2312_FONT_SIZE font);
int lcd_draw_ch(int x, int y, char ch, enum LCD_ASCII_FONT_SIZE font,enum LCD_COLOR color);
int lcd_draw_TextStr(int x, int y,char *str,enum LCD_GB2312_FONT_SIZE font);


int lcd_draw_string(struct Gui_Point *point,unsigned char *str,enum LCD_ASCII_FONT_SIZE font);





int lcd_revese_string_lcd_hw(int x, int y, unsigned char *str, enum LCD_ASCII_FONT_SIZE font);


// defined by wd 20160927
void lcd_init_test_for_ready(void);

#endif

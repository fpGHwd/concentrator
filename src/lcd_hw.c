#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "lcd_hw.h"
#include "picture.h"
#include <pthread.h>

#include "lcd.h"

#define CONFIG_MCU_LCD               1

#define MODULE_DEV_PATH              "/dev/mcu_lcd"

//GB2312�����ֿ�
#define GB2312_FONT_12X12_PATH   "/usr/lib/GuiFont/GB2312_12X12.FONT"
#define GB2312_FONT_16X16_PATH   "/usr/lib/GuiFont/GB2312_16X16.FONT"
#define GB2312_FONT_24X24_PATH   "/usr/lib/GuiFont/GB2312_24X24.FONT"
#define GB2312_FONT_32X32_PATH   "/usr/lib/GuiFont/GB2312_32X32.FONT"

//GB2312Ӣ���ֿ�
#define ASCII_FONT_8X8_PATH     "/usr/lib/GuiFont/ASCII8X8.FONT"
#define ASCII_FONT_12X12_PATH   "/usr/lib/GuiFont/ASCII12X12.FONT"
#define ASCII_FONT_16X16_PATH   "/usr/lib/GuiFont/ASCII16X16.FONT"
#define ASCII_FONT_20X20_PATH   "/usr/lib/GuiFont/ASCII20X20.FONT"
#define ASCII_FONT_24X24_PATH   "/usr/lib/GuiFont/ASCII24X24.FONT"
#define ASCII_FONT_32X32_PATH   "/usr/lib/GuiFont/ASCII32X32.FONT"

//lcd �ߴ�,��0��ʼ
#define LCD_XRES              160
#define LCD_YRES              160

int TextFontSize[GB_FONT_MAX][2] = {{12,12},{16,16},{24,24},{32,32}};
int AsciiFontSize[ASCII_FONT_MAX][2] = {{4,8},{6,12},{8,16},{10,20},{12,24},{16,32}};
pthread_mutex_t p_t;

/*
//��ͼ����
enum LCD_DRAW_CMD {
    DRAW_GB2312 = 0x01,
    DRAW_ASCII ,
    DRAW_DOT  ,
    DRAW_LINE,
    DRAW_RECT,
    DRAW_COLOR,
    DRAW_CLEAR,
    DRAW_PICTURE,
    DRAW_FONT_COLOR,
    DRAW_SET_CUR_COLOR,
	DRAW_PICTURE_1,
    DRAW_CMD_NULL,
};
*/


enum LCD_DRAW_CMD {
	DRAW_GB2312 = 0x01,
	DRAW_ASCII ,
	DRAW_DOT  ,
	DRAW_LINE,
	DRAW_RECT,
	DRAW_COLOR,
	DRAW_CLEAR,
	DRAW_PICTURE,
	DRAW_FONT_COLOR,
	DRAW_SET_CUR_COLOR,
	DRAW_SELECT_BAR,
	
	DRAW_PICTURE_1,
	
	DRAW_CMD_NULL,
};


static struct GUI_Draw gui_lcd_draw;

static struct Gui_FontSize lcd_get_text_fontsize(enum LCD_GB2312_FONT_SIZE font)
{
    struct Gui_FontSize FontSize = {0};

    FontSize.width = TextFontSize[font][0];
    FontSize.height = TextFontSize[font][1];

    return FontSize;
}

static struct Gui_FontSize lcd_get_ascii_fontsize(enum LCD_ASCII_FONT_SIZE font)
{
    struct Gui_FontSize FontSize = {0};

    FontSize.width = AsciiFontSize[font][0];
    FontSize.height = AsciiFontSize[font][1];

    return FontSize;
}

int lcd_set_current_color(enum LCD_COLOR color)
{
    pthread_mutex_lock(&p_t);
    int fd;
    unsigned char buf[2];

    fd = open(MODULE_DEV_PATH,O_RDWR);
    if (fd < 0){
        printf("open lcd failed\n");
        pthread_mutex_unlock(&p_t);
        return 0;
    }
    buf[0] = DRAW_SET_CUR_COLOR;
    buf[1] = (unsigned char)color;
    write(fd,buf,sizeof(buf));
    close(fd);
    pthread_mutex_unlock(&p_t);
    return 1;
}


/*
static int lcd_set_font_color(enum LCD_FONT_COLOR color)
{
    int fd;
    unsigned char buf[2];

    fd = open(MODULE_DEV_PATH,O_RDWR);
    if (fd < 0){
        printf("open lcd failed\n");
        return 0;
    }
    buf[0] = DRAW_FONT_COLOR;
    buf[1] = (unsigned char)color;
    write(fd,buf,sizeof(buf));
    close(fd);
    return 1;
}
*/
static int lcd_draw_dot(struct Gui_Point *point,unsigned char dot_size)
{
    int fd;
    unsigned char buf[4];

    if (!point)
        return 0;
    if ((point->x >= LCD_XRES) || (point->x < 0) || (point->y >= LCD_YRES) || (point->y < 0))
    {
        return 0;
    }

    if ((dot_size <= 0) || (dot_size >= LCD_XRES)){
        dot_size = 1;
    }

    fd = open(MODULE_DEV_PATH,O_RDWR);
    if (fd < 0){
        printf("open lcd failed\n");
        return 0;
    }

    buf[0] = DRAW_DOT;
    buf[1] = point->x;
    buf[2] = point->y;
    buf[3] = dot_size;
    write(fd,buf,sizeof(buf));
    close(fd);
    return 1;
}

static int lcd_draw_line(struct Gui_Point *point,unsigned char length,unsigned char line_width,enum LCD_LINE_STYLE style)
{
    int fd;
    unsigned char buf[6];

    if (!point)
        return 0;
    if ((point->x >= LCD_XRES) || (point->x < 0) || (point->y >= LCD_YRES) || (point->y < 0))
    {
        return 0;
    }

    fd = open(MODULE_DEV_PATH,O_RDWR);
    if (fd < 0){
        printf("open lcd failed\n");
        return 0;
    }
    buf[0] = DRAW_LINE;
    buf[1] = point->x;
    buf[2] = point->y;
    buf[3] = (length > LCD_XRES) ? LCD_XRES  : length;
    buf[4] = (line_width > LCD_XRES) ? LCD_XRES : line_width;
    buf[5] = style;
    write(fd,buf,sizeof(buf));
    close(fd);
    return 1;
}

static int lcd_draw_rect(struct Gui_Point *point,unsigned char width,unsigned char height,unsigned char line_width)
{
    int fd;
    unsigned char buf[6];

    if (!point)
        return 0;
    if ((point->x >= LCD_XRES) || (point->x < 0) || (point->y >= LCD_YRES) || (point->y < 0))
    {
        return 0;
    }

    fd = open(MODULE_DEV_PATH,O_RDWR);
    if (fd < 0){
        printf("open lcd failed\n");
        return 0;
    }
    buf[0] = DRAW_RECT;
    buf[1] = point->x;
    buf[2] = point->y;
    buf[3] = (width >= LCD_XRES) ? (LCD_XRES ) : width;
    buf[4] = (height >= LCD_YRES) ? (LCD_YRES ) : height;
    buf[5] = (line_width >= LCD_YRES) ? (LCD_YRES ) : line_width;
    write(fd,buf,sizeof(buf));
    close(fd);
    return 1;
}

//д����,Ҫ��GB2312�����뷨������,������Ӣ��,���Զ�����
static int lcd_draw_text(struct Gui_Point *point,unsigned char *text,enum LCD_GB2312_FONT_SIZE font)
{
    int fd;
    unsigned int size;
    unsigned char buf[256] = {0};
    FILE *file = NULL;
    int count;
    int step = 0;
    unsigned long offset_addr = 0;

    if (!point)
        return 0;
    if ((point->x >= LCD_XRES) || (point->x < 0) || (point->y >= LCD_YRES) || (point->y < 0))
    {
        return 0;
    }

    switch(font)
    {
        case GB_FONT_12X12:
        file = fopen(GB2312_FONT_12X12_PATH,"rb");
        if (file == NULL){
            printf("open GB2312_FONT_12X12_PATH failed\n");
            return 0;
        }
        step = 12;
        count = 24;
        break;

        case GB_FONT_16X16:
        file = fopen(GB2312_FONT_16X16_PATH,"rb");
        if (file == NULL){
            printf("open GB2312_FONT_16X16_PATH failed\n");
            return 0;
        }
        step = 16;
        count = 32;
        break;

        case GB_FONT_24X24:
        file = fopen(GB2312_FONT_24X24_PATH,"rb");
        if (file == NULL){
            printf("open GB2312_FONT_24X24_PATH failed\n");
            return 0;
        }
        step = 24;
        count = 72;
        break;

        case GB_FONT_32X32:
        file = fopen(GB2312_FONT_32X32_PATH,"rb");
        if (file == NULL){
            printf("open GB2312_FONT_32X32_PATH failed\n");
            return 0;
        }
        step = 32;
        count = 128;
        break;

        default:
        break;
    }

    fd = open(MODULE_DEV_PATH,O_RDWR);
    if (fd < 0){
        printf("open lcd failed\n");
        return 0;
    }
    for (size = 0; size < strlen(text);)
    {
        buf[0] = DRAW_GB2312;
        buf[1] = font;
        buf[2] = point->x + step * (size / 2);
        buf[3] = point->y;
        offset_addr = (((unsigned long)(text[size] - 0xA1)) * 94 + (unsigned long)(text[size + 1] - 0xA1)) * count;
        fseek(file,offset_addr,SEEK_SET);
        fread(&buf[4],1,count,file);
        write(fd,buf,count + 4);
        size += 2;
    }
    if (file != NULL){
        fclose(file);
    }
    close(fd);
    return 1;
}



#define WD 1

//дӢ��,Ҫ��GB2312�����뷨������,��֪������,����������
//static int lcd_draw_string(struct Gui_Point *point,unsigned char *str,enum LCD_ASCII_FONT_SIZE font)
int lcd_draw_string(struct Gui_Point *point,unsigned char *str,enum LCD_ASCII_FONT_SIZE font)
{
    int fd;
    unsigned int size;
    unsigned char buf[256] = {0};
    FILE *file = NULL;
    int count;
    int step = 0;
    unsigned long offset_addr = 0;

    if (!point)
        return 0;
    if ((point->x >= LCD_XRES) || (point->x < 0) || (point->y >= LCD_YRES) || (point->y < 0))
    {
        return 0;
    }

    switch(font)
    {
        case ASCII_FONT_8X8:
        file = fopen(ASCII_FONT_8X8_PATH,"rb");
        if (file == NULL){
            printf("open ASCII_FONT_8X8_PATH failed\n");
            return 0;
        }
        step = 4;
        count = 4;
        break;

        case ASCII_FONT_12X12:
        file = fopen(ASCII_FONT_12X12_PATH,"rb");
        if (file == NULL){
            printf("open ASCII_FONT_12X12_PATH failed\n");
            return 0;
        }
        step = 6;
        count = 12;
        break;

        case ASCII_FONT_16X16:
        file = fopen(ASCII_FONT_16X16_PATH,"rb");
        if (file == NULL){
            printf("open ASCII_FONT_16X16_PATH failed\n");
            return 0;
        }
        step = 8;
        count = 16;
        break;

        case ASCII_FONT_20X20:
        file = fopen(ASCII_FONT_20X20_PATH,"rb");
        if (file == NULL){
            printf("open ASCII_FONT_20X20_PATH failed\n");
            return 0;
        }
        step = 10;
        count = 30;
        break;

        case ASCII_FONT_24X24:
        file = fopen(ASCII_FONT_24X24_PATH,"rb");
        if (file == NULL){
            printf("open ASCII_FONT_24X24_PATH failed\n");
            return 0;
        }
        step = 12;
        count = 36;
        break;

        case ASCII_FONT_32X32:
        file = fopen(ASCII_FONT_32X32_PATH,"rb");
        if (file == NULL){
            printf("open ASCII_FONT_32X32_PATH failed\n");
            return 0;
        }
        step = 16;
        count = 64;
        break;
        default:
        break;
    }

    fd = open(MODULE_DEV_PATH,O_RDWR);
    if (fd < 0){
        printf("open lcd failed\n");
        return 0;
    }
	
    for (size = 0; size < strlen(str);)
    {
        buf[0] = DRAW_ASCII;
        buf[1] = font;
        buf[2] = point->x + step * size;
        buf[3] = point->y;
        offset_addr = str[size] * count; //
        fseek(file,offset_addr,SEEK_SET); //
        fread(&buf[4],1,count,file); //
        write(fd,buf,count + 4);
        size += 1;
    }
	
    if (file != NULL){
        fclose(file);
    }
    close(fd);
    return 1;
}

int lcd_draw_ch(int x, int y, char ch, enum LCD_ASCII_FONT_SIZE font,enum LCD_COLOR color)
{
    int fd;
    unsigned int size;
    unsigned char buf[256] = {0};
    FILE *file = NULL;
    int count;
    int step = 0;
    unsigned long offset_addr = 0;

    if(color == LCD_BLACK)
    {
        color = LCD_WHITE;
    }
    else
    {
        color = LCD_BLACK;
    }
    lcd_set_current_color(color);

    if ((x >= LCD_XRES) || (x < 0) || (y >= LCD_YRES) || (y < 0))
    {
        return 0;
    }

    switch(font)
    {
        case ASCII_FONT_8X8:
        file = fopen(ASCII_FONT_8X8_PATH,"rb");
        if (file == NULL){
            printf("open ASCII_FONT_8X8_PATH failed\n");
            return 0;
        }
        step = 4;
        count = 4;
        break;

        case ASCII_FONT_12X12:
        file = fopen(ASCII_FONT_12X12_PATH,"rb");
        if (file == NULL){
            printf("open ASCII_FONT_12X12_PATH failed\n");
            return 0;
        }
        step = 6;
        count = 12;
        break;

        case ASCII_FONT_16X16:
        file = fopen(ASCII_FONT_16X16_PATH,"rb");
        if (file == NULL){
            printf("open ASCII_FONT_16X16_PATH failed\n");
            return 0;
        }
        step = 8;
        count = 16;
        break;

        case ASCII_FONT_20X20:
        file = fopen(ASCII_FONT_20X20_PATH,"rb");
        if (file == NULL){
            printf("open ASCII_FONT_20X20_PATH failed\n");
            return 0;
        }
        step = 10;
        count = 30;
        break;

        case ASCII_FONT_24X24:
        file = fopen(ASCII_FONT_24X24_PATH,"rb");
        if (file == NULL){
            printf("open ASCII_FONT_24X24_PATH failed\n");
            return 0;
        }
        step = 12;
        count = 36;
        break;

        case ASCII_FONT_32X32:
        file = fopen(ASCII_FONT_32X32_PATH,"rb");
        if (file == NULL){
            printf("open ASCII_FONT_32X32_PATH failed\n");
            return 0;
        }
        step = 16;
        count = 64;
        break;
        default:
        break;
    }

    fd = open(MODULE_DEV_PATH,O_RDWR);
    if (fd < 0){
        printf("open lcd failed\n");
        return 0;
    }

    buf[0] = DRAW_ASCII;
    buf[1] = font;
    buf[2] = x;
    buf[3] = y;
    offset_addr = ch * count;
    fseek(file,offset_addr,SEEK_SET);
    fread(&buf[4],1,count,file);
    write(fd,buf,count + 4);

    if (file != NULL){
        fclose(file);
    }
    close(fd);
    return 1;
}

int lcd_set_background(enum LCD_COLOR color)
{
    int fd;
    unsigned char buf[2];

    fd = open(MODULE_DEV_PATH,O_RDWR);
    printf("set background color!\n");
    if (fd < 0){
        printf("open lcd failed\n");
        return 0;
    }

    buf[0] = DRAW_COLOR;
    buf[1] = color;
    write(fd,buf,sizeof(buf));
    close(fd);
    return 1;
}

static int lcd_draw_picture(struct Gui_Point *point,int width,int height,unsigned char *picture)
{
    int fd;
    unsigned char *buf = NULL;
    unsigned int picture_size = 0;
    int n_eight = 0;

    if ((point->x >= LCD_XRES) || (point->x < 0) || (point->y >= LCD_YRES) || (point->y < 0))
    {
        return 0;
    }

    if ((width <= 0) || (height <= 0) || (width > LCD_XRES) || (height > LCD_YRES))
    {
        return 0;
    }
    fd = open(MODULE_DEV_PATH,O_RDWR);
    if (fd < 0){
        printf("open lcd failed\n");
        return 0;
    }

    if (height % 8 == 0)
    {
        n_eight = height / 8;
    }
    else {
        n_eight = height / 8 + 1;
    }


    picture_size = width * n_eight;
    buf = malloc(picture_size + 5);
    if (buf == NULL)
    {
        return 0;
    }
    buf[0] = DRAW_PICTURE;
    buf[1] = point->x;
    buf[2] = point->y;
    buf[3] = width ;
    buf[4] = height;
    memcpy(&buf[5],picture,picture_size);
    write(fd,buf,picture_size + 5);
    close(fd);
    free(buf);
    buf = NULL;
    return 1;
}


int get_screen_fd(void);
static int __LCD_DrawPicture_1(struct Gui_Point *xy, int uwWidth, int uwHeight, unsigned char *ucData )
{
    
    int k = 0;
    
    int fd = -1;
    
    unsigned int uwSize = 0;
    
    unsigned char *ucBuffer = NULL;
    //-------------------------------------------------------------------------------------------------
    if ( (xy->x < 0) || ((xy->x + uwWidth) > LCD_XRES) || 
             (xy->y < 0) || ((xy->y + uwHeight) > LCD_YRES) )
    {
        return ( 0 );   
    }
    
    if ( (xy->x & 0x07) || (uwWidth & 0x07) ) {
        
        return (0);
    }
    
    //fd = open( MODULE_DEV_PATH, O_RDWR );
    fd = get_screen_fd();

    if ( fd < 0 ){
        
    //printf("\r\n__LCD_DrawPicture : LCD Open Fail\r\n");
    
        return 0;
    }
    
    uwSize = ( ( uwWidth / 8 ) * uwHeight );
    
    ucBuffer = malloc(uwSize + 5);
    
    if ( ucBuffer == NULL ) {
        if (fd > 0)
            ///close(fd); ///wd
        return (0);
    }
    
    k = 0;
    ucBuffer[k++] = DRAW_PICTURE_1;
    ucBuffer[k++] = xy->x;
    ucBuffer[k++] = xy->y;
    ucBuffer[k++] = uwWidth;
    ucBuffer[k++] = uwHeight;
    
    memcpy( &ucBuffer[k], ucData, uwSize ); k += uwSize;
    
    //lcd_lock_interface(); /// wd
    write( fd, ucBuffer, k );
    //lcd_unlock_interface(); /// wd

    //close(fd); /// wd
    
    free(ucBuffer);
    
    ucBuffer = NULL;
    //----------------------------------------------------------------------------------------
    return 1;
}

/*
static int __LCD_DrawPicture_1( struct Gui_Point *xy, int uwWidth, int uwHeight, unsigned char *ucData )
{
	
	int k = 0;
	
	int fd = -1;
	
	unsigned int uwSize = 0;
	
	unsigned char *ucBuffer = NULL;
	//-------------------------------------------------------------------------------------------------

	//printf("xy->x = %d, xy->y = %d, uwwWidth = %d, uwHeight=%d\n", xy->x, xy->y, uwWidth, uwHeight);
	if ( (xy->x < 0) || ((xy->x + uwWidth) >= LCD_XRES) || 
			 (xy->y < 0) || ((xy->y + uwHeight) >= LCD_YRES) )
	{

		printf("xy->x = %d, xy->y = %d, uwwWidth = %d, uwHeight=%d\n", xy->x, xy->y, uwWidth, uwHeight);
		return ( 0 ); 	
	}
	
	//printf("xy->x = %d, xy->y = %d, uwwWidth = %d, uwHeight=%d\n", xy->x, xy->y, uwWidth, uwHeight);
	if ( (xy->x & 0x07) || (uwWidth & 0x07) ) {
		
		return (0);
	}
	
//	printf("xy->x = %d, xy->y = %d, uwwWidth = %d, uwHeight=%d\n", xy->x, xy->y, uwWidth, uwHeight);
	printf("open MODULE_DEV_PATH\n");
	fd = open( MODULE_DEV_PATH, O_RDWR );
	
	if ( fd < 0 ){
		
  	printf("\r\n__LCD_DrawPicture : LCD Open Fail\r\n");
  	
 		return 0;
	}
	
	uwSize = ( ( uwWidth / 8 ) * uwHeight );
	
	ucBuffer = malloc(uwSize + 5);
	
	if ( ucBuffer == NULL ) {
		
		return (0);
	}
	
	k = 0;


	ucBuffer[k++] = DRAW_PICTURE_1;
	ucBuffer[k++] = xy->x;
	ucBuffer[k++] = xy->y;
	ucBuffer[k++] = uwWidth ;
	ucBuffer[k++] = uwHeight;
	
	memcpy( &ucBuffer[k], ucData, uwSize ); k += uwSize;
	
	printf("xy->x = %d, xy->y = %d, uwwWidth = %d, uwHeight=%d\n", xy->x, xy->y, uwWidth, uwHeight);
	write( fd, ucBuffer, k );

	close(fd);
	
	free(ucBuffer);
	
	ucBuffer = NULL;
	//----------------------------------------------------------------------------------------
	return 1;
}
*/

static int lcd_clear_screen_erea(struct Gui_Point *point,int width,int height)
{
    int fd;
    unsigned char buf[5];
    if ((point->x >= LCD_XRES) || (point->x < 0) || (point->y >= LCD_YRES) || (point->y < 0))
    {
        return 0;
    }

    if ((width < 0) || (height < 0) || (width > LCD_XRES) || (height > LCD_YRES))
    {
        return 0;
    }

    fd = open(MODULE_DEV_PATH,O_RDWR);
    if (fd < 0){
        printf("open lcd failed\n");
        return 0;
    }

    buf[0] = DRAW_CLEAR;
    buf[1] = point->x;
    buf[2] = point->y;
    buf[3] = width;
    buf[4] = height;
    write(fd,buf,sizeof(buf));
    close(fd);
    return 1;
}


static int lcd_fix_screen_erea(struct Gui_Point *point,int width,int height,enum LCD_COLOR color)
{
    lcd_set_current_color(color);
    lcd_clear_screen_erea(point,width,height);
    return 1;
}

int fix_screen_erea(int x, int y, int width,int height,enum LCD_COLOR color)
{
    struct Gui_Point point;
    point.x = x;
    point.y = y;
    return lcd_fix_screen_erea(&point, width, height, color);
}

struct GUI_Draw *getGuiDrawContext(void)
{
    #if CONFIG_MCU_LCD == 1
    gui_lcd_draw.set_current_color = lcd_set_current_color;
    gui_lcd_draw.draw_dot       = lcd_draw_dot;
    gui_lcd_draw.draw_line      = lcd_draw_line;
    gui_lcd_draw.draw_rect      = lcd_draw_rect;
    gui_lcd_draw.draw_text      = lcd_draw_text;
    gui_lcd_draw.draw_string    = lcd_draw_string;
    gui_lcd_draw.draw_picture   = lcd_draw_picture;
    gui_lcd_draw.set_background = lcd_set_background;
    gui_lcd_draw.clear_erea     = lcd_clear_screen_erea;
    gui_lcd_draw.fix_erea       = lcd_fix_screen_erea;
    return &gui_lcd_draw;
    #else
    return NULL;
    #endif
}

int lcd_draw_TextStr(int x, int y,char *str,enum LCD_GB2312_FONT_SIZE font)
{
    pthread_mutex_lock(&p_t);
    unsigned int i = 0;
    struct Gui_Point p;
    char tmpfont;
    struct Gui_FontSize FontSize = {0};
    char name[3] = "";
    p.x = x;
    p.y = y;

    switch(font)
    {
        case GB_FONT_12X12:
        tmpfont = ASCII_FONT_12X12;
        break;
        case GB_FONT_16X16:
        tmpfont = ASCII_FONT_16X16;
        break;
        case GB_FONT_24X24:
        tmpfont = ASCII_FONT_24X24;
        break;
        case GB_FONT_32X32:
        tmpfont = ASCII_FONT_32X32;
        break;
        default:
        break;
    }

    while(str[i] != '\0')
    {
        if (str[i] < 0x80) //ascii
        {
            name[0] = str[i];
            name[1] = '\0';
            name[2] = '\0';
            lcd_draw_string(&p,name,tmpfont);
            i++;
            FontSize = lcd_get_ascii_fontsize(tmpfont);
            p.x += FontSize.width;

        }
        else {
            //����
            name[0] = str[i];
            name[1] = str[i+1];
            name[2] = '\0';
            lcd_draw_text(&p,name,font);
            i += 2;
            FontSize = lcd_get_text_fontsize(font);
            p.x += FontSize.width;
        }
    }
    pthread_mutex_unlock(&p_t);
    return 0;
}

int font_size(enum LCD_GB2312_FONT_SIZE font)
{
    switch(font)
    {
    case GB_FONT_12X12:
        return 12;
        break;
    case GB_FONT_16X16:
        return 16;
        break;
    case GB_FONT_24X24:
        return 24;
        break;
    case GB_FONT_32X32:
        return 32;
        break;
    }
    return 161;
}


int draw_textbox(int x, int y, int width, int height, enum LCD_COLOR color, unsigned char *text ,enum LCD_GB2312_FONT_SIZE font)
{
    int f_size = font_size(font);
    static const int margin_left = 0;

    if(height < f_size)
    	return -1;

    int margin = (height - f_size) / 2;
    int f_draw_width = f_size * (strlen(text) / 2);
   // int f_draw_width = f_size / 2 * (strlen(text));
   // lcd_draw_TextStr(x, y, text, font);
    fix_screen_erea(x + f_draw_width + margin_left , y, width - f_draw_width - margin_left, height, color);
    fix_screen_erea(x , y,margin_left, height, color);
    fix_screen_erea(x + margin_left  , y, f_draw_width, margin, color);
    fix_screen_erea(x + margin_left  , y + height - margin, f_draw_width, margin, color);

    //draw_text(x + margin_left , y + margin , text , font);
    lcd_draw_TextStr(x + margin_left, y + margin, text, font);
    return 0;

}

int draw_dot(int x, int y,unsigned char dot_size)
{
    struct Gui_Point point;
    point.x = x;
    point.y = y;
    return lcd_draw_dot(&point,dot_size);
}

int draw_line(int x, int y,unsigned char length,unsigned char line_width,enum LCD_LINE_STYLE style)
{
    struct Gui_Point point;
    point.x = x;
    point.y = y;
    return lcd_draw_line(&point, length, line_width, style);
}

int draw_rect(int x, int y,unsigned char width,unsigned char height,unsigned char line_width)
{
    struct Gui_Point point;
    point.x = x;
    point.y = y;
    return lcd_draw_rect(&point, width, height, line_width);
}

int draw_text(int x, int y,unsigned char *text,enum LCD_GB2312_FONT_SIZE font)
{
    struct Gui_Point point; /// point
    point.x = x; //// setting the point
    point.y = y; ////
    return lcd_draw_text(&point, text, font); ////
}

int draw_string(int x, int y,unsigned char *str,enum LCD_ASCII_FONT_SIZE font)
{
    struct Gui_Point point;
    point.x = x;
    point.y = y;
    return lcd_draw_string(&point, str, font);
}

int LCD_DrawPicture_1( int x, int y, int uwWidth, int uwHeight, unsigned char *ucData )
{
	
	struct Gui_Point tXY;
	//--------------------------------------------------------------------------------------------
	tXY.x = x;
	
	tXY.y = y;
	
	return ( __LCD_DrawPicture_1( &tXY, uwWidth, uwHeight, ucData ) );
}

int draw_picture(int x, int y,int width,int height,unsigned char *picture)
{
    struct Gui_Point point;
    point.x = x;
    point.y = y;
    return lcd_draw_picture(&point, width, height, picture);
}

int clear_screen_erea(int x, int y,int width,int height)
{
    struct Gui_Point point;
    point.x = x;
    point.y = y;
    return lcd_clear_screen_erea(&point, width, height);
}


void lcd_api_test(void)
{
    struct GUI_Draw *gui;
    struct Gui_Point point;

    gui = getGuiDrawContext();
    if (!gui)
        return;

    point.x = 0;
    point.y = 0;
    gui->clear_erea(&point,160,160);
    gui->set_background(LCD_WHITE);
    point.x = 0;
    point.y = 0;
    gui->set_current_color(LCD_BLACK);
    gui->draw_text(&point,"����ϵͳ",GB_FONT_32X32);
    point.x = 0;
    point.y = 32;
    gui->set_current_color(LCD_WHITE);
    gui->draw_text(&point,"������ϵͳ",GB_FONT_24X24);
    point.x = 0;
    point.y = 56;
    gui->draw_string(&point,"lcd show system 123!",ASCII_FONT_16X16);
    point.x = 0;
    point.y = 72;
    gui->draw_picture(&point,100,80,CE_SHI_PIC);
    gui->draw_rect(&point,60,60,2);
    gui->fix_erea(&point,60,60,LCD_BLACK);
}


#if 0
void lcd_test(void)
{
    struct Gui_Point point;
    point.x = 0;
    point.y = 0;

    lcd_clear_screen_erea(&point,160,160);
    //return;
    lcd_set_background(LCD_BLACK);
    point.x = 0;
    point.y = 0;
    lcd_draw_text(&point,"����",GB_FONT_32X32);
    point.x = 10;
    point.y = 90;
    lcd_draw_string(&point,"456STW,?.",ASCII_FONT_20X20);
    point.x = 10;
    point.y = 118;
    lcd_draw_string(&point,"0*/ABC,?.",ASCII_FONT_32X32);
    point.x = 0;
    point.y = 0;
    lcd_clear_screen_erea(&point,100,40);
    //return;
    lcd_draw_picture(&point,100,80,CE_SHI_PIC);

    point.x = 10;
    point.y = 10;
    lcd_draw_dot(&point,2);

    point.x = 10;
    point.y = 13;
    lcd_draw_text(&point,"�������й�������",GB_FONT_12X12);
    point.x = 10;
    point.y = 26;
    lcd_draw_text(&point,"�������й�������",GB_FONT_16X16);
    point.x = 10;
    point.y = 43;
    lcd_draw_text(&point,"�������й���",GB_FONT_24X24);
    point.x = 10;
    point.y = 68;
    lcd_draw_string(&point,"123abc,i<",ASCII_FONT_8X8);
    point.x = 10;
    point.y = 77;
    lcd_draw_string(&point,"789xyz,?.",ASCII_FONT_12X12);
    point.x = 10;
    point.y = 90;
    lcd_draw_string(&point,"456STW,?.",ASCII_FONT_16X16);
    point.x = 10;
    point.y = 108;
    lcd_draw_string(&point,"0*/ABC,?.",ASCII_FONT_24X24);
    point.x = 10;
    point.y = 133;
    lcd_draw_line(&point,100,2,LINE_HORIZONTAL);
    lcd_draw_line(&point,10,2,LINE_VERTICAL);
    point.x = 15;
    point.y = 143;
    lcd_draw_rect(&point,100,15,2);
    lcd_set_background(LCD_BLACK);

    point.x = 10;
    point.y = 120;
    lcd_draw_text(&point,"�������й���",GB_FONT_32X32);
    point.x = 0;
    point.y = 0;
    lcd_clear_screen_erea(&point,160,160);
}
#endif




// retained; reserved; for other use sometime
void lcd_init_test_for_ready(void){
	//


}



int lcd_revese_string_lcd_hw(int x, int y, unsigned char *str, enum LCD_ASCII_FONT_SIZE font)
{
	// clear this domain
	// Gui_Point 
	struct Gui_Point *point;
	point->x = x;
	point->y = y;


	int width = 0;
	int height = 0;

	int flag = 0; // string:0, character:1
	if(flag == 0 && font == ASCII_FONT_12X12){
		width = 6 * strlen(str);
		height = 12;
	}else if(flag == 0 && font == ASCII_FONT_16X16){
		width = 8 * strlen(str);
		height = 16;
	}
	// fix
	if(lcd_fix_screen_erea(point,width,height,LCD_BLACK));
	// draw string
	if( font == ASCII_FONT_12X12){
		draw_string( x, y, str, ASCII_FONT_12X12);
	}else if( font == ASCII_FONT_16X16){
		draw_string( x, y, str, ASCII_FONT_16X16);
	}

}

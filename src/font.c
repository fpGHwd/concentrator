/*
 * font.c
 *
 *  Created on: 2015-8-15
 *      Author: Johnnyzhang
 */

#include "font.h"
#include "common.h"

struct font_info {
	unsigned int width; /* font width */
	void *name; /* font file name */
	void *asc, *hzk; /* start pointer of pixels */
	void *buf; /* font memory */
	unsigned int len; /* font memory size */
};

static struct font_info font_info[] = {
		{ 12, "/opt/concentrator/font/font12" },
		{ 16, "/opt/concentrator/font/font16" }, //hzk16s for test
		{ 0, NULL }
};

static struct font_info *font = NULL;

static sem_t sem_t_font;

static void font_lock(void) {
	sem_wait(&sem_t_font);
}

static void font_unlock(void) {
	sem_post(&sem_t_font);
}

int font_init(int size) {
	int fd;
	struct font_info *ptr;

	sem_init(&sem_t_font, 0, 1);
	ptr = font_info, font = NULL;
	while (ptr->width != 0 && ptr->width != size)
		ptr++;
	if (ptr->width != 0 && (fd = open(ptr->name, O_RDONLY)) >= 0) {
		ptr->len = lseek(fd, 0, SEEK_END);
		lseek(fd, 0, SEEK_SET);
		if ((ptr->buf = malloc(ptr->len)) != NULL) {
			memset(ptr->buf, 0, sizeof(ptr->len));
			safe_read(fd, ptr->buf, ptr->len);
			ptr->asc = ptr->buf;
			ptr->hzk = ptr->buf + 95 * ptr->width; /* 32..126 */
			font = ptr;
		}
		close(fd);
	}
	return font != NULL;
}

void font_destroy(void) {
	if (font != NULL) {
		free(font->buf);
		font = NULL;
	}
	sem_destroy(&sem_t_font);
}

int text_out(int x, int y, const void *buf, int len,
		void (*pixel_out)(int x, int y, int pixel)) {
	int i, j, off, ret = 0;
	unsigned short word;
	const unsigned char *addr;
	const unsigned char *ptr = buf;

	font_lock();
	while (font != NULL && len > 0) {
		if (ptr[0] >= 0xa1 && ptr[0] <= 0xfe && len >= 2) {
			off = (ptr[0] - 0xa1) * 94 + (ptr[1] - 0xa1);
			addr = font->hzk + off * font->width * 2;
			for (i = 0; i < font->width; i++) {
				word = (addr[0] << 8) + addr[1];
				for (j = 0; j < font->width; j++) {
					(*pixel_out)(x + j, y + i, (word & 0x8000) ? 1 : 0);
					word <<= 1;
				}
				addr += 2;
			}
			x += font->width;
			ptr += 2, len -= 2;
			ret += font->width;
		} else {
			if (ptr[0] >= 32 && ptr[0] <= 126)
				off = ptr[0] - 32;
			else
				off = 0;
			addr = font->asc + off * font->width;
			for (i = 0; i < font->width; i++) {
				word = addr[0] << 8;
				for (j = 0; j < font->width / 2; j++) {
					(*pixel_out)(x + j, y + i, (word & 0x8000) ? 1 : 0);
					word <<= 1;
				}
				addr++;
			}
			x += font->width / 2;
			ptr++, len--;
			ret += (font->width / 2);
		}
	}
	font_unlock();
	return ret;
}

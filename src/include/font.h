/*
 * font.h
 *
 *  Created on: 2015��8��15��
 *      Author: Johnnyzhang
 */

#ifndef FONT_H_
#define FONT_H_

#include "typedef.h"

int font_init(int size);

void font_destroy(void);

int text_out(int x, int y, const void *buf, int len,
		void (*pixel_out)(int x, int y, int pixel));

#endif /* FONT_H_ */

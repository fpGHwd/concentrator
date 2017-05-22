/*
 * gpio.h
 *
 *  Created on: 2015-8-11
 *      Author: Johnnyzhang
 */

#ifndef GPIO_H_
#define GPIO_H_

#include "typedef.h"

int gpio_open(void);
void gpio_close(void);

void device_lcd_reset(void);
void device_lcd_light(int on);

void modem_hard_reset(void);
void modem_soft_reset(void);
void modem_gprs_shutdown(void);
void modem_gprs_turn_on(void);

enum led_index {
	LED1 = 1, /// RUNNING LIGHT /// blue
	LED2, /// RS485-2
	LED3, /// RS485-1
	LED4, /// WARNING /// green
};

enum led_status {
	FADE = 0, BRIGHT,
};

void control_led(enum led_index, enum led_status);
void led_show(void);
void led_fade(void);


enum lora_light{
	BLUE_RECEIVE,
	GREEN_SEND,
};
void lora_lights(enum lora_light idx, int light);

#endif /* GPIO_H_ */

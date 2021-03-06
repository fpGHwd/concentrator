/*
 * devices.c
 *
 *  Created on: 2015-8-15
 *      Author: Johnnyzhang
 */

#include "devices.h"
#include "gpio.h"

#ifdef AM335X
const char *rdmeter_device = "/dev/ttyO2";
const char *modem_device = "/dev/ttyO4";
const char *modem_lockname = "/var/lock/LCK..ttyO4";
const char *modem_lockpid_name = "/var/run/ppp-ttyO4.pid";
const char *gpio_device = "/dev/gpio";
const char *lcd_device = "/dev/mcu_lcd";
const char *watch_dog_device = "/dev/watchdog";
const char *modem_reset_device_path = "/sys/class/gprs_cls/power_ctrl";
const char *modem_reset_device_path1 = "/sys/class/gprs_cls/power_ctrl1";
const char *lora_led_rx = "/sys/class/gprs_cls/lora_int"; // gprs1_17
const char *lora_led_tx = "/sys/class/gprs_cls/lora_state0"; // gprs1_14
const char *clock_device = "/dev/rtc0";
const char *key_device = "/dev/input/event0";

#elif defined(IMX28)

const char *rdmeter_device = "/dev/ttySP1";
const char *modem_device = "/dev/ttySP4";
const char *modem_lockname = "/var/lock/LCK..ttySP4";
const char *modem_lockpid_name = "/var/run/ppp-ttySP4.pid";
const char *gpio_device = "/dev/gpio";
const char *lcd_device = "/dev/char_cdev";
const char *watch_dog_device = "/dev/watchdog";
const char *clock_device = "/dev/rtc0";
const char *key_device = "/dev/input/event0";
const char *modem_reset_device_path = "/sys/class/gprs_cls/power_ctrl";

#endif

/*
 * devices.c
 *
 *  Created on: 2015-8-15
 *      Author: Johnnyzhang
 */

#include "devices.h"
#include "gpio.h"

//const char *rdmeter_device = "/dev/ttyPSHS0";
//const char *rdmeter_device = "/dev/Lora";
//const char *rdmeter_device = "/dev/ttyPSHS3";
//const char *rdmeter_device = "/dev/ttyO2";
const char *rdmeter_device = "/dev/ttySP0";
//const char *modem_device = "/dev/ttyPSHS2";
//const char *modem_device = "/dev/ttyO4";  /// gprs modern
const char *modem_device = "/dev/ttySP2";
const char *modem_lockname = "/var/lock/LCK..ttyPSHS2";
const char *modem_lockpid_name = "/var/run/ppp-ttyPSHS2.pid";
const char *gpio_device = "/dev/gpio";
//const char *lcd_device = "/dev/fb";
//const char *lcd_device = "/dev/mcu_lcd";
const char *lcd_device = "/dev/char_cdev";
const char *watch_dog_device = "/dev/watchdog";
//#define WATCHDOG_DEV "/dev/watchdog"
const char *modem_reset_device_path = "/sys/class/gprs_cls/power_ctrl";
//const char *clock_device = "/dev/rtc";
//const char *key_device = "/dev/input/event0";
const char *key_device = "/dev/input/event1";

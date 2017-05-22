/*
 * devices.c
 *
 *  Created on: 2015-8-15
 *      Author: Johnnyzhang
 */

#include "devices.h"
#include "gpio.h"

#ifdef AM335X
//const char *rdmeter_device = "/dev/ttyPSHS0";
//const char *rdmeter_device = "/dev/Lora";
//const char *rdmeter_device = "/dev/ttyPSHS3";
const char *rdmeter_device = "/dev/ttyO2";
//const char *rdmeter_device = "/dev/ttySP0";
//const char *modem_device = "/dev/ttyPSHS2";
const char *modem_device = "/dev/ttyO4";
//const char *modem_device = "/dev/ttySP2";
const char *modem_lockname = "/var/lock/LCK..ttyO4";
const char *modem_lockpid_name = "/var/run/ppp-ttyO4.pid";
const char *gpio_device = "/dev/gpio";
//const char *lcd_device = "/dev/fb";
const char *lcd_device = "/dev/mcu_lcd";
//const char *lcd_device = "/dev/char_cdev";
const char *watch_dog_device = "/dev/watchdog";
//#define WATCHDOG_DEV "/dev/watchdog"
const char *modem_reset_device_path = "/sys/class/gprs_cls/power_ctrl"; // gprs0_19
const char *modem_reset_device_path1 = "/sys/class/gprs_cls/power_ctrl1"; // gprs3_19
const char *lora_led_rx = "/sys/class/gprs_cls/lora_int"; // gprs1_17 // blue receive
const char *lora_led_tx = "/sys/class/gprs_cls/lora_state0"; // gprs1_14 //  green send
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
// 3_7(imx28) -> 3_19 3_11(imx28) -> 0_19
// echo 55 > export  3_19 -> gprs_pctrl(concentrator board)
// echo 59 > export	 0_19 -> gprs_onoff(concentrator board)

#endif

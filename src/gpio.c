/*
 * gpio.c
 *
 *  Created on: 2015-8-11
 *      Author: Johnnyzhang
 */

#include "gpio.h"
#include "common.h"
#include "devices.h"

#define GPIOCTL_INP_STATE		_IOR('G', 0x01, unsigned int)  
#define GPIOCTL_OUTP_STATE		_IOR('G', 0x02, unsigned int)
#define GPIOCTL_DIR_STATE		_IOR('G', 0x03, unsigned int)
#define GPIOCTL_SDINP_STATE		_IOR('G', 0x04, unsigned int)
#define GPIOCTL_MUX_STATE		_IOR('G', 0x05, unsigned int) /* READ 1~5 type comand */

#define GPIOCTL_OUTP_SET		_IOW('G', 0x01, unsigned int)
#define GPIOCTL_OUTP_CLR		_IOW('G', 0x02, unsigned int)
#define GPIOCTL_DIR_SET			_IOW('G', 0x03, unsigned int)
#define GPIOCTL_DIR_CLR			_IOW('G', 0x04, unsigned int)
#define GPIOCTL_SDOUTP_SET		_IOW('G', 0x05, unsigned int)
#define GPIOCTL_SDOUTP_CLR		_IOW('G', 0x06, unsigned int)
#define GPIOCTL_MUX_SET			_IOW('G', 0x07, unsigned int)
#define GPIOCTL_MUX_CLR			_IOW('G', 0x08, unsigned int) /* write 1~8 command*/

//#define MISC_GPIO "/dev/misc/gpio"


#define BIT_WATCHDOG			(1 << 12)
#define BIT_EB_RELAY			(1 << 9)
#define BIT_STATUS_LED_RED		(1 << 15)
#define BIT_STATUS_LED_GREEN	(1 << 10)
#define BIT_LCD_LIGHT			(1 << 6) 
#define BIT_LCD_XRESET			(1 << 7)

#define BIT_MODEM_RESET			(1 << 4)
#define BIT_MODEM_IGT			(1 << 13)
#define BIT_MODEM_RELAY			(1 << 18)

#define BIT_MODEM_DTR			(1 << 3)
#define BIT_MODEM_RTS			(1 << 22)


#define MODEM_ONOFF
#define MODOM_RESET
#define MODEN_POWER

#define WATCH_DOG
#define LCD_DEVICE "/dev/leds"


static sem_t sem_gpio;
static int gpio_fd = -1;

int gpio_open(void) {
	sem_init(&sem_gpio, 0, 1);
	gpio_fd = open(gpio_device, O_RDWR);
	if (gpio_fd < 0)
		fprintf(stderr, "gpio open error");
	else
		fprintf(stdout, "gpio open OK");
	return gpio_fd;
}

void gpio_close(void) {
	if (gpio_fd >= 0) {
		close(gpio_fd);
	}
	sem_destroy(&sem_gpio);
}

static inline void gpio_lock(void) {
	sem_wait(&sem_gpio);
}

static inline void gpio_unlock(void) {
	sem_post(&sem_gpio);
}

static void gpio_ioctl(int cmd, unsigned int *val) {
	if (gpio_fd >= 0) {
		gpio_lock();
		ioctl(gpio_fd, cmd, val); /// system func control io-file-descriptor, to set io status /// refering
		gpio_unlock();
	} else {
		//printf("gpio_fd < 0\n");
	}
}

/*
 static void clear_modem_flow_ctrl(void)
 {
 unsigned int val;

 val = BIT_MODEM_DTR;
 gpio_ioctl(GPIOCTL_OUTP_CLR, &val);
 val = BIT_MODEM_RTS;
 gpio_ioctl(GPIOCTL_OUTP_CLR, &val);
 }
 */

void device_lcd_reset(void)
{
	// turn off light
}

void device_lcd_light(int on)
{
	// turn on light
}

#define MODEM_POWER_ON	0x33 // 0x33 '3'
#define MODEM_POWER_OFF 0x32 // 0x32 '2'
#define MODEM_GPRS_ON   0x30 // 0x30 '0'
#define MODEM_GPRS_OFF  0X31 // 0X31 '1'

// TODO: rewrite the modem power reset
void modem_hard_reset(void)
{
#if 1
	int fd;
	char buf[2];
	int ret;

	if ((fd = open(modem_reset_device_path, O_WRONLY)) < 0) {
		PRINTF("Open %s failed\n", modem_reset_device_path);
		return;
	}
	PRINTF("%s: 15 seconds for shutdown modem power and re-enpower\n",
			__FUNCTION__);
	buf[0] = MODEM_POWER_OFF;
	ret = write(fd, buf, 2);
	wait_delay(5000);
	buf[0] = MODEM_POWER_ON;
	ret = write(fd, buf, 2);
	wait_delay(2000);
	close(fd);
	return;
#else
	unsigned int val;

	PRINTF("Modem hardware reset\n");
	val = BIT_MODEM_IGT;
	gpio_ioctl(GPIOCTL_OUTP_SET, &val);

	val = BIT_MODEM_RELAY;
	gpio_ioctl(GPIOCTL_OUTP_CLR, &val); /// gpio_ioctl
	clear_modem_flow_ctrl();
	wait_delay(10000);
	gpio_ioctl(GPIOCTL_OUTP_SET, &val);
	wait_delay(1000);
	modem_soft_reset();
#endif

}

void modem_soft_reset(void) {
	int ret;

#if 1
	int fd;
	char buf[2];

	if ((fd = open(modem_reset_device_path, O_WRONLY)) < 0) {
		PRINTF("Open %s failed\n", modem_reset_device_path);
		return;
	}
	///PRINTF("Open %s successfully, fd: %d\n", modem_reset_device_path,fd);
	PRINTF(
			"%s: 7 seconds for disable gprs-function and re-enable gprs-function\n",
			__FUNCTION__);
	buf[0] = MODEM_GPRS_OFF;
	ret = write(fd, buf, 2);
	wait_delay(5000);
	buf[0] = MODEM_GPRS_ON;
	ret = write(fd, buf, 2);
	close(fd);
	///PRINTF("Close %s successfully, fd: %d\n", modem_reset_device_path, fd);
	wait_delay(2000);
	return;

#else 
	unsigned int val;

	PRINTF("Modem init or software reset\n");
	clear_modem_flow_ctrl();
	val = BIT_MODEM_RELAY;
	gpio_ioctl(GPIOCTL_OUTP_SET, &val); /// GPIOCTL_DIR_SET file descriptor
	wait_delay(50);

	val = BIT_MODEM_IGT;
	gpio_ioctl(GPIOCTL_OUTP_CLR, &val);

	val = BIT_MODEM_RESET;
	gpio_ioctl(GPIOCTL_OUTP_CLR, &val);
	wait_delay(15);
	val = BIT_MODEM_RESET;
	gpio_ioctl(GPIOCTL_OUTP_SET, &val);
	wait_delay(30);

	wait_delay(1000);
	val = BIT_MODEM_IGT;
	gpio_ioctl(GPIOCTL_OUTP_SET, &val);
	wait_delay(500);

	PRINTF("Wait 10s for software reset\n");
	wait_delay(10000);
#endif
}

void modem_gprs_shutdown(void) {

	int fd;
	char buf[2];
	int ret;

	if ((fd = open(modem_reset_device_path, O_WRONLY)) < 0) { /// if(fd = open(modem_reset_device_path, O_WRONLY) < 0){
		PRINTF("Open %s failed\n", modem_reset_device_path);
		return;
	}
	///PRINTF("%s: 15 seconds for shutdown modem power and re-enpower\n", __FUNCTION__);
	///printf("power delay for 10 seconds\n");
	buf[0] = MODEM_POWER_OFF;
	ret = write(fd, buf, 2);
	wait_delay(2000);
	close(fd);

}

void modem_gprs_turn_on(void) {
	int fd, ret;
	char buf[2];

	if ((fd = open(modem_reset_device_path, O_WRONLY)) < 0) {
		PRINTF("Open %s failed\n", modem_reset_device_path);
		return;
	}
	buf[0] = MODEM_POWER_ON;
	ret = write(fd, buf, 2);
	wait_delay(2000);
	close(fd);
}

void control_led(enum led_index i, enum led_status s) {
	char buf[100];
	int ret;

	snprintf(buf, 100, "echo %d > /sys/class/am335x_led/LED%d/brightness",
			s ? 1 : 0, i - 1);
	ret = system(buf);

	return; // NOT NECESSARY
}

void led_show(void) {
	control_led(1, 1);
	control_led(2, 1);
	control_led(3, 1);
	control_led(4, 1);
}

void led_fade(void) {
	control_led(1, 0);
	control_led(2, 0);
	control_led(3, 0);
	control_led(4, 0);
}

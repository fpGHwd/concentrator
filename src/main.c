/*
 * main.c
 *
 *  Created on: 2015-8-10
 *      Author: Johnnyzhang
 */

//#include <key_hw.h.BAK>
#include "main.h"
#include "common.h"
#include "f_tables.h"
#include "devices.h"
#include "f_param.h"
#include "threads.h"
#include "msg_que.h"
#include "f_gasmeter.h"
#include "f_current.h"
#include "f_day.h"
#include "f_month.h"
#include "f_con_alarm.h"
#include "f_gasmeter_alarm.h"
#include "lcd.h"
#include "gpio.h"
#include <stdlib.h>
//#include "gprs_flux.h"

const char *g_release_time = RELEASE_APP_TIME;

int g_terminated = 0;
int g_silent = 0;
int g_lcd_type = 1;
const int g_retry_times = 3;
const int rf_id = 0;

const struct debug debug_ctrl = {
		.gasmeter_test = false, /// -> false
		.watchdog_enable = false, /// -> true
		.repeater_enable = false, /// -> ture;
		.sqlite_enable = false, /// -> false
		.led_enable = false, /// -> false
		.gpio_enable = false,
		.gprs_display_back_enable = false,
		.key_enable = true,
};

UP_COMM_SOCKET_TYPE g_socket_type = UP_COMM_SOCKET_TYPE_TCP;

static void print_version(void) {
	printf("Concentrator program for GAS Concentrator V%s build on %s\n",
			APP_VERSION, g_release_time);
}

static void print_addr(void) {
	BYTE addr[7];

	fparam_init();
	fparam_get_value(FPARAMID_CON_ADDRESS, addr, 7);
	fparam_destroy();
	printf("Concentrator Address: %02X%02X%02X%02X%02X%02X%02X\n", addr[0], addr[1],
			addr[2], addr[3], addr[4], addr[5], addr[6]);
	}

	static void print_quit(void) {
	char prog[PATH_MAX];

	get_prog_name(prog, sizeof(prog));
	PRINTF("Concentrator program \"%s\" exit.\n", prog);
}

static void usage(const char *prog) {
	printf("Usage:\n\t%s [OPTIONS]\n", prog);
	printf("\t-v,\ttake no param, to display the software version\n");
	printf("\t-h,\ttake no param, to display help information\n");
}

static void signal_term(int sig) {
	g_terminated = 1;
}

static void signal_chld(int sig) {
	int status;

	while (waitpid(-1, &status, WNOHANG) > 0)
		;
}

static void daemon_init(void) {
	if (fork() != 0)
		exit(0);
	setsid();
	signal(SIGHUP, SIG_IGN);
	signal(SIGPIPE, SIG_IGN);
	if (fork() != 0)
		exit(0);
	close_all(3);
}

static void power_fail(int sig) {
	static int fail = 0;
	int sys_ret = 0;

	if (fail == 0) {
		fail = 1;
		if (sig) {
			g_terminated = 1; // tell all threads to exit
			flush_tables();
		}
		sync();
		LOG_PRINTF("power fail\n");
		sync(); // FILES CHANGE -> DISK

		kill_watchdog();
		sys_ret = system("/sbin/reboot");
		exit(1);
	}
}

static void handler_power_fail(void) {
	int flags;

	/*
	if (gpio_fd < 0) {
	 PRINTF("Can't open /dev/gpio\n");
	}else {
	 signal(SIGIO, power_fail);
	 fcntl(gpio_fd, F_SETOWN, getpid());
	 flags = fcntl(gpio_fd, F_GETFL);
	 fcntl(gpio_fd, F_SETFL, flags | FASYNC);
	}
	*/

	signal(SIGIO, power_fail);
}

void sqlite_initiate(void);
static void system_init(void) {
	modem_gprs_turn_on();
	if(debug_ctrl.key_enable)
		key_initiate();
	read_rtc();

	msg_que_init();

#ifdef ORIGIN_FILE_SAVE
	fparam_init();
	fgasmeter_open();
	fcurrent_open();
	fday_open();
	fmon_open();
#endif

#ifdef SQLITE_SAVE
	sqlite_initiate();
#endif

	fconalm_open();
	fgasmeteralm_open();
	//sim_card_flux_database_init();

	lcd_open(lcd_device, g_lcd_type, 0x10);
	lcd_show_lines();
	if(debug_ctrl.gpio_enable)
		gpio_open();
	else
		PRINTF("WARNNING: gpio is not enable\n");

}

static void system_exit(void) {
	if(debug_ctrl.led_enable)
		control_led(1,0);
	else
		PRINTF("WARNNING: led is not enable\n");

	if(debug_ctrl.key_enable)
		key_exit();
	gpio_close();
	lcd_close();
	fgasmeteralm_close();
	fconalm_close();
	fmon_close();
	fday_close();
	fcurrent_close();
	fgasmeter_close();
	fparam_destroy();
	msg_que_destroy();
}

int main(int argc, char **argv) {
	int opt;

	set_prog_name(argv[0]);
	while ((opt = getopt(argc, argv, "vhs")) > 0) {
		switch (opt) {
		case 'v':
			print_version();
			print_addr();
			return 0;
		case 's':
			g_silent = 1;
			break;
		case 'h':
		default:
			usage(argv[0]);
			return 1;
		}
	}
	LOG_PRINTF("Concentrator program for GAS Concentrator V%s build on %s\n", APP_VERSION, g_release_time);
	daemon_init();
	signal(SIGINT, signal_term);
	signal(SIGTERM, signal_term);
	signal(SIGCHLD, signal_chld);
	system_init();
	if(debug_ctrl.watchdog_enable)
		init_watchdog();
	else
		PRINTF("WARNNING: watchdog is not set\n");
	threads_create();
	handler_power_fail();
	threads_join();
	system_exit();
	print_quit();

	return 0;
}

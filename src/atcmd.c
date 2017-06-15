/*
 * atcmd.c
 *
 *  Created on: 2015-8-11
 *      Author: Johnnyzhang
 */

#include "atcmd.h"
#include "main.h"
#include "common.h"
#include "serial.h"
#include "threads.h"

int at_cmd_sub(int fd, const char *send, char *recv, int max_len, int timeout1,
		int timeout2, BOOL silent) {
	char temp[1024];
	int len;

	if ((len = strlen(send)) > 0) {
		if (!silent) {
			PRINTF("%s To modem: %s\n", __FUNCTION__, send);
		}
		write_serial(fd, send, len, timeout1);
	}
	notify_watchdog();
	if (wait_for_ready(fd, timeout1, 0) <= 0 || g_terminated)
		return 0;
	if ((len = read_serial(fd, recv, max_len, timeout2)) > 0) {
		recv[len] = 0x0;
		/* discard data when we have not enough memory */
		while (read_serial(fd, temp, sizeof(temp), timeout2) > 0)
			;
		if (!silent) {
			PRINTF("%s From modem: %s\n", __FUNCTION__, recv);
		}
		return len;
	}
	return 0;
}

int at_cmd(int fd, const char *send, char *recv, int max_len, int timeout1,
		int timeout2) {
	return at_cmd_sub(fd, send, recv, max_len, timeout1, timeout2, FALSE);
}

int at_cmd_send(int fd, const char *send, /*BYTE*/ int len, int timeout1, int timeout2) {

	if (len > 0) {
		PRINTF("%s %d bytes\n", __FUNCTION__, len);
		return write_serial(fd, send, len, timeout1);
	}
	return 0;
}

int at_cmd_receive(int fd, char *recv, int max_len, int timeout1, int timeout2) {
	int len;
	if (wait_for_ready(fd, timeout1, 0) <= 0)
		return 0;
	if ((len = read_serial(fd, recv, max_len, timeout2)) > 0) {
		recv[len] = 0x0;
		PRINTF("%s %d bytes\n", __FUNCTION__, len);
		return len;
	}
	return 0;
}

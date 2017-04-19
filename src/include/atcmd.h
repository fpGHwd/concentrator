/*
 * atcmd.h
 *
 *  Created on: 2015-8-11
 *      Author: Johnnyzhang
 */

#ifndef ATCMD_H_
#define ATCMD_H_

#include "typedef.h"

int at_cmd(int fd, const char *send, char *recv, int max_len, int timeout1,
		int timeout2);
int at_cmd_sub(int fd, const char *send, char *recv, int max_len, int timeout1,
		int timeout2, BOOL silent);
int at_cmd_send(int fd, const char *send, BYTE len, int timeout1, int timeout2);
int at_cmd_receive(int fd, char *recv, int max_len, int timeout1, int timeout2);

/// s = command form, m = read time out, n = write timeout, e = enum status, r = result string;
#define AT_CMD(s,m,n,e) \
	do { \
		if (!at_cmd(fd, s, resp, sizeof(resp), m, n)) { \
			return e; \
		} \
	} while (0)

#define AT_CMD_CHECK(s,m,n,e, r) \
	do { \
		if (at_cmd(fd, s, resp, sizeof(resp), m, n) <= 0) { \
			return e; \
		} \
		if ((strstr(resp, r)) == NULL) \
			return e; \
	} while (0)

#endif /* ATCMD_H_ */

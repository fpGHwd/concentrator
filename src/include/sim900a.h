/*
 * sim900a.h
 *
 *  Created on: 2015-9-4
 *      Author: Johnnyzhang
 */

#ifndef INCLUDE_SIM900A_H_
#define INCLUDE_SIM900A_H_

#include "typedef.h"
#include "gprscdma.h"

e_remote_module_status sim900a_init(int fd);
int sim900a_ppp_connect(const char *device_name, const char *lock_name,
		const char *baudstr);
int sim900a_tcp_connect(int fd, const char *addr, int port, int timeout);
int sim900a_udp_connect(int fd, const char *addr, int port);
int sim900a_send(int fd, const BYTE *buf, int len, int *errcode);
int sim900a_receive(int fd, BYTE *buf, int maxlen, int timeout, int *errcode);
int sim900a_shutdown(int fd);
BOOL sim900a_getip(int fd, char *ipstr);

#endif /* INCLUDE_SIM900A_H_ */

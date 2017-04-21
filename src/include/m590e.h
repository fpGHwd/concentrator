/*
 * m590e.h
 *
 *  Created on: Apr 20, 2017
 *      Author: nayowang
 */

#ifndef SRC_INCLUDE_M590E_H_
#define SRC_INCLUDE_M590E_H_

#include "gprscdma.h"

extern void *g_m590e_resource;
e_remote_module_status m590e_init(int fd);

int m590e_ppp_connect(const char *device_name, const char *lock_name,
		const char *baudstr);
int m590e_tcp_connect(int fd, const char *addr, int port, int timeout);
int m590e_udp_connect(int fd, const char *addr, int port);
int m590e_send(int fd, const BYTE *buf, int len, int *errcode);
int m590e_receive(int fd, BYTE *buf, int maxlen, int timeout, int *errcode);
int m590e_shutdown(int fd);
BOOL m590e_getip(int fd, char *ipstr);



#endif /* SRC_INCLUDE_M590E_H_ */

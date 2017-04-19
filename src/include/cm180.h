/*
 * cm180.h
 *
 *  Created on: 2015-8-11
 *      Author: Johnnyzhang
 */

#ifndef CM180_H_
#define CM180_H_

#include "typedef.h"
#include "gprscdma.h"
//--------------------------------------------------------------------------------------------------------
#define NEOWAY_TYPE_GPRS 1U

enum GprsAuthType {
	AUTH_NONE = 0u, AUTH_PAP = 1u, AUTH_CHAP = 2u
};
//----------------------------------------------------------------------------------------------------------
extern void *g_cm180_resource;
e_remote_module_status cm180_init(int fd);

int cm180_ppp_connect(const char *device_name, const char *lock_name,
		const char *baudstr);
int cm180_tcp_connect(int fd, const char *addr, int port, int timeout);
int cm180_udp_connect(int fd, const char *addr, int port);
int cm180_send(int fd, const BYTE *buf, int len, int *errcode);
int cm180_receive(int fd, BYTE *buf, int maxlen, int timeout, int *errcode);
int cm180_shutdown(int fd);
BOOL cm180_getip(int fd, char *ipstr);

#endif /* CM180_H_ */

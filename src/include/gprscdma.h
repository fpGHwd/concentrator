/*
 * gprscdma.h
 *
 *  Created on: 2015-8-15
 *      Author: Johnnyzhang
 */

#ifndef GPRSCDMA_H_
#define GPRSCDMA_H_

#include "typedef.h"

#define MODEM_DEFAULT_BAUD		B115200  /// B115200 macro in system...
#define MODEM_DEFAULT_BAUD_STR "115200"

#define REMOTE_MODULE_RW_NORMAL	0
#define REMOTE_MODULE_RW_UNORMAL -1
#define REMOTE_MODULE_RW_ISP_CLOSE_CONNECT -2

// unread data by TCP/IP protocol stack in OS
#define REMOTE_MODULE_UNREAD_PCLSTACKINOS 1

// unwrite data by TCP/IP in OS
#define REMOTE_MODULE_UNWRITE_PCLSTACKINOS 2 /// 

#define REMOTE_MODULE_RW_ABORT 3 /// write abort

enum {
	MODEM_NET_TYPE_GPRS = 0, MODEM_NET_TYPE_CDMA = 1, MODEM_NET_TYPE_UNKNOWN = 2,
};

typedef enum {
	e_model_unknown,
	e_cdma_model_neo_cm180,
	e_gprs_model_sim900,
	e_gprs_model_neo_m590e_r2, /// add by wd
	e_gprs_model_test_command, /// add by wd
} e_remote_module_model;

typedef enum {
	e_modem_st_normal,
	e_modem_st_deivce_abort,
	e_modem_st_atcmd_abort,
	e_modem_st_sim_card_abort,
	e_modem_st_signal_intensity_abort, // such as signal intensity is too weak
	e_modem_st_register_network_fail,
	e_modem_st_unknown_abort,
} e_remote_module_status; /// module status

int open_modem_device(const char *device_name, const char *lock_name,
		int baudrate);
void close_modem_device(const char *lock_name);

int modem_check(const char *device_name, const char *lock_name, int baudrate);

int remote_ppp_connect(const char *device_name, const char *lock_name,
		const char *baudstr);

int remote_tcpudp_connect(BYTE type, const char *addr, int port, int timeout);

int remote_tcpudp_read(int fd, void *buf, int max_len, int timeout,
		int *errcode);

int remote_tcpudp_write(int fd, const void *buf, int len, int *errcode);

int remote_module_close(const char *lock_name);

BOOL remote_module_get_ip(char *des_addr);

int remote_module_get_netinfo(BYTE *type, BYTE *value, BYTE *level);

#endif /* GPRSCDMA_H_ */

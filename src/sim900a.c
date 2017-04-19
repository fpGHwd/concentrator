/*
 * sim900a.c
 *
 *  Created on: 2015-9-4
 *      Author: Johnnyzhang
 */

#include "sim900a.h"
#include "common.h"
#include "atcmd.h"

#define SIM900A_WRITE_TIMEOUT (2 * 1000)
#define SIM900A_READ_TIMEOUT (10 * 1000)

static char sim900a_ip_str[64] = { 0 };

e_remote_module_status sim900a_init(int fd) {
	char resp[1024], *ptr;
	int t1 = SIM900A_READ_TIMEOUT, t2 = SIM900A_WRITE_TIMEOUT;
	const e_remote_module_status abort_st = e_modem_st_deivce_abort;

	if (fd < 0)
		return abort_st;
	PRINTF("%s Start\n", __FUNCTION__);
	AT_CMD_CHECK("ATE0\r", t1, t2, abort_st, "OK");  /// close back display

	/// dicuss resp
	if (at_cmd(fd, "AT+CIPMUX?\r", resp, sizeof(resp), t1, t2) > 0) { // CIPMUX:<n> and OK in resp
		if ((ptr = strstr(resp, "+CIPMUX:")) == NULL)
			return abort_st; // not CIPMUX and abort in resp
		if ((ptr = strstr(ptr, "0")) != NULL) { /// 0 in resp
			if (at_cmd(fd, "AT+CIPSTATUS\r", resp, sizeof(resp), t1, t2) > 0) { /// connection status
				if ((ptr = strstr(resp, "STATE")) == NULL) // if status NOT IN resp
					return abort_st; // abort
				if (strstr(ptr, "CONNECT OK") != NULL) { // if CONNECT OK after STATE
					return e_modem_st_normal;  /// normal
				}
			}
		} else {
			AT_CMD_CHECK("AT+CIPMUX=0\r", t1, t2, abort_st, "OK"); // set SINGLE CONNECTION and OK, and set abort_st's value
		}
	}

	AT_CMD_CHECK("AT+CSMINS=1\r", t1, t2, abort_st, "OK"); /// SIM STATUS OK?
	AT_CMD_CHECK("AT+CPIN?\r", t1, t2, abort_st, "+CPIN: READY"); /// if sim need PIN code; not need->normal status
	AT_CMD_CHECK("AT+CGATT=1\r", t1, t2, abort_st, "OK"); /// adhere GPRS SERVICE, ok -> nomal status

	if (at_cmd(fd, "AT+CIPRXGET?\r", resp, sizeof(resp), t1, t2) > 0) { // NO cmd
		if ((ptr = strstr(resp, "+CIPRXGET:")) == NULL)
			return abort_st; /// abort
		if ((ptr = strstr(ptr, "1")) == NULL) {
			AT_CMD_CHECK("AT+CIPRXGET=1\r", t1, t2, abort_st, "OK");
		}
	}

	//AT_CMD("AT+CIPQRCLOSE=1\r", t1, t2, abort_st);
	if (at_cmd(fd, "AT+CIPMODE?\r", resp, sizeof(resp), t1, t2) > 0) { // TCP/IP app mode
		if ((ptr = strstr(resp, "+CIPMODE:")) == NULL)
			return abort_st;
		if ((ptr = strstr(ptr, "0")) == NULL) { ///  check if there is 0, if no 0 ,set 0
			AT_CMD_CHECK("AT+CIPMODE=0\r", t1, t2, abort_st, "OK"); /// set 0, check if it is OK
		}
	}
	AT_CMD_CHECK("AT+CIPSHUT\r", t1, t2, abort_st, "SHUT OK"); /// shutdown mobile SECENE /// if shut ok, shut ok
	AT_CMD_CHECK("AT+CSTT=\"CMNET\",,\r", t1, t2, abort_st, "OK"); /// access by APN, id, password
	return e_modem_st_normal; /// if not ok ,return before running here; run here OK
}

int sim900a_ppp_connect(const char *device_name, const char *lock_name,
		const char *baudstr)  /// point to point protocol
// device_name, lock_name, baudstr
{
	int fd;
	char resp[1024] = { 0 }, *ptr;
	int t1 = SIM900A_READ_TIMEOUT, t2 = SIM900A_WRITE_TIMEOUT;
	int wait_cnt = 50;

	if ((fd = open_modem_device(device_name, lock_name, MODEM_DEFAULT_BAUD))
			< 0) /// open modem device to get FD
		return -1;
	while (at_cmd(fd, "AT+CIPSTATUS\r", resp, sizeof(resp), t1, t2) > 0) {
		if ((ptr = strstr(resp, "STATE:")) != NULL) {
			if (strstr(ptr, "CONNECT OK") != NULL
					|| strstr(ptr, "IP STATUS") != NULL
					|| strstr(ptr, "IP GPRSACT") != NULL) { //// connect ok || ip status //// multiple connections
				at_cmd(fd, "AT+CIFSR\r", resp, sizeof(resp), t1, t2); /// local ip address
				if (strstr(resp, "ERROR") != NULL) {
					memset(sim900a_ip_str, 0, sizeof(sim900a_ip_str));
				} else {
					if ((ptr = strstr(resp, "\n")) != NULL) { /// if \n in
						strcpy(sim900a_ip_str, ptr + 1); /// copy ptr(ip string)  in sim900a_ip_str
					} else {
						memset(sim900a_ip_str, 0, sizeof(sim900a_ip_str)); ///set zero
					}
				}
				return fd; ///
			} else if (strstr(ptr, "IP START") != NULL) { ////
				AT_CMD_CHECK("AT+CIICR\r", t1, t2, -1, "OK"); /// activate mobile scene
			}
		}
		if (wait_cnt < 0) /// wait_connect 50 times
			break;
		wait_cnt--;
		msleep(500);
	}
	return -1;
}

static int sim900a_tcpudp_connect(const char *connect_str, int fd,
		const char *addr, int port, int timeout) /// connect name, fd, addr, port timeout
{
	int ret = 0;
	char resp[1024] = { 0 }, *resp_ptr = resp, *ptr;
	char send[1024] = { 0 };
	int t1 = SIM900A_READ_TIMEOUT, t2 = SIM900A_WRITE_TIMEOUT;
	long last_uptime;

	if (connect_str == NULL || fd < 0)  ///
		return fd;
	if (at_cmd(fd, "AT+CIPSTATUS\r", resp, sizeof(resp), t1, t2) > 0) { /// cip status
		if ((ptr = strstr(resp, "STATE:")) != NULL /// state
		&& (strstr(ptr, "CONNECT OK") != NULL)) { /// connect ok
			AT_CMD_CHECK("AT+CIPCLOSE=1\r", t1, t2, -1, "CLOSE OK"); /// close connection
		}
	}
	snprintf(send, sizeof(send), "AT+CIPSTART=\"%s\",\"%s\",%d\r", connect_str,
			addr, port); /// single connection  /// tcp udp relying on connect_str
	ret = at_cmd(fd, send, resp, sizeof(resp), t1, t2); /// for send and at_cmd  to GPRS module
	if (ret <= 0)
		return -1;
	if ((resp_ptr = strstr(resp + 2, "CONNECT OK")) != NULL /// \n ommited by + 2
	|| (resp_ptr = strstr(resp + 2, "ALREADY CONNECT")) != NULL) /// 
		return fd; /// connect success
	else {
		last_uptime = uptime();
		while (at_cmd(fd, "AT+CIPSTATUS\r", resp, sizeof(resp), t1, t2) > 0) {
			if ((ptr = strstr(resp, "STATE:")) != NULL) {
				if (strstr(ptr, "CONNECT OK") != NULL)
					return fd;
				else if (strstr(ptr, "CONNECTING") != NULL) {  //// server mode?
					if (uptime() - last_uptime < (timeout / 1000))
						break;
					msleep(500);
				} else
					break;
			} else
				break;
		}
		return -1;
	}
}

int sim900a_tcp_connect(int fd, const char *addr, int port, int timeout) {
	return sim900a_tcpudp_connect("TCP", fd, addr, port, timeout);
}

int sim900a_udp_connect(int fd, const char *addr, int port) {
	return sim900a_tcpudp_connect("UDP", fd, addr, port, SIM900A_READ_TIMEOUT);
}

int sim900a_send(int fd, const BYTE *buf, int len, int *errcode) //// fd- file descriptor, buf- send pointer, len - message send length , errcode - self return errcode
{
	int ret = 0;
	char resp[1024], *resp_ptr = resp;
	char send[2048];
	BYTE *buf_ptr = (BYTE *) buf;
	int t1 = SIM900A_READ_TIMEOUT, t2 = SIM900A_WRITE_TIMEOUT;
	int data_len = 0, send_len = 0;
	int wait_cnt;
#define MAX_DEFAULT_SEND_LEN 1024

	if (fd < 0)
		return 0;
	while (len > 0) {
		memset(send, 0x0, sizeof(send));
		if (len > MAX_DEFAULT_SEND_LEN) {
			if (at_cmd(fd, "AT+CIPSEND?\r", resp, sizeof(resp), t1, t2) > 0) { /// query the size fo the send string size
				if ((resp_ptr = strstr(resp, "+CIPSEND:")) != NULL) { // if CIPSEND: IN
					resp_ptr = strstr(resp_ptr, ":");
					data_len = atoi(resp_ptr + 1);  /// string to int
					PRINTF("MAX send length is %d bytes in module\n", data_len); /// get th
				}
			}
			if (data_len == 0) {
				data_len = MAX_DEFAULT_SEND_LEN;
			}
		} else {
			data_len = len;
		}
		data_len = min(data_len, sizeof(send) - 1);
		data_len = min(data_len, len);
		if (data_len <= 0) {
			PRINTF("%s FAIL (NO DATA)\n", __FUNCTION__);
			return 0;
		}
		snprintf(send, sizeof(send), "%s=%d\r", "AT+CIPSEND", data_len);
		PRINTF("%s To modem AT+CIPSEND=%d\n", __FUNCTION__, data_len);
		ret = at_cmd_sub(fd, send, resp, sizeof(resp), t1, t2, TRUE);
		if (ret <= 0) {
			*errcode = REMOTE_MODULE_RW_ABORT;
			return 0;
		}
		if ((resp_ptr = strstr(resp, ">")) == NULL) {
			*errcode = REMOTE_MODULE_RW_ABORT;
			return 0;
		}
		memcpy(send, buf_ptr, data_len);
		send[data_len] = 0x1A;
		ret = at_cmd_send(fd, send, data_len + 1, t1, t2) - 1;
		if (at_cmd_receive(fd, resp, min(sizeof(resp), 9), t1, t2)
				== 9 && (resp_ptr = strstr(resp, "SEND OK")) != NULL) {
			*errcode = REMOTE_MODULE_RW_NORMAL;
			send_len += ret;
			len -= ret;
			buf_ptr += ret;
			PRINTF("%s Send %d HEX bytes OK\n", __FUNCTION__, ret); /// send %d
			if (len <= 0)
				break;
			wait_cnt = 10;
			while (wait_cnt-- > 0) {
				data_len = 0;
				if (at_cmd(fd, "AT+CIPSEND?\r", resp, sizeof(resp), t1, t2)
						> 0) {
					if ((resp_ptr = strstr(resp, "CLOSED")) != NULL) {
						*errcode = REMOTE_MODULE_RW_ABORT;
						return send_len;
					}
					if ((resp_ptr = strstr(resp, "+CIPSEND:")) != NULL) {
						resp_ptr = strstr(resp_ptr, ":");
						data_len = atoi(resp_ptr + 1);
						PRINTF(
								"MAX send length is %d bytes for next in module\n",
								data_len);
					}
				}
				if (data_len > 0)
					break;
				else {
					msleep(200);
				}
			}
		} else {
			*errcode = REMOTE_MODULE_RW_ABORT;
			PRINTF("%s FAIL\n", __FUNCTION__);
			return 0;
		}
	}
	return send_len;
}

int sim900a_receive(int fd, BYTE *buf, int maxlen, int timeout, int *errcode) {
	int ret, data_len = 0;
	char resp[1024] = { 0 }, *resp_ptr = resp;
	char send[1024] = { 0 };
	int t1 = SIM900A_READ_TIMEOUT, t2 = SIM900A_WRITE_TIMEOUT;

	if (fd < 0)
		return FALSE;
	snprintf(send, sizeof(send), "%s\r", "AT+CIPRXGET=2,1000"); //// AT+CIPRXGET?
	ret = at_cmd_sub(fd, send, resp, sizeof(resp), t1, t2, TRUE);
	if (ret <= 0) {
		PRINTF("%s AT CMD send FAIL for AT+CIPRXGET=2,1000\n", __FUNCTION__);
		return 0;
	}
	if ((resp_ptr = strstr(resp, "+CIPRXGET")) == NULL) {
		PRINTF("%s Not found '+CIPRXGET'\n",
		__FUNCTION__);
		*errcode = REMOTE_MODULE_RW_ABORT;
		return 0;
	}
	if ((resp_ptr = strstr(resp_ptr, ",")) == NULL) {
		PRINTF("%s Not found ','\n",
		__FUNCTION__);
		*errcode = REMOTE_MODULE_RW_ABORT;
		return 0;
	}
	sscanf(resp_ptr + 1, "%d", &data_len);
	if ((resp_ptr = strstr(resp_ptr, "\n")) == NULL) {
		PRINTF("%s Not found '\\n'",
		__FUNCTION__);
		*errcode = REMOTE_MODULE_RW_ABORT;
		return 0;
	}
	if (data_len > 0) {
		memcpy(buf, resp_ptr + 1, data_len);
		PRINTF("%s %d bytes\n", __FUNCTION__, data_len);
	}
	*errcode = REMOTE_MODULE_RW_NORMAL;
	return data_len;
}

int sim900a_shutdown(int fd) /// shutdown 
{
	int ret = 0;
	char resp[1024] = { 0 }, *resp_ptr = resp;
	char send[1024] = { 0 };
	int t1 = SIM900A_READ_TIMEOUT, t2 = SIM900A_WRITE_TIMEOUT;

	if (fd < 0)
		return FALSE;
	snprintf(send, sizeof(send), "AT+CIPCLOSE=1\r"); /// close TCP or UDP
	ret = at_cmd(fd, send, resp, sizeof(resp), t1, t2);
	if (ret <= 0)
		return FALSE;
	if ((resp_ptr = strstr(resp, "CLOSE OK")) == NULL) /// close ok, sigle connection
		return FALSE;
	return TRUE;
}

BOOL sim900a_getip(int fd, char *ipstr) /// get ip
{
	if (strlen(sim900a_ip_str) > 0) {
		if (ipstr) {
			PRINTF(sim900a_ip_str);
			strcpy(ipstr, sim900a_ip_str);
		}
		return TRUE;
	} else
		return FALSE;
}

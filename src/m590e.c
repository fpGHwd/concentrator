/*
 * m590e.c
 *
 *  Created on: Apr 20, 2017
 *      Author: nayowang
 */
#include "m590e.h"
#include "atcmd.h"
#include "common.h"
#include "main.h"

void *g_m590e_resource = NULL;

//#define M590E_WRITE_TIMEOUT (2 * 1000u)
//#define M590E_READ_TIMEOUT (10 * 1000u)
#define M590E_WRITE_TIMEOUT (1 * 1000u)
#define M590E_READ_TIMEOUT (2 * 1000u)
#define M590E_CONNECTO_TIMEOUT (10 * 1000u)

#define M590E_SOCKET_ID 0
static char m590e_ip_str[64] = {0};

typedef enum{
	SOCKETID_1,
	SOCKETID_2,
	SOCKETID_3,
	SOCKETID_4,
	SOCKETID_5,
}GPRS_SOCKETID_T;

e_remote_module_status m590e_init(int fd)
{
	char resp[1024]; //*ptr;
	int t1 = M590E_READ_TIMEOUT, t2 = M590E_WRITE_TIMEOUT;
	const e_remote_module_status abort_st = e_modem_st_deivce_abort;

	if(fd<0)
		return abort_st;
	else
		PRINTF("%s Start\n", __FUNCTION__);

	AT_CMD_CHECK("AT+CFUN=1,1\r", t1, 5000u, abort_st, "OK"); // restart module

	//if(debug_ctrl.gprs_display_back_enable)
		//AT_CMD_CHECK("ATE1\r", t1, t2, abort_st, "OK");
	//else
		AT_CMD_CHECK("ATE0\r", t1, t2, abort_st, "OK");// should close display back

	//AT_CMD_CHECK("AT$MYTYPE?\r", t1, t2, abort_st, "OK");
	AT_CMD_CHECK("AT$MYGMR\r", t1, t2, abort_st, "OK");
	AT_CMD_CHECK("AT+CPIN?\r", t1, t2, abort_st, "+CPIN: READY");
	AT_CMD_CHECK("AT$MYCCID\r", t1, t2, abort_st, "$MYCCID: ");
	AT_CMD_CHECK("AT+CSQ\r", t1, t2, abort_st, "+CSQ: "); // check
	AT_CMD_CHECK("AT+CREG?\r",t1, t2, abort_st, "+CREG: "); // check
	AT_CMD_CHECK("AT$MYNETCON=0,APN,CMNET\r", t1, t2, abort_st, "OK");
	AT_CMD_CHECK("AT$MYNETCON=0,AUTH,0\r", t1, t2, abort_st, "OK");
	AT_CMD_CHECK("AT$MYNETCON=0,USERPWD,CMNET,CMNET\r", t1, t2, abort_st, "OK");

	PRINTF("M590E initiated OK\n");

	return e_modem_st_normal;
}

int m590e_ppp_connect(const char *device_name, const char *lock_name, const char *baudstr)
{
	int fd;
	char resp[1024] = {0}, *ptr, *ptr1;
	int t1 = M590E_READ_TIMEOUT, t2 = M590E_WRITE_TIMEOUT;
	int wait_cnt = 5;

	if ((fd = open_modem_device(device_name, lock_name, MODEM_DEFAULT_BAUD)) < 0)
		return -1;
	while (at_cmd(fd, "AT$MYNETURC=1\r", resp, sizeof(resp), t1, t2) > 0){
		if((ptr = strstr(resp, "OK"))!= NULL){
			at_cmd(fd, "AT$MYNETACT=0,1\r", resp, sizeof(resp), t1, t2);
			if((ptr = strstr(resp, ",\"")) != NULL && (ptr1 = strstr(resp, "\"\r")) != NULL){
				memcpy(m590e_ip_str, ptr + 2, ptr1 - ptr - 2);
				return fd;
			}else{
				if(ptr == NULL){
					PRINTF("PTR = NULL \n");
				}else{
					PRINTF("PTR1 = null\n");
				}
			}
		}
		if (wait_cnt < 0)
			break;
		wait_cnt--;
		msleep(1000);
	}
	return -1;
}

static int m590e_tcpudp_connect(const char *connect_str, int fd,
		const char *addr, int port, int timeout)
{
	char resp[1024] = { 0 }, *resp_ptr = resp;
	char send[1024] = { 0 };
	int t1 = M590E_READ_TIMEOUT, t2 = M590E_WRITE_TIMEOUT;


	if(connect_str == NULL || fd < 0)
		return -1;

	if(strcmp(connect_str, "TCP") == 0){
		snprintf(send, sizeof(send), "AT$MYNETSRV=%d,%d,%d,%d,\"%s:%d\"\r", 0,0,0,0, addr, port);
		if(at_cmd(fd, send,resp, sizeof(resp), t1, t2)>0){
			if(strstr(resp, "OK") == NULL){
				PRINTF("%s Not found 'OK' for AT$MYNETSRV=%d,%d,%d,%d,\"%s:%d\"\n", __FUNCTION__, 0,0,0,0, addr, port);
				return -1;
			}
			snprintf(send, sizeof(send), "AT$MYNETOPEN=%d\r", M590E_SOCKET_ID);
			if((at_cmd(fd, send, resp, sizeof(resp), t1, M590E_CONNECTO_TIMEOUT))> 0){
				if(strstr(resp, "OK") != NULL)
					return fd;
				else{
					PRINTF("%s Not found 'OK' for 'AT$MYNETOPEN=%d'\n", __FUNCTION__);
					return -1;
				}
			}else{
				PRINTF("%s Not receive for 'AT$MYNETOPEN=%d'\n", __FUNCTION__,M590E_SOCKET_ID);
				return -1;
			}
		}
	}else if(strcmp(connect_str,"UDP") == 0){
		PRINTF("choose udp connect and NO UDP implementation\n");
		return -1;
	}
	PRINTF("%s: unknown addressed reason for failure to connect\n", __FUNCTION__);
	return -1;
}


int m590e_tcp_connect(int fd, const char *addr, int port, int timeout)
{
	return m590e_tcpudp_connect("TCP", fd, addr, port, timeout);
}

int m590e_udp_connect(int fd, const char *addr, int port)
{
	return m590e_tcpudp_connect("UDP", fd, addr, port, M590E_READ_TIMEOUT);
}

int m590e_send(int fd, const BYTE *buf, int len, int *errcode)
{
	int ret = 0;
	char resp[1024],*resp_ptr = resp;
	char send[2048];
	BYTE *buf_ptr = (BYTE *)buf;
	int t1 = M590E_READ_TIMEOUT, t2 = M590E_WRITE_TIMEOUT;
	int data_len = 0, send_len = 0;

#define MAX_DEFAULT_SEND_LEN 1024

	if (fd < 0)
		return 0;

	while (len > 0) {
		memset(send,0x0,sizeof(send));
		if (len > MAX_DEFAULT_SEND_LEN) { /// to send len > MAX len limited
			snprintf(send, sizeof(send), "AT$MYNETOPEN=%d\r", M590E_SOCKET_ID);
			if (at_cmd(fd, send, resp, sizeof(resp), t1, t2) > 0) { // query the socket size of m590e
				if ((resp_ptr = strstr(resp, "$MYNETOPEN:")) != NULL) {// if CIPSEND: IN
					resp_ptr = strstr(resp_ptr, ":");
					data_len = atoi(resp_ptr + 4); // get max data length send //$MYNETOPEN: 0,2000
					PRINTF("MAX send length is %d bytes in module\n", data_len);
				}
			}
			if (data_len == 0) {
				data_len = MAX_DEFAULT_SEND_LEN;
			}
		}
		else {
			data_len = len;
		}
		data_len = min(data_len, sizeof(send) - 1);
		data_len = min(data_len, len);
		if (data_len <= 0) {
			PRINTF("%s FAIL (NO DATA)\n", __FUNCTION__);
			return 0;
		}
		snprintf(send,sizeof(send),"%s=%d,%d\r","AT$MYNETWRITE",M590E_SOCKET_ID,data_len);
		PRINTF("%s To modem AT$MYNETWRITE=%d,%d\n", __FUNCTION__,M590E_SOCKET_ID,data_len);
		ret = at_cmd_sub(fd, send, resp, sizeof(resp), t1, t2, TRUE);
		if (ret <= 0) {
			*errcode = REMOTE_MODULE_RW_ABORT;
			return 0;
		}
		if ((resp_ptr = strstr(resp, "$MYNETWRITE:")) == NULL) {// TIP TO SEND
			*errcode = REMOTE_MODULE_RW_ABORT;
			return 0;
		}
		memcpy(send,buf_ptr,data_len);
		send[data_len] = 0x0d; // enter as ending-character // send[data_len] =0x1A; // 0x1A = CTRL + Z
		ret = at_cmd_send(fd, send, data_len + 1, t1, t2) - 1;
		if (at_cmd_receive(fd, resp, min(sizeof(resp), 6), t1, t2) == 6
				&& (resp_ptr = strstr(resp, "OK")) != NULL) {
			*errcode = REMOTE_MODULE_RW_NORMAL;
			send_len += ret;
			len -= ret;
			buf_ptr += ret;
			PRINTF("%s Send %d HEX bytes OK\n", __FUNCTION__, ret);
			if (len <= 0)
				return send_len;
			else
				continue;
		}
		else {
			*errcode = REMOTE_MODULE_RW_ABORT;
			PRINTF("%s FAIL\n", __FUNCTION__);
			return 0;
		}
	}
	return 0;
}

int m590e_receive(int fd, BYTE *buf, int maxlen, int timeout, int *errcode)
{
	int ret,data_len = 0;
	char resp[1024] = {0},*resp_ptr = resp;
	char send[1024] = {0};
	int t1 = M590E_READ_TIMEOUT, t2 = M590E_WRITE_TIMEOUT;

	if(fd<0)
		return FALSE;

	if((ret = at_cmd_receive(fd, resp, sizeof(resp), t1, t2)) > 0)
		if(strstr(resp, "$MYURCREAD: 0") == NULL){
			PRINTF("%s Not found '$MYURCREAD'\n", __FUNCTION__);
			return 0;
		}

	snprintf(send,sizeof(send),"AT$MYNETREAD=%d,2048\r",M590E_SOCKET_ID);
	ret = at_cmd_sub(fd, send, resp, sizeof(resp), t1, t2, TRUE);

	if(ret<0){
		PRINTF("%s AT CMD send FAIL for AT$MYNETREAD=%d,2048\n", __FUNCTION__,M590E_SOCKET_ID);
		return 0;
	}

	if((resp_ptr = strstr(resp, "$MYNETREAD")) == NULL){ // check receive  // $MYNETREAD: 0,33
		PRINTF("%s Not found '$MYNETREAD'\n", __FUNCTION__);
		*errcode = REMOTE_MODULE_RW_ABORT;
		return 0;
	}

	if((resp_ptr = strstr(resp, ":")) == NULL){
		PRINTF("%s Not found ':'\n", __FUNCTION__);
		*errcode = REMOTE_MODULE_RW_ABORT;
		return 0;
	}

	if((resp_ptr = strstr(resp,",")) == NULL){ /// \r = enter, \n = next line
		PRINTF("%s Not found ','\n");
		*errcode = REMOTE_MODULE_RW_ABORT;
		return 0;
	}
	sscanf(resp_ptr + 1, "%d", &data_len); //Only close display-back can we get data_len
	if ((resp_ptr = strstr(resp_ptr, "\n")) == NULL) {
		PRINTF("%s Not found '\\n'(character)",__FUNCTION__);
		*errcode = REMOTE_MODULE_RW_ABORT;
		return 0;
	}

	if(data_len > 0){
		memcpy(buf,resp_ptr + 1,data_len);
		PRINTF("%s %d bytes\n", __FUNCTION__, data_len);
		*errcode = REMOTE_MODULE_RW_NORMAL;
		return data_len;
	}
	PRINTF("%s: unknown addressed reason for receive\n", __FUNCTION__);
	return 0;
}

int m590e_shutdown(int fd)
{
	int ret = 0;
	char resp[1024] = {0}, *resp_ptr = resp;
	char send[1024] = {0};
	int t1 = M590E_READ_TIMEOUT, t2 = M590E_WRITE_TIMEOUT;

	if(fd < 0)
		return FALSE;
	snprintf(send, sizeof(send), "AT$MYNETCLOSE=%d\r", M590E_SOCKET_ID);
	ret = at_cmd(fd, send, resp, sizeof(resp), t1, t2);
	if(ret <= 0)
		return FALSE;
	if((resp_ptr = strstr(resp, "OK"))== NULL)
		return FALSE;
	return TRUE;
}

BOOL m590e_getip(int fd, char *ipstr)
{
	if(strlen(m590e_ip_str)> 0){
		if(ipstr){
			PRINTF(m590e_ip_str);
			strcpy(ipstr, m590e_ip_str);
		}
		return TRUE;
	}else{
		return FALSE;
	}
}
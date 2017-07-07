/*
 * th_upgprscdma.c
 *
 *  Created on: 2015-8-11
 *      Author: Johnnyzhang
 */



#include "threads.h"
#include "main.h"
#include "common.h"
#include "msg_que.h"
#include "protocol.h"
#include "up_comm.h"
#include "menu.h"
#include "devices.h"
#include "gprscdma.h"
#include "f_param.h"
#include "spont_alarm.h"
#include "language.h"

#define CONFIG_GPRSCDMA_THREAD_SLEEP 100

static UP_COMM_PRIVATE gprscdma_private = {
	.packetID = 1,
	.save_hb_packetID = 1,
};

static BOOL gprscdma_device_init(struct UP_COMM_ST *up);
static BOOL gprscdma_tcpip_connect(struct UP_COMM_ST *up);
static INT32 gprscdma_fep_receive(struct UP_COMM_ST *up, int timeout);
static BOOL gprscdma_fep_send(struct UP_COMM_ST *up);
static void gprscdma_disconnect(struct UP_COMM_ST *up);

static UP_COMM_INTERFACE gprscdma_comm = {
		.describe = "GPRS/CDMA",
		.fd = -1,
		.up_status = e_up_offline,
		.que_in = MSG_QUE_GPRSCDMA_IN,
		.que_out = MSG_QUE_GPRSCDMA_OUT,
		.need_diag = FALSE,
		.device_init = gprscdma_device_init,
		.connect = gprscdma_tcpip_connect,
		.login = NULL,
		.logout = NULL,
		.heartbeat_cycle = 60,
		.timeout = -1,
		.comm_receive = gprscdma_fep_receive,
		.comm_send = gprscdma_fep_send,
		.heartbeat_request = NULL,
		.disconnect = gprscdma_disconnect,
		.private = &gprscdma_private,
};

static BOOL check_gprscdma_online(struct UP_COMM_ST *up) /// change by wd 20170608
{
	BOOL ret = (up->up_status == e_up_online);
	static bool prev_state = false;

	if (ret && prev_state == false) {
		lcd_update_info(c_login_ok_str);
		prev_state = true;
	}else{
		if(!ret)
			prev_state = false;
	}

	lcd_update_head_info();
	return ret;
}

static BOOL gprscdma_device_init(struct UP_COMM_ST *up)
{
	return !!modem_check(modem_device, modem_lockname, MODEM_DEFAULT_BAUD);
}

static BOOL gprscdma_tcpip_connect(struct UP_COMM_ST *up)
{
	char ip_addr[16];
	BYTE host_ip[4], host_port[2];
	short ip_port;

	if (remote_ppp_connect(modem_device, modem_lockpid_name, MODEM_DEFAULT_BAUD_STR))
	{
		memset(host_ip, 0, sizeof(host_ip));
		memset(host_port, 0, sizeof(host_port));
		memset(ip_addr, 0, sizeof(ip_addr));
		fparam_get_value(FPARAMID_COMM_HOST_IP_PRI, host_ip, sizeof(host_ip));
		fparam_get_value(FPARAMID_COMM_HOST_PORT_PRI, host_port, sizeof(host_port));
		snprintf(ip_addr, sizeof(ip_addr), "%u.%u.%u.%u", host_ip[0], host_ip[1], host_ip[2], host_ip[3]);
		ip_port = (host_port[0] << 8) + host_port[1];
		if (g_socket_type == UP_COMM_SOCKET_TYPE_TCP){
			up->fd = remote_tcpudp_connect(0, ip_addr, ip_port, 30 * 1000);
		}
		else {
			up->fd = remote_tcpudp_connect(1, ip_addr, ip_port, 30 * 1000);
		}
		if (up->fd > 0)
			return TRUE;
		else {
			PRINTF("GPRS/CDMA TCP/UCP connect fail\n");
			return FALSE;
		}
	}
	else {
		PRINTF("GPRS/CDMA PPP connect fail\n");
		return FALSE;
	}
}

static INT32 gprscdma_fep_receive(struct UP_COMM_ST *up, int timeout)
{
	RECEIVE_BUFFER *receive = up->receive;
	UINT8 buf[CONFIG_MAX_APDU_LEN];
	INT32 len;
	int errcode;

	if (timeout < 0) {
		timeout = 5000;
	}
	if (up->fd < 0)
		return -1;
	if (wait_for_ready(up->fd, timeout, 0) > 0) {
		len = remote_tcpudp_read(up->fd, buf, sizeof(buf), timeout, &errcode);
		if(len > 0){
			PRINTB("From GPRS/CDMA: ", buf, len);
			receive_add_bytes(receive, buf, len);
			return len;
		}
		else if(len <= 0){
			if (errcode == REMOTE_MODULE_RW_ISP_CLOSE_CONNECT) { /// when receive the closed stirng from module
				up->up_status = e_up_offline;
				up->up_connect_status = e_up_disconnected;
				LOG_PRINTF("Module GPRS disconnect in function: %s, Cause: may receive close info from the module\n", __func__);
			}else{ // other situations: like when remove SIM card // other errcode, all len = 0
				up->up_status = e_up_offline;
				up->up_connect_status = e_up_disconnected;
				LOG_PRINTF("Module GPRS disconnect in function: %s, Cause: SIM card removed maybe\n", __func__);
			}
			PRINTF("GPRS/CDMA receive ERROR, may the SIM card was removed, "
					"nonetheless TRY to disconnect and reconnect to the main-station\n");
			return -1;
		}
	}
	return 0;
}

static BOOL gprscdma_fep_send(struct UP_COMM_ST *up)
{
	UINT8 buf[CONFIG_MAX_APDU_LEN];
	INT32 len;
	int errcode;

	if (up->fd < 0)
		return FALSE;
	msg_que_get(MSG_QUE_GPRSCDMA_OUT, buf, sizeof(buf), &len, MSG_QUE_NO_STAMP);
	if(len <= 0)
		return FALSE;
	PRINTB("To GPRS/CDMA: ", buf, len);
	if(remote_tcpudp_write(up->fd, buf, len, &errcode) == len)
		return TRUE;
	else {
		if (errcode != REMOTE_MODULE_RW_NORMAL) {
			up->up_status = e_up_offline;
			up->disconnect(up);
			up->up_connect_status = e_up_disconnected;
			PRINTF("GPRS/CDMA OFFLINE for send ERROR\n");
		}
		return FALSE;
	}
}

static void gprscdma_disconnect(struct UP_COMM_ST *up)
{
	remote_module_close(modem_lockname);
}

int fep_is_connect(void)
{
	return gprscdma_comm.up_status == e_up_online;
}

void *th_upgprscdma(void * arg)
{
	RECEIVE_BUFFER receive;
	BYTE hb_cycle[2];

	print_thread_info();
	gprscdma_comm.private->spont_chnidx = spontalarm_register_channel(gprscdma_comm.describe);
	receive_buffer_init(&receive, CONFIG_MAX_APDU_LEN);
	gprscdma_comm.receive = &receive;
	gprscdma_comm.idle_uptime = uptime();
	while (!g_terminated) {
		notify_watchdog();
		fparam_get_value(FPARAMID_HEARTBEAT_CYCLE, hb_cycle, 2);
		gprscdma_comm.heartbeat_cycle = (hb_cycle[1] << 8) + hb_cycle[0];
		up_comm_proc(&gprscdma_comm);
		check_gprscdma_online(&gprscdma_comm);
		msleep(CONFIG_GPRSCDMA_THREAD_SLEEP);
	}
	gprscdma_disconnect(&gprscdma_comm);
	receive_buffer_destory(&receive);
	return NULL;
}

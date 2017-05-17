/*
 * th_upeth.c
 *
 *  Created on: 2015-8-18
 *      Author: Johnnyzhang
 */


#include "threads.h"
#include "main.h"
#include "common.h"
#include "msg_que.h"
#include "up_comm.h"
#include "f_param.h"
#include "protocol_gasup.h"
#include "spont_alarm.h"

#define CONFIG_ETH_THREAD_SLEEP 5000

static UP_COMM_PRIVATE eth_private = {
	.packetID = 1,
	.save_hb_packetID = 1,
};

static BOOL eth_connect(struct UP_COMM_ST *up);
static INT32 eth_fep_receive(struct UP_COMM_ST *up, int timeout);
static BOOL eth_fep_send(struct UP_COMM_ST *up);
static void eth_disconnect(struct UP_COMM_ST *up);

static UP_COMM_INTERFACE eth_comm = {
		.describe = "Ethernet",
		.fd = -1,
		.up_status = e_up_offline,
		.que_in = MSG_QUE_ETH_IN,
		.que_out = MSG_QUE_ETH_OUT,
		.need_diag = FALSE,
		.device_init = NULL,
		.connect = eth_connect,
		.login = NULL,
		.logout = NULL,
		.timeout = -1,
		.comm_receive = eth_fep_receive,
		.comm_send = eth_fep_send,
		.heartbeat_request = NULL,
		.disconnect = eth_disconnect,
		.private = &eth_private,
};

static void set_nonblocking(INT32 fd, INT32 which)
{
	INT32 flags;

	if (fd >= 0) {
		flags = fcntl(fd, F_GETFL, 0);
		if (which)
			flags |= O_NONBLOCK;
		else
			flags &= (~O_NONBLOCK);
		fcntl(fd, F_SETFL, flags);
	}
}

static int open_tcp(const char *addr, short port, int timeout)
{
	int fd, error, tmp;
	UINT32 len;
	struct sockaddr_in sa_in;
	struct timeval tv;
	fd_set fds;

	if ((fd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
		return -1;
	bzero(&sa_in, sizeof(sa_in));
	sa_in.sin_family = AF_INET;
	inet_aton(addr, &sa_in.sin_addr);
	sa_in.sin_port = htons(port);
	set_nonblocking(fd, 1);
	PRINTF("Connecting to %s:%d, timeout is %d ms\n", addr, port, timeout);
	tmp = connect(fd, (struct sockaddr *)&sa_in, sizeof(sa_in));
	if (tmp < 0) {
		error = errno;
		if (error != EINTR && error != EINPROGRESS) {
			close(fd);
			PRINTF("Connect fail, error code:%d, reason:%s\n",
				error, strerror(error));
			return -1;
		}
	}
	while (!g_terminated && timeout > 0) {
		tv.tv_sec = 0;
		tv.tv_usec = 0;
		FD_ZERO(&fds);
		FD_SET(fd, &fds);
		tmp = select(fd + 1, NULL, &fds, NULL, &tv);
		if (tmp > 0) {
			len = sizeof(error);
			if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len) < 0
				|| error) {
				close(fd);
				PRINTF("Connect fail, error code:%d, reason:%s\n",
					error, strerror(error));
				return -1;
			}
			else {
				set_nonblocking(fd, 0);
				PRINTF("TCP Connect %s:%d OK, fd is %d\n", addr, port, fd);
				return fd;
			}
		}
		else if (tmp < 0) {
			error = errno;
			if (error != EINTR && error != EINPROGRESS) {
				close(fd);
				PRINTF("Connect fail, error code:%d, reason:%s\n",
					error, strerror(error));
				return -1;
			}
		}
		msleep(100);
		timeout -= 100;
	}
	close(fd);
	PRINTF("Connect timeout\n");
	return -1;
}

static int open_udp(const char *addr, short port)
{
	int fd;
	struct sockaddr_in sa_in;

	if ((fd = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
		return -1;
	memset((INT8 *)&sa_in, 0, sizeof(sa_in));
	sa_in.sin_family = AF_INET;
	sa_in.sin_port = htons(port + 1);
	sa_in.sin_addr.s_addr = INADDR_ANY;
	bind(fd, (struct sockaddr *)&sa_in, sizeof(sa_in));
	memset((INT8 *)&sa_in, 0, sizeof(sa_in));
	sa_in.sin_family = AF_INET;
	sa_in.sin_port = htons(port);
	sa_in.sin_addr.s_addr = inet_addr(addr);
	if (connect(fd, (struct sockaddr *)&sa_in, sizeof(sa_in)) < 0) {
		close(fd);
		PRINTF("UDP Connect fail\n");
		return -1;
	}
	PRINTF("UDP Connect ok, fd is %d\n", fd);
	return fd;
}

static BOOL eth_connect(struct UP_COMM_ST *up)
{
	char ip_addr[16];
	BYTE host_ip[4], host_port[2];
	short ip_port;
	char ipaddr[32], dstaddr[16];

	if (!get_network_addr("eth0", ipaddr, dstaddr))
		return FALSE;
	memset(host_ip, 0, sizeof(host_ip));
	memset(host_port, 0, sizeof(host_port));
	memset(ip_addr, 0, sizeof(ip_addr));
	fparam_get_value(FPARAMID_COMM_HOST_IP_PRI, host_ip, sizeof(host_ip));
	fparam_get_value(FPARAMID_COMM_HOST_PORT_PRI, host_port, sizeof(host_port));
	snprintf(ip_addr, sizeof(ip_addr), "%u.%u.%u.%u", host_ip[0], host_ip[1], host_ip[2], host_ip[3]);
	ip_port = (host_port[0] << 8) + host_port[1];
	if (g_socket_type == UP_COMM_SOCKET_TYPE_TCP){
		up->fd = open_tcp(ip_addr, ip_port, 6 * 1000);
	}
	else{
		up->fd = open_udp(ip_addr, ip_port);
	}
	if (up->fd >= 0)
		return TRUE;
	else
		return FALSE;
}

static INT32 eth_fep_receive(struct UP_COMM_ST *up, int timeout)
{
	RECEIVE_BUFFER *receive = up->receive;
	UINT8 buf[CONFIG_MAX_APDU_LEN];
	INT32 len;

	if (timeout < 0) {
		timeout = 500;
	}
	if (up->fd < 0)
		return -1;
	if (wait_for_ready(up->fd, timeout, 0) > 0) {
		len = plt_gasup_read_socket(up->fd, buf, sizeof(buf), timeout);
		if(len > 0){
			PRINTB("From ETH: ", buf, len);
			receive_add_bytes(receive, buf, len);
			return len;
		}
		else if(len <= 0){
			up->up_status = e_up_offline;
			up->up_connect_status = e_up_disconnected;
			close(up->fd);
			up->fd = -1;
			PRINTF("ETH Disconnected\n");
			return -1;
		}
	}
	return 0;
}

static BOOL eth_fep_send(struct UP_COMM_ST *up)
{
	UINT8 buf[CONFIG_MAX_APDU_LEN];
	INT32 len;

	if (up->fd < 0)
		return FALSE;
	msg_que_get(MSG_QUE_ETH_OUT, buf, sizeof(buf), &len, MSG_QUE_NO_STAMP);
	if(len <= 0)
		return FALSE;
	PRINTB("To ETH: ", buf, len);
	if(safe_write_timeout(up->fd, buf, len, 2 * 60 * 1000) == len)
		return TRUE;
	else
		return FALSE;
}

static void eth_disconnect(struct UP_COMM_ST *up)
{
	up->up_status = e_up_offline;
	up->up_connect_status = e_up_disconnected;
	shutdown(up->fd, SHUT_RDWR);
	close(up->fd);
	return;
}

void *th_upeth(void *arg)
{
	RECEIVE_BUFFER receive;
	BYTE hb_cycle[2];

	print_thread_info();
	eth_comm.private->spont_chnidx = spontalarm_register_channel(eth_comm.describe);
	receive_buffer_init(&receive, CONFIG_MAX_APDU_LEN);
	eth_comm.receive = &receive;
	eth_comm.idle_uptime = uptime();
	while (!g_terminated) {
		notify_watchdog();
		fparam_get_value(FPARAMID_HEARTBEAT_CYCLE, hb_cycle, 2);
		eth_comm.heartbeat_cycle = (hb_cycle[0] << 8) + hb_cycle[1];
		up_comm_proc(&eth_comm);
		msleep(CONFIG_ETH_THREAD_SLEEP);
	}
	eth_disconnect(&eth_comm);
	receive_buffer_destory(&receive);
	return NULL;
}

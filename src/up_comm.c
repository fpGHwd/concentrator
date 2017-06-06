/*
 * up_comm.c
 *
 *  Created on: 2015-8-15
 *      Author: Johnnyzhang
 */

#include "up_comm.h"
#include "common.h"
#include "msg_que.h"
#include "protocol.h"
#include "protocol_gasup.h"
#include "spont_alarm.h"
#include "f_param.h"

static BOOL up_comm_login(UP_COMM_INTERFACE *up)
{
	UINT8 buffer[CONFIG_MAX_APDU_LEN];
	UINT32 packetID;
	int len;
	UINT8 address[7] = {0};

	if (up->login)
		return up->login(up);
	else {
		packetID = up->private->packetID;
		len = plt_gasup_pack_special(PTL_GASUP_FN_COMM_REGISTER, buffer,
				sizeof(buffer), NULL, 0, packetID);
		if (len <= 0)
			return FALSE;
		msg_que_put(up->que_out, buffer, len, MSG_QUE_NO_STAMP);
		PRINTF("LOGIN Request\n");
		if (!up->comm_send(up)) {
			PRINTF("Login FAIL by %s (SEND FAIL)\n", up->describe);
			return FALSE;
		}
		up->private->packetID++;
		if (up->comm_receive(up, 3 * 60 * 1000) > 0) {
			len = get_data_from_receive(up->receive, buffer, sizeof(buffer));
			if (len < 0)
				return FALSE;
			receive_del_bytes(up->receive, len);
			fparam_get_value(FPARAMID_CON_ADDRESS, address, 7);
			if (plt_gasup_check_pack_special(address, PTL_GASUP_FN_COMM_REGISTER, buffer, len, packetID)) {
				PRINTF("Login OK by %s\n", up->describe);
				return TRUE;
			}
			else {
				PRINTF("Login FAIL by %s (RESPONSE TIMEOUT)\n", up->describe);
				return FALSE;
			}
		}
		else
			return FALSE;
	}
}

static BOOL up_comm_heartbeat_proc(struct UP_COMM_ST *up)
{
	UINT8 buffer[CONFIG_MAX_APDU_LEN];
	int len;
	BOOL ret;

	if (up->private->hb_status == e_up_wait_response) {
		if (uptime() - up->last_heartbeat_request > 3 * 60 * 1000) {
			up->disconnect(up);
			up->up_connect_status = e_up_disconnected;
		}
		return FALSE;
	}

	if (uptime() - up->idle_uptime > up->heartbeat_cycle) {
		up->private->hb_status = e_up_request;
	}
	if (up->private->hb_status == e_up_request) {
		if (up->heartbeat_request) {
			ret = up->heartbeat_request(up);
			up->idle_uptime = up->last_heartbeat_request = uptime();
			up->private->hb_status = e_up_wait_response;
			return ret;
		}
		else {
			len = plt_gasup_pack_special(PTL_GASUP_FN_COMM_HEARTBEAT, buffer,
					sizeof(buffer), NULL, 0, up->private->packetID);
			if (len <= 0)
				return FALSE;
			msg_que_put(up->que_out, buffer, len, MSG_QUE_NO_STAMP);
			PRINTF("HEART BEAT Request\n");
			up->comm_send(up);
			up->private->save_hb_packetID = up->private->packetID;
			up->private->packetID++;
			up->idle_uptime = up->last_heartbeat_request = uptime();
			up->private->hb_status = e_up_wait_response;
			return TRUE;
		}
	}
	return TRUE;
}

void up_comm_spont_alarm(UP_COMM_INTERFACE *up)
{
	UINT8 buffer[CONFIG_MAX_APDU_LEN], data[CONFIG_MAX_APDU_LEN];
	UINT32 packetID;
	int len;
	UINT16 fn;

	if (up->private->spont_status == e_up_wait_response) {
		if (uptime() - up->private->spont_tt > 3 * 60 * 1000) {
			spontalarm_reset_info(up->private->spont_chnidx);
			up->private->spont_status = e_up_wait_none;
		}
	}
	packetID = up->private->packetID;
	len = spontalarm_get_data(data, sizeof(data), up->private->spont_chnidx, &fn);
	len = plt_gasup_pack_special(fn, buffer, sizeof(buffer), data, len, packetID);
	if (len <= 0)
		return;
	msg_que_put(up->que_out, buffer, len, MSG_QUE_NO_STAMP);
	PRINTF("SPONT ALARM FN: %d\n", fn);
	up->comm_send(up);
	up->private->spont_status = e_up_wait_response;
	up->idle_uptime = up->private->spont_tt = uptime();
	up->private->save_spont_packetID = up->private->packetID;
	up->private->packetID++;
}

void up_comm_proc(UP_COMM_INTERFACE *up)
{
	if (up->up_connect_status == e_up_disconnected) {
		if (up->device_init) {
			if (!up->device_init(up))
				return;
		}
		if (!up->connect || !up->connect(up)) {
			up->up_connect_status = e_up_disconnected;
			return;
		}
		up->up_connect_status = e_up_connected;
		if (!up_comm_login(up))
			return;
		up->up_status = e_up_online;
	}
	else {
		if (up->up_status != e_up_online) {
			if (!up_comm_login(up))
				return;
			up->up_status = e_up_online;
		}
		if (up->comm_receive(up, up->timeout) > 0) {
			up->idle_uptime = uptime();
		}
		if (uptime() - up->idle_uptime > 7 * 60){
			up->need_diag = TRUE;
			up->idle_uptime = uptime();
		}
		up_protocol_proc(up->que_in, up->que_out, up->receive, up->private);
		up->comm_send(up);
		up_comm_heartbeat_proc(up);
		up_comm_spont_alarm(up);
	}
}

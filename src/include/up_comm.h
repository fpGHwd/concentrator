/*
 * up_comm.h
 *
 *  Created on: 2015Äê8ÔÂ15ÈÕ
 *      Author: Johnnyzhang
 */

#ifndef UP_COMM_H_
#define UP_COMM_H_

#include "typedef.h"
#include "protocol.h"

typedef enum {
	UP_COMM_SOCKET_TYPE_TCP,
	UP_COMM_SOCKET_TYPE_UDP,
} UP_COMM_SOCKET_TYPE;

typedef enum {
	e_up_offline,
	e_up_online,
} E_UP_STATUS;

typedef enum {
	e_up_disconnected,
	e_up_connected,
} E_UP_CONNECT_STATUS;

typedef enum {
	e_up_wait_none,
	e_up_request = 1,
	e_up_wait_response,
	e_up_finish,
} E_UP_WAIT_STATUS;

typedef struct UP_COMM_PRIVATE_ST {
	INT32 packetID;

	E_UP_WAIT_STATUS hb_status;
	INT32 save_hb_packetID;

	E_UP_WAIT_STATUS spont_status;
	long spont_tt;
	INT32 save_spont_packetID;
	int spont_chnidx;
} UP_COMM_PRIVATE;

typedef struct UP_COMM_ST {
	const char *describe;
	INT32 fd;
	INT32 que_in;
	INT32 que_out;

	long idle_uptime;

	BOOL need_diag;

	RECEIVE_BUFFER *receive;

	BOOL (*device_init)(struct UP_COMM_ST *);

	E_UP_CONNECT_STATUS up_connect_status;
	BOOL (*connect)(struct UP_COMM_ST *);
	void (*disconnect)(struct UP_COMM_ST *);

	E_UP_STATUS up_status;
	BOOL (*login)(struct UP_COMM_ST *);
	BOOL (*logout)(struct UP_COMM_ST *);
	long last_heartbeat_request;
	long heartbeat_cycle; // unit is second
	BOOL (*heartbeat_request)(struct UP_COMM_ST *);

	INT32 timeout;
	INT32 (*comm_receive)(struct UP_COMM_ST *, int timeout);
	BOOL (*comm_send)(struct UP_COMM_ST *);
	UP_COMM_PRIVATE *private;
} UP_COMM_INTERFACE;

void do_nothing(void);
BOOL do_nothing_ret(void);
void up_comm_proc(UP_COMM_INTERFACE *up);

#endif /* UP_COMM_H_ */

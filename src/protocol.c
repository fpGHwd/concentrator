/*
 * protocol.c
 *
 *  Created on: 2015-8-15
 *      Author: Johnnyzhang
 */

#include "protocol.h"
#include "main.h"
#include "common.h"
#include "msg_que.h"
#include "protocol_gasup.h"
#include "protocol_cjt188.h"

typedef struct {
	E_PROTOCOL_TYPE type;
	const INT8 *describe;
	PROTOCOL_PARSE_FUNC parse;
	PROTOCOL_PROCEDURE_FUNC proc;
} PROTOCOL_T; // protocol type's enumeration a implement

#define INIT_MSG_QUE_DESCRIBE(x) {x, #x}

static struct {
	INT32 idx;
	const INT8 *describe;
} msg_que_describe[] = {
INIT_MSG_QUE_DESCRIBE(MSG_QUE_GPRSCDMA_IN),
INIT_MSG_QUE_DESCRIBE(MSG_QUE_GPRSCDMA_OUT),
INIT_MSG_QUE_DESCRIBE(MSG_QUE_ETH_IN),
INIT_MSG_QUE_DESCRIBE(MSG_QUE_ETH_OUT), }; /// core structure

const INT8 *msg_que_idx_to_str(INT32 idx) /// index to string
{
	INT32 i;

	for (i = 0; i < ARRAY_SIZE(msg_que_describe); i++) {
		if (idx == msg_que_describe[i].idx) {
			return msg_que_describe[i].describe;
		}
	}
	return "Invalid MSG_QUE";
}

const char *msg_que_idx_to_str_new(int index_for_queue_type) {
	int i;

	for (i = 0; i < (sizeof(msg_que_describe) / sizeof(msg_que_describe[0]));
			i++) {
		if (index_for_queue_type == i) {
			return msg_que_describe[i].describe;
		} else {
			PRINTF("not this type\n");
		}
	}
	return "Invalid MSG_QUE";/*exception*/
}

static PROTOCOL_T protocol_arr[] = { { e_up_gasmeter, "GAS Meter Up Protocol",
		plt_gasup_parse, plt_gasup_proc }, };

#define PTL_GROUP_UP              0
#define PTL_GROUP_METER           1

static E_PROTOCOL_TYPE protocol_type_arr[][MAX_PROTOCOL_CNT] = {
		{ e_up_gasmeter, }, { e_ptl_unknown, }, };

/*/
 typedef struct {
 UINT8 *head;
 UINT8 *tail; 
 INT32 len; 
 UINT8 *buf; 
 } RECEIVE_BUFFER;
 /*/
void receive_buffer_init(RECEIVE_BUFFER *receive, INT32 len) {
	if (NULL == receive)
		return;
	/*
	 if((receive->buf = malloc(len)) == NULL){ /// add by wd
	 return;
	 }
	 */
	receive->buf = malloc(len);
	receive->len = len;
	receive->head = receive->buf;
	receive->tail = receive->buf;
}

void receive_buffer_destory(RECEIVE_BUFFER *receive) {
	if (receive && receive->buf) {
		free(receive->buf); /// free the malloc pointer and space /// no pointer but only space
	}
	receive->len = 0;
	receive->buf = NULL;
	receive->head = NULL;
	receive->tail = NULL;
}

static BOOL receive_is_valid(const RECEIVE_BUFFER *receive) {
	UINT8 *max_ptr;

	if (NULL == receive)
		return FALSE;
	if (receive->len <= 0)
		return FALSE;
	if (NULL == receive->buf || NULL == receive->head || NULL == receive->tail)
		return FALSE;
	if ((receive->head < receive->buf) || (receive->tail < receive->buf))
		return FALSE;
	max_ptr = receive->buf + receive->len;
	if ((receive->head > max_ptr) || (receive->tail > max_ptr))
		return FALSE;
	if (abs(receive->tail - receive->head) > receive->len)
		return FALSE;
	return TRUE;
}

INT32 get_data_from_receive(const RECEIVE_BUFFER *receive, UINT8 *buf,
		INT32 maxlen) {
	INT32 len;
	UINT8 *ptr;

	if (!receive_is_valid(receive))
		return 0;
	if ((receive->tail - receive->head) > 0) {
		len = (receive->tail - receive->head);
		if (maxlen >= len) {
			memcpy(buf, receive->head, len);
			return len;
		} else {
			return 0;
		}
	} else if ((receive->tail - receive->head) < 0) {
		if (maxlen < receive->len - (receive->head - receive->tail)) /// circle model
			return 0;
		len = (receive->len - (receive->head - receive->buf));
		ptr = buf;
		memcpy(ptr, receive->head, len);
		ptr += len;
		len = receive->tail - receive->buf;
		memcpy(ptr, receive->buf, len);
		ptr += len;
		return ptr - buf;
	} else {
		return 0;
	}
}

BOOL receive_add_bytes(RECEIVE_BUFFER *receive, const UINT8 *buf, INT32 len) /// len = length of (*buf)
{
	INT32 empty_len, tmp;
	const UINT8 *ptr;

	if (!receive_is_valid(receive)) // no full
		return FALSE;
	if ((receive->tail - receive->head) >= 0) {
		empty_len = receive->len - (receive->tail - receive->head);
		if (len > empty_len)
			return FALSE; // connot add bytes commly
		tmp = receive->len - (receive->tail - receive->buf);
		if (tmp >= len) {
			memcpy(receive->tail, buf, len);
			receive->tail += len;
			return TRUE;
		} else {
			ptr = buf;
			memcpy(receive->tail, ptr, tmp);
			ptr += tmp;
			memcpy(receive->buf, ptr, len - tmp);
			receive->tail = receive->buf + (len - tmp);
			return TRUE;
		}
	} else { // receive->tail - receive->head < 0
		// -1 : forbid to fill data in tail,that means that the tail is not empty permanently
		empty_len = receive->head - receive->tail - 1;
		if (len > empty_len)
			return FALSE;
		memcpy(receive->tail, buf, len);
		receive->tail += len;
		return TRUE;
	}
}

BOOL receive_del_bytes(RECEIVE_BUFFER *receive, INT32 len) {
	INT32 used_len, tmp;

	if (!receive_is_valid(receive))
		return FALSE;
	if ((receive->tail - receive->head) >= 0) {
		used_len = receive->tail - receive->head;
		if (len > used_len) {
			memset(receive->buf, 0, receive->len);
			receive->head = receive->buf;
			receive->tail = receive->buf;
			return FALSE;
		} else {
			memset(receive->head, 0, len);
			receive->head += len;
			return TRUE;
		}
	} else {
		used_len = receive->len - (receive->head - receive->tail);
		if (len > used_len) {
			memset(receive->buf, 0, receive->len);
			receive->head = receive->buf;
			receive->tail = receive->buf;
			return FALSE;
		}
		tmp = receive->len - (receive->head - receive->buf);
		if (tmp >= len) {
			memset(receive->head, 0, len);
			receive->head += len;
			return TRUE;
		} else {
			memset(receive->head, 0, tmp);
			memset(receive->buf, 0, len - tmp);
			receive->head = receive->buf + (len - tmp);
			return TRUE;
		}
	}
}

static PROTOCOL_T *parse_protocol(RECEIVE_BUFFER *receive, UINT8 *outbuf,
		INT32 maxlen, INT32 *outlen) /// receive(buff) to get the protocol type
{
	UINT8 buf[CONFIG_MAX_APDU_LEN], *ptr = buf;
	INT32 i, len, prot_len;

	*outlen = 0;
	if (!receive_is_valid(receive))
		return NULL;
	len = get_data_from_receive(receive, buf, sizeof(buf));
	if (len <= 0)
		return NULL;
	while (!g_terminated && receive_is_valid(receive)) {
		for (i = 0; i < ARRAY_SIZE(protocol_arr); i++) {
			if ((prot_len = protocol_arr[i].parse(ptr, len)) > 0) {
				receive_del_bytes(receive, prot_len);
				if (maxlen < prot_len)
					return NULL;
				memcpy(outbuf, ptr, prot_len);
				*outlen = prot_len;
				return &protocol_arr[i];
			}
		}
		ptr++;
		if (!receive_del_bytes(receive, 1))
			break;
	}
	return NULL;
}

static BOOL is_valid_protocol(INT32 idx, E_PROTOCOL_TYPE protocol) /// if registered(in code) protocol
{
	INT32 i;

	if (idx < 0 || idx > ARRAY_SIZE(protocol_type_arr))
		return FALSE;
	for (i = 0; i < ARRAY_SIZE(protocol_type_arr[idx]); i++) {
		if (protocol_type_arr[idx][i] == protocol)
			return TRUE;
	}
	return FALSE;
}

#define TO_STR(x) msg_que_idx_to_str(x)  /// ixdex to describe
static INT32 protocol_proc(INT32 in_idx, INT32 out_idx, RECEIVE_BUFFER *receive,
		UINT8 *req, INT32 max_len, INT32 ptl_group, void *priv) {
	PROTOCOL_T *protocol_type;
	INT32 reqlen = 0;

	protocol_type = parse_protocol(receive, req, max_len, &reqlen);
	if (protocol_type == NULL
			|| !is_valid_protocol(ptl_group, protocol_type->type))
		return 0;
	PRINTF("The packet is %s from %s(%d)\n", protocol_type->describe,
			TO_STR(in_idx), in_idx); /// protocol, queue name, queue index(macro)
	if (reqlen <= 0) {
		return 0;
	}
	msg_que_put(in_idx, req, reqlen, MSG_QUE_NO_STAMP); /// request queue
	protocol_type->proc(in_idx, out_idx, CONFIG_MAX_APDU_LEN, priv);
	return 1;
}

INT32 up_protocol_proc(INT32 in_idx, INT32 out_idx, RECEIVE_BUFFER *receive,
		void *priv) {
	UINT8 req[CONFIG_MAX_APDU_LEN];
	return protocol_proc(in_idx, out_idx, receive, req, sizeof(req),
			PTL_GROUP_UP, priv);
}

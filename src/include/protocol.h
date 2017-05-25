/*
 * protocol.h
 *
 *  Created on: 2015-8-15
 *      Author: Johnnyzhang
 */

#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include "typedef.h"

#define CONFIG_MAX_APDU_LEN 16355

typedef struct {
	UINT8 *head;
	UINT8 *tail;
	INT32 len;
	UINT8 *buf;
} RECEIVE_BUFFER;

typedef enum {
	e_ptl_unknown,
	e_up_gasmeter,
	e_down_cjt188_2004,
} E_PROTOCOL_TYPE;

#define MAX_PROTOCOL_CNT  2

typedef INT32 (*PROTOCOL_PARSE_FUNC)(const UINT8 *buf, INT32 buf_len);
typedef void (*PROTOCOL_PROCEDURE_FUNC)(INT32 in_idx, INT32 out_idx,
		INT32 max_out_len, void *priv);

const INT8 *msg_que_idx_to_str(INT32 idx);
INT32 get_data_from_receive(const RECEIVE_BUFFER *receive, UINT8 *buf,
		INT32 maxlen);
BOOL receive_add_bytes(RECEIVE_BUFFER *receive, const UINT8 *buf, INT32 len);
BOOL receive_del_bytes(RECEIVE_BUFFER *receive, INT32 len);

INT32 up_protocol_proc(INT32 in_idx, INT32 out_idx, RECEIVE_BUFFER *receive,
		void *priv);
void receive_buffer_init(RECEIVE_BUFFER *receive, INT32 maxlen);
void receive_buffer_destory(RECEIVE_BUFFER *receive);

#endif /* PROTOCOL_H_ */

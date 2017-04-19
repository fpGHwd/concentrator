/*
 * protocol_gasup_fn.h
 *
 *  Created on: 2015-8-16
 *      Author: Johnnyzhang
 */

#ifndef PROTOCOL_GASUP_FN_H_
#define PROTOCOL_GASUP_FN_H_

#include "typedef.h"
#include "protocol_gasup.h"

typedef enum {
	VALVE_STATUS_ON = 0,
	VALVE_STATUS_OFF = 1,
	VALVE_STATUS_ABORT = 2,
	VALVE_STATUS_NONE = 3,
} VALVE_STATUS; /// valve status

/// 集中器和主站的业务
UINT32 ptl_gasup_fn_2001(const PTL_GASUP_MSG *msg, INT8 *outdata,
		INT32 max_outlen, INT32 *datalen, INT32 max_datalen); /// 主站通信测试
UINT32 ptl_gasup_fn_2002(const PTL_GASUP_MSG *msg, INT8 *outdata,
		INT32 max_outlen, INT32 *datalen, INT32 max_datalen);
UINT32 ptl_gasup_fn_2003(const PTL_GASUP_MSG *msg, INT8 *outdata,
		INT32 max_outlen, INT32 *datalen, INT32 max_datalen);
UINT32 ptl_gasup_fn_2004(const PTL_GASUP_MSG *msg, INT8 *outdata,
		INT32 max_outlen, INT32 *datalen, INT32 max_datalen);
UINT32 ptl_gasup_fn_2011(const PTL_GASUP_MSG *msg, INT8 *outdata,
		INT32 max_outlen, INT32 *datalen, INT32 max_datalen);
UINT32 ptl_gasup_fn_2012(const PTL_GASUP_MSG *msg, INT8 *outdata,
		INT32 max_outlen, INT32 *datalen, INT32 max_datalen);
UINT32 ptl_gasup_fn_2013(const PTL_GASUP_MSG *msg, INT8 *outdata,
		INT32 max_outlen, INT32 *datalen, INT32 max_datalen);
UINT32 ptl_gasup_fn_2014(const PTL_GASUP_MSG *msg, INT8 *outdata,
		INT32 max_outlen, INT32 *datalen, INT32 max_datalen);
UINT32 ptl_gasup_fn_2015(const PTL_GASUP_MSG *msg, INT8 *outdata,
		INT32 max_outlen, INT32 *datalen, INT32 max_datalen);
UINT32 ptl_gasup_fn_2016(const PTL_GASUP_MSG *msg, INT8 *outdata,
		INT32 max_outlen, INT32 *datalen, INT32 max_datalen);
UINT32 ptl_gasup_fn_2021(const PTL_GASUP_MSG *msg, INT8 *outdata,
		INT32 max_outlen, INT32 *datalen, INT32 max_datalen);
UINT32 ptl_gasup_fn_2022(const PTL_GASUP_MSG *msg, INT8 *outdata,
		INT32 max_outlen, INT32 *datalen, INT32 max_datalen);
UINT32 ptl_gasup_fn_2023(const PTL_GASUP_MSG *msg, INT8 *outdata,
		INT32 max_outlen, INT32 *datalen, INT32 max_datalen);
UINT32 ptl_gasup_fn_2024(const PTL_GASUP_MSG *msg, INT8 *outdata,
		INT32 max_outlen, INT32 *datalen, INT32 max_datalen);
UINT32 ptl_gasup_fn_2031(const PTL_GASUP_MSG *msg, INT8 *outdata,
		INT32 max_outlen, INT32 *datalen, INT32 max_datalen);
UINT32 ptl_gasup_fn_2032(const PTL_GASUP_MSG *msg, INT8 *outdata,
		INT32 max_outlen, INT32 *datalen, INT32 max_datalen);
UINT32 ptl_gasup_fn_2033(const PTL_GASUP_MSG *msg, INT8 *outdata,
		INT32 max_outlen, INT32 *datalen, INT32 max_datalen);
UINT32 ptl_gasup_fn_2034(const PTL_GASUP_MSG *msg, INT8 *outdata,
		INT32 max_outlen, INT32 *datalen, INT32 max_datalen);
UINT32 ptl_gasup_fn_2035(const PTL_GASUP_MSG *msg, INT8 *outdata,
		INT32 max_outlen, INT32 *datalen, INT32 max_datalen);
UINT32 ptl_gasup_fn_2036(const PTL_GASUP_MSG *msg, INT8 *outdata,
		INT32 max_outlen, INT32 *datalen, INT32 max_datalen);
UINT32 ptl_gasup_fn_2041(const PTL_GASUP_MSG *msg, INT8 *outdata,
		INT32 max_outlen, INT32 *datalen, INT32 max_datalen);
UINT32 ptl_gasup_fn_2042(const PTL_GASUP_MSG *msg, INT8 *outdata,
		INT32 max_outlen, INT32 *datalen, INT32 max_datalen);
UINT32 ptl_gasup_fn_2043(const PTL_GASUP_MSG *msg, INT8 *outdata,
		INT32 max_outlen, INT32 *datalen, INT32 max_datalen);
UINT32 ptl_gasup_fn_2044(const PTL_GASUP_MSG *msg, INT8 *outdata,
		INT32 max_outlen, INT32 *datalen, INT32 max_datalen);
UINT32 ptl_gasup_fn_2051(const PTL_GASUP_MSG *msg, INT8 *outdata,
		INT32 max_outlen, INT32 *datalen, INT32 max_datalen);
UINT32 ptl_gasup_fn_2052(const PTL_GASUP_MSG *msg, INT8 *outdata,
		INT32 max_outlen, INT32 *datalen, INT32 max_datalen);
UINT32 ptl_gasup_fn_2053(const PTL_GASUP_MSG *msg, INT8 *outdata,
		INT32 max_outlen, INT32 *datalen, INT32 max_datalen);
UINT32 ptl_gasup_fn_2061(const PTL_GASUP_MSG *msg, INT8 *outdata,
		INT32 max_outlen, INT32 *datalen, INT32 max_datalen);
UINT32 ptl_gasup_fn_2062(const PTL_GASUP_MSG *msg, INT8 *outdata,
		INT32 max_outlen, INT32 *datalen, INT32 max_datalen);

#endif /* PROTOCOL_GASUP_FN_H_ */

#ifndef __AT_H__
#define __AT_H__
//-----------------------------------------------------------------------------------------
#include "typedef.h"
//-----------------------------------------------------------------------------------------
#define AT_C_BUFSIZ 1024UL
//----------------------------------------------------------------------------------------

typedef struct _struct_ATCmd {

	UINT8 Cmd;

	void *Arg;

} ATCmd_t;

typedef struct _TcpSetupArg {

	UINT8 Link;

	UINT8 IP[16];

	UINT8 Port[6];
} TcpSetupArg_t;

typedef struct _TcpSendArg {

	UINT8 *Buffer;

	UINT8 Length;

	UINT8 Link;
} TcpSendArg_t;

typedef struct _ApnArg {

	UINT16 PDN;

	char APN[16];

	char Type[16];
} ApnArg_t;

typedef struct _AuthArg {

	UINT16 CID;

	UINT8 AuthType;

	char Name[32];

	char PSW[16];
} AuthArg_t;

typedef struct _Domainname_for_update_time {
	char *domain_p;
} DNArg_t;

typedef struct _struct_AT { /// at abstract struct/layer

	int (*Expect)(const UINT8 *RspBuffer);

	UINT8 *ptrBuffer;
	UINT16 Length;

	UINT16 TmoMax; // timeout

	UINT8 Stus;
	UINT8 Tick;
	UINT8 TryMax;  /// try max time
	UINT8 RcvCnt;
	UINT8 RcvMax;
	UINT8 SndSize;
	UINT8 RcvSize;
	UINT8 SndBuffer[AT_C_BUFSIZ];
	UINT8 RcvBuffer[AT_C_BUFSIZ];
} AT_t;
//------------------------------------------------------------------------------
enum DAT_FORMAT {
	DAT_HEX = 0u, DAT_TXT = 1u
};

enum CNCT_TYPE {
	TCP_CLIENT = 0, TCP_SERVER = 1, UDP = 2
};

enum _e_at_st {
	S_AT_IDL = 0, S_AT_BUSY, S_AT_ING, S_AT_SND, S_AT_RCV, S_AT_FIN, S_AT_ERR
};

enum at_c_def {
	AT_C_AT = 0,
	AT_C_WAIT_RDY,
	AT_C_GET_TYPE,
	AT_C_GET_VER,
	AT_C_GET_SIM,
	AT_C_GET_CID,
	AT_C_GET_CSQ,
	AT_C_GET_GSM,
	AT_C_INTERNAL_PROTOCOL_STACK_ENABLE,
	AT_C_SET_APN,
	AT_C_SET_AUTH_TYPE,
	AT_C_SET_AUTH,
	AT_C_GET_CONN_PM,
	AT_C_GET_GPRS_STATE,
	AT_C_GPRS_ATTCH,
	AT_C_CNCT_PPP,
	AT_C_PPP_CLOSE,
	AT_C_GET_PPP_ST,
	AT_C_TCP_SETUP,
	AT_C_TCP_CONNECT,

	AT_C_TCP_STATE,

	//AT_C_TCP_RCVD,

	AT_C_TCP_RCVD_ACCESS,

	AT_C_TCP_CLOSE,

	AT_C_TCP_SEND,

	AT_C_TCP_SEND_DAT,
	AT_C_CCLK,
	AT_C_TIME_UPDATE,
};

UINT8 *AT_GetRcvBuffer(void);

int AT_CmdSend(int fd, UINT8 Cmd, void *Arg);

#include <stdbool.h>
int AT_Receive(int fd, UINT8 *Rec, UINT16 RecMax, int timeout, bool IsCdma);
//-----------------------------------------------------------------------------------------
#endif /* AT_H_ */


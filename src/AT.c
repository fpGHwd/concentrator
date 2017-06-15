
#include "AT.h"
#include "string.h"
#include "stdio.h"
#include "common.h"
#include "serial.h"
#include "main.h"
//------------------------------------------------------------------------------------------------
#define AT_TIMEOUT (2000UL)
//------------------------------------------------------------------------------------------------
static AT_t tAT;
//------------------------------------------------------------------------------------------------
int AT_EXPECT_OK(const UINT8 *Rsp)
{
	if (Rsp == NULL) {
		return (-1);
	}
	if (strstr((char *) Rsp, "OK") != NULL) {
		return (0);
	}
	return (-1);
}

int AT_EXPECT_RUIM(const UINT8 *Rsp) /// check for UIM string  // +CIND:RUIM 表示UIM初始化完成
{
	if (Rsp == NULL) {
		return (-1);
	}
	if (strstr((char *) Rsp, "+CIND:RUIM") != NULL) {
		return (0);
	}
	return (-1);
}

int AT_EXPECT_READY(const UINT8 *Rsp) {
	if (Rsp == NULL) {
		return (-1);
	}

	if (strstr((char *) Rsp, "READY") != NULL) {
		return (0);
	}
	return (-1);
}

int AT_EXPECT_CSQ(const UINT8 *Rsp) /// 查询信号强度指令
{

	if (Rsp == NULL) {

		return (-1);
	}

	if (strstr((char *) Rsp, "+CSQ: 99,99") != NULL) {

		return (-1);
	}

	return (0);
}

int AT_EXPECT_CREG(const UINT8 *Rsp) /// 查询网络注册指令
{
	if (Rsp == NULL) {
		return (-1);
	}
	if ((strstr((char *) Rsp, "+CREG: 0,1") != NULL)
			|| (strstr((char *) Rsp, "+CREG: 0,5") != NULL)) {
		return (0);
	}
	return (-1);
}

int AT_EXPECT_CGATT(const UINT8 *Rsp) /// 主动进行 GPRS 附着
{

	if (Rsp == NULL) {
		return (-1);
	}

	if (strstr((char *) Rsp, "+CGATT: 1") != NULL) {

		return (0);
	}

	return (-1);
}

int AT_EXPECT_XIIC(const UINT8 *Rsp) {

	if (Rsp == NULL) {

		return (-1);
	}

	if (strstr((char *) Rsp, "+XIIC:    1") != NULL) { /// GPRS附着成功

		return (0);
	}

	return (-1);
}

int AT_EXPECT_CONNECT(const UINT8 *Rsp) {
	if (Rsp == NULL) {
		return (-1);
	}
	if (strstr((char *) Rsp, "2000") != NULL) { /// AT$MYNETOPEN = 0,2000
		return (0);
	}
	return (-1);
}

int AT_EXPECT_TCP_SETUP(const UINT8 *Rsp) ////
{
	if (Rsp == NULL) {
		return (-1);
	}

	if (strstr((char *) Rsp, "+TCPSETUP:0,OK") != NULL) {

		return (0);
	}

	if (strstr((char *) Rsp, "+TCPSETUP:1,OK") != NULL) {

		return (0);
	}

	if (strstr((char *) Rsp, "+TCPSETUP:2,OK") != NULL) {

		return (0);
	}

	if (strstr((char *) Rsp, "+TCPSETUP:3,OK") != NULL) {

		return (0);
	}

	return (-1);
}

int AT_EXPECT_TCP_SEND(const UINT8 *Rsp) /// 发送数据指令
{

	if (Rsp == NULL) {

		return (-1);
	}

	if (strstr((char *) Rsp, "$MYNETWRITE:") != NULL) {

		return (0);
	}
	//---------------------------------------------------------------------
	return (-1);
}

int AT_EXPECT_SEND_DAT(const UINT8 *Rsp) /// send data if OK
{
	if (Rsp == NULL) {
		return (-1);
	} else {
		if (strstr((char *) Rsp, "OK") != NULL) {
			return (0);
		} else {
			return (-1);
		}
	}
}

int AT_EXPECT_SET_CLOCK(const UINT8 *Rsp) {
	if (Rsp == NULL) {
		PRINTF("NULL for Rsp\n");
	}
	if (strstr((char *) Rsp, "OK")) { // read time
		return 0;
	}

	return -1;

}

int AT_EXPECT_CHECK_CCLK(const UINT8 *Rsp) {
	if (Rsp == NULL) {
		PRINTF("NULL for Rsp\n");
	}

	if (strstr((char *) Rsp, "+CCLK:")) { // read time
		return 0;
	}

	return -1;
}

int AT_EXPECT_MYTIMEUPDATE(const UINT8 *Rsp)
{
	if(Rsp == NULL)
		PRINTF("NULL for Rsp\n");
	if(strstr((char*)Rsp, "$MYTIMEUPDATE:"))
		return 0;
	else
		return -1;
}


int AT_EXPECT_SET_MYNET_ACT(const UINT8 *Rsp)
{
	if(Rsp == NULL)
		PRINTF("NULL for Rsp\n");
	if(strstr((char*)Rsp, "OK")){
		return 0;
	}else if(strstr((char*)Rsp, "902")){
		PRINTF("react the net, noting wrong\n");
		return 0;
	}else{
		return -1;
	}
}

int AT_EXPECT_MYTIME_UPDATE(const UINT8 *Rsp)
{
	if(Rsp == NULL)
		PRINTF("NULL for Rsp\n");
	if(strstr((char*)Rsp, "AT$MYTIMEUPDATE=\"time.nist.gov\"") || strstr((char*)Rsp, "OK"))
		return 0;
	else
		return -1;
}

int AT_EXPECT_CLOSE_UPDATE_TIME_SOCKET(const UINT8 *Rsp)
{
	if(Rsp == NULL)
		PRINTF("NULL for Rsp\n");
	if(strstr((char*)Rsp, "AT$MYTIMEUPDATE=\"time.nist.gov\""))
		return 0;
	else
		return -1;
}

int AT_CmdSetup(AT_t *AT, UINT8 Cmd, void *Arg) {
	int Ret = 0;

	UINT8 Format = DAT_HEX;
	UINT8 ConnectType = TCP_CLIENT;
	ApnArg_t *APN = NULL;
	AuthArg_t *AUTH = NULL;
	TcpSendArg_t *Send = NULL;
	TcpSetupArg_t *TCP = NULL;
	DNArg_t *dn_send = NULL;
	//-------------------------------------------------------------------
	switch (Cmd) {

	case AT_C_AT:
		AT->TryMax = 3;
		AT->RcvMax = 1;
		AT->TmoMax = 2000;
		AT->Expect = AT_EXPECT_OK;
		snprintf((char *) AT->SndBuffer, AT_C_BUFSIZ, "AT\r");
		break;

	case AT_C_WAIT_RDY:
		AT->TryMax = 1;
		AT->RcvMax = 8;
		AT->TmoMax = 2000;
		AT->Expect = AT_EXPECT_RUIM;

		snprintf((char *) AT->SndBuffer, AT_C_BUFSIZ, "AT\r");
		break;

	case AT_C_GET_TYPE:
		AT->TryMax = 2;
		AT->RcvMax = 1;
		AT->TmoMax = 2000;
		AT->Expect = AT_EXPECT_OK;

		snprintf((char *) AT->SndBuffer, AT_C_BUFSIZ, "AT$MYTYPE?\r");
		break;

	case AT_C_GET_VER:
		AT->TryMax = 2;
		AT->RcvMax = 1;
		AT->TmoMax = 2000;
		AT->Expect = AT_EXPECT_OK;

		snprintf((char *) AT->SndBuffer, AT_C_BUFSIZ, "AT$MYGMR\r");
		break;

	case AT_C_GET_SIM:
		AT->TryMax = 2;
		AT->RcvMax = 1;
		AT->TmoMax = 2000;
		AT->Expect = AT_EXPECT_READY;

		snprintf((char *) AT->SndBuffer, AT_C_BUFSIZ, "AT+CPIN?\r");
		break;

	case AT_C_GET_CID:
		AT->TryMax = 2;
		AT->RcvMax = 1;
		AT->TmoMax = 2000;
		AT->Expect = AT_EXPECT_OK;

		snprintf((char *) AT->SndBuffer, AT_C_BUFSIZ, "AT$MYCCID\r");
		break;

	case AT_C_GET_CSQ:
		AT->TryMax = 3;
		AT->RcvMax = 1;
		AT->TmoMax = 5000;
		AT->Expect = AT_EXPECT_CSQ;

		snprintf((char *) AT->SndBuffer, AT_C_BUFSIZ, "AT+CSQ\r");
		break;

	case AT_C_GET_GSM:
		AT->TryMax = 5;
		AT->RcvMax = 2;
		AT->TmoMax = 5000;
		AT->Expect = AT_EXPECT_CREG;

		snprintf((char *) AT->SndBuffer, AT_C_BUFSIZ, "AT+CREG?\r");
		break;

	case AT_C_INTERNAL_PROTOCOL_STACK_ENABLE: ///  开启主动上报，这条指令请放在 AT$MYNETACT 前面
		AT->TryMax = 2;
		AT->RcvMax = 1;
		AT->TmoMax = 2000;
		AT->Expect = AT_EXPECT_OK;
		snprintf((char *) AT->SndBuffer, AT_C_BUFSIZ, "AT$MYNETURC=1\r");
		break;

	case AT_C_SET_APN: /// operations before TCP connecting, set TCP arguments.

		APN = (ApnArg_t *) Arg;
		AT->TryMax = 2;
		AT->RcvMax = 6;
		AT->TmoMax = 2000;
		AT->Expect = AT_EXPECT_OK;

		snprintf((char *) AT->SndBuffer, AT_C_BUFSIZ,
				"AT$MYNETCON=0,\"APN\",\"%s\"\r", APN->APN); /// channel = 0, APN ~ string type;	
		break;

	case AT_C_SET_AUTH_TYPE:
		AT->TryMax = 2;
		AT->RcvMax = 6;
		AT->TmoMax = 2000;
		AT->Expect = AT_EXPECT_OK;
		snprintf((char *) AT->SndBuffer, AT_C_BUFSIZ,
				"AT$MYNETCON=0,\"AUTH\",%d\r", (*(UINT8 *) Arg));
		break;

	case AT_C_SET_AUTH:
		AUTH = (AuthArg_t *) Arg;
		AT->TryMax = 2;
		AT->RcvMax = 6;
		AT->TmoMax = 2000;
		AT->Expect = AT_EXPECT_OK;

		snprintf((char *) AT->SndBuffer, AT_C_BUFSIZ,
				"AT$MYNETCON=0,\"USERPWD\",\"%s,%s\"\r", AUTH->Name, AUTH->PSW);
		break;

	case AT_C_GET_CONN_PM:
		AT->TryMax = 2;
		AT->RcvMax = 2;
		AT->TmoMax = 2000;
		AT->Expect = AT_EXPECT_OK;

		snprintf((char *) AT->SndBuffer, AT_C_BUFSIZ, "AT$MYNETCON?\r");
		break;

	case AT_C_GET_GPRS_STATE:
		AT->TryMax = 16;
		AT->RcvMax = 1;
		AT->TmoMax = 2000;
		AT->Expect = AT_EXPECT_CGATT;

		snprintf((char *) AT->SndBuffer, AT_C_BUFSIZ, "AT+CGATT?\r");
		break;

	case AT_C_GPRS_ATTCH:

		snprintf((char *) AT->SndBuffer, AT_C_BUFSIZ, "AT+CGATT=1\r");
		break;

	case AT_C_CNCT_PPP: ///
		AT->TryMax = 2;
		AT->RcvMax = 8;
		AT->TmoMax = 2000;
		AT->Expect = AT_EXPECT_OK;
		snprintf((char *) AT->SndBuffer, AT_C_BUFSIZ, "AT$MYNETACT=0,1\r");
		break;

	case AT_C_GET_PPP_ST: /// 网络激活查询指令
		AT->TryMax = 16;
		AT->RcvMax = 1;
		AT->TmoMax = 2000;
		AT->Expect = AT_EXPECT_OK;
		snprintf((char *) AT->SndBuffer, AT_C_BUFSIZ, "AT$MYNETACT?\r");
		break;

	case AT_C_PPP_CLOSE:
		AT->TryMax = 1;
		AT->RcvMax = 3;
		AT->TmoMax = 2000;
		snprintf((char *) AT->SndBuffer, AT_C_BUFSIZ, "AT+XIIC=0\r");
		break;

	case AT_C_TCP_SETUP: ///   TCP 链接服务参数设置指令

		TCP = (TcpSetupArg_t *) Arg;

		AT->TryMax = 2;
		AT->RcvMax = 8;
		AT->TmoMax = 2000;
		AT->Expect = AT_EXPECT_OK;

		snprintf((char *) AT->SndBuffer, AT_C_BUFSIZ,
				"AT$MYNETSRV=%d,%d,%d,%d,\"%s:%s\"\r", TCP->Link, TCP->Link,
				Format, ConnectType, TCP->IP, TCP->Port);
		break;

	case AT_C_TCP_CONNECT:
		AT->TryMax = 1;
		AT->RcvMax = 8;
		AT->TmoMax = 2000;
		AT->Expect = AT_EXPECT_CONNECT;

		snprintf((char *) AT->SndBuffer, AT_C_BUFSIZ, "AT$MYNETOPEN=%d\r",
				(*(UINT8 *) Arg));
		break;

	case AT_C_TCP_STATE:
		AT->TryMax = 1;
		AT->RcvMax = 3;
		AT->TmoMax = 2000;

		snprintf((char *) AT->SndBuffer, AT_C_BUFSIZ, "AT+IPSTATUS=0\r");
		break;

	case AT_C_TCP_CLOSE:
		AT->TryMax = 1;
		AT->RcvMax = 1;
		AT->TmoMax = 2000;
		AT->Expect = AT_EXPECT_OK;

		snprintf((char *) AT->SndBuffer, AT_C_BUFSIZ, "AT$MYNETCLOSE=%d\r",
				(*(UINT8 *) Arg));
		break;

	case AT_C_TCP_RCVD_ACCESS:

		break;

	case AT_C_TCP_SEND:

		Send = (TcpSendArg_t *) Arg;
		AT->TryMax = 1;
		AT->RcvMax = 16;
		AT->TmoMax = 200;
		AT->Expect = AT_EXPECT_TCP_SEND;
		snprintf((char *) AT->SndBuffer, AT_C_BUFSIZ, "AT$MYNETWRITE=%d,%d\r",
				Send->Link, Send->Length);
		break;

	case AT_C_TCP_SEND_DAT:
		Send = (TcpSendArg_t *) Arg;
		AT->TryMax = 1;
		AT->RcvMax = 50;
		AT->TmoMax = 200;
		AT->Length = Send->Length;
		AT->ptrBuffer = Send->Buffer; /// other arguments
		AT->Expect = AT_EXPECT_OK;
		break;

	case AT_C_CCLK:
		AT->TryMax = 1; /// try times
		AT->RcvMax = 50; // receive times
		AT->TmoMax = 200; // time
		AT->Expect = AT_EXPECT_CHECK_CCLK;
		snprintf((char *) (AT->SndBuffer), AT_C_BUFSIZ, "AT+CCLK?\r");
		break;
	case AT_C_TIME_UPDATE:
		AT->TryMax = 1;
		AT->RcvMax = 50;
		AT->TmoMax = 200;
		if (Arg == NULL) {
			PRINTF("Arg == null");
			return -1;
		}
		dn_send = (DNArg_t*) Arg;
		AT->Expect = AT_EXPECT_OK;

		snprintf((char *) (AT->SndBuffer), AT_C_BUFSIZ, "AT$MYTIMEUPDATE=%s\r",
				dn_send->domain_p);
		break;
	case AT_CHECK_TEMPRETURE:
		AT->TryMax = 1;
		AT->RcvMax = 50;
		AT->TmoMax = 200;
		break;
	case AT_TIME_UPDATE_CHECK:
		AT->TryMax = 1;
		AT->RcvMax = 50;
		AT->TmoMax = 200;
		AT->Expect = AT_EXPECT_MYTIMEUPDATE;
		snprintf((char *) AT->SndBuffer, AT_C_BUFSIZ, "AT$MYTIMEUPDATE?\r");
		break;
	case AT_NET_URC:
		AT->TryMax = 1;
		AT->RcvMax = 50;
		AT->TmoMax = 200;
		AT->Expect = AT_EXPECT_OK;
		snprintf((char *) AT->SndBuffer, AT_C_BUFSIZ, "AT$MYNETURC=1\r");
		break;
	case AT_MYTIME_UPDATE:
		AT->TryMax = 1;
		AT->RcvMax = 50;
		AT->TmoMax = 3000;
		AT->Expect = AT_EXPECT_MYTIME_UPDATE;
		snprintf((char *) AT->SndBuffer, AT_C_BUFSIZ, "AT$MYTIMEUPDATE=\"time.nist.gov\"\r");
		break;
	case AT_C_SET_AUTH_UPDATE_TIME:
		AT->TryMax = 1;
		AT->RcvMax = 50;
		AT->TmoMax = 200;
		AT->Expect = AT_EXPECT_OK;
		snprintf((char *) AT->SndBuffer, AT_C_BUFSIZ,
				"AT$MYNETCON=1,APN,CMNET\r");
		break;
	case AT_C_SET_MYNET_ACT:
		AT->TryMax = 1;
		AT->RcvMax = 50;
		AT->TmoMax = 200;
		AT->Expect = AT_EXPECT_SET_MYNET_ACT;
		snprintf((char *) AT->SndBuffer, AT_C_BUFSIZ, "AT$MYNETACT=1,1\r");
		break;
	case AT_C_CLOSE_UPDATE_TIME_SOCKET:
		AT->TryMax = 1;
		AT->RcvMax = 50;
		AT->TmoMax = 200;
		AT->Expect = AT_EXPECT_OK;
		snprintf((char *) AT->SndBuffer, AT_C_BUFSIZ, "AT$MYNETACT=1,0\r");
		break;
	default:
		Ret = -1;
		break;
	}

	return (Ret);
}

//void notify_watchdog(void);
int AT_CmdSend(int fd, UINT8 Cmd, void *Arg) /// Arg
{
	UINT8 i = 0;
	UINT8 k = 0;
	AT_t *AT = &tAT;
	//----------------------------------------------------------------------------
	memset((void *) AT, 0, sizeof(AT_t));

	if (AT_CmdSetup(AT, Cmd, Arg) < 0) {
		return (-1);
	}

	if (AT->TryMax == 0)
		AT->TryMax++;

	do {
		memset(AT->RcvBuffer, 0, AT_C_BUFSIZ);
		if ((AT->ptrBuffer != NULL) && (AT->Length > 0))
			write_serial(fd, AT->ptrBuffer, AT->Length, AT_TIMEOUT);
		else {
			PRINTF("%s: Modem Send: %s\n", __FUNCTION__, AT->SndBuffer);
			write_serial(fd, AT->SndBuffer,
					strlen((const char *) AT->SndBuffer), AT_TIMEOUT);
		}
		///notify_watchdog();
		if ((g_terminated > 0) || (wait_for_ready(fd, AT_TIMEOUT, 0) <= 0)) {
			return (0);
		}
		AT->RcvCnt = 0;
		for (i = 0; i < AT->RcvMax; i++) {
			k = read_serial(fd, AT->RcvBuffer, AT_C_BUFSIZ, AT->TmoMax);
			if (k > 0) {
				PRINTF("%s: Modem Receive: %s\n", __FUNCTION__, AT->RcvBuffer);
				if ((AT->Expect != NULL)
						&& (AT->Expect((const UINT8 *) AT->RcvBuffer) == 0)) {
					//PRINTF(" OK\n");
					return (0);
				} else {
					PRINTF("%s: UNEXPECTED \n", __FUNCTION__);
				}
			}
		}
	} while (--AT->TryMax);
	return (-1);
}

int AT_IsReceive(int fd, int timeout)
{
	AT_t *AT = &tAT;

	int uwLen = 0;
	int uwTmoCnt = (timeout / 200);

	if (uwTmoCnt == 0)
		uwTmoCnt++;

	memset(AT->RcvBuffer, 0, AT_C_BUFSIZ);

	do {
		uwLen = read_serial(fd, AT->RcvBuffer, AT_C_BUFSIZ, 200);

		if (uwLen > 0) {
			PRINTF("%s Receive String: %s\n", __FUNCTION__, AT->RcvBuffer);
		}

		if (strstr((const char *)(AT->RcvBuffer), "$MYURCCLOSE:") != NULL) {
			PRINTF("CONNECTION INTERRUPTED in module\n");
			return (-1);
		}

		if (strstr((const char *)(AT->RcvBuffer), "$MYURCREAD:") != NULL) {
			PRINTF("RECEIVE DATA from module\n");
			return (1);
		}

	} while (--uwTmoCnt);
	//-------------------------------------------------------------------------------
	return (0);
}

UINT8 NeowayNumberBits(UINT16 Number);
UINT32 NeowayStringToNumber(const char *String);
int AT_Receive(int fd, UINT8 *Rec, UINT16 RecMax, int timeout, bool IsCdma) {
	int iRet = -1;
	char *s = NULL;
	const char *RcvStr = "$MYNETREAD: ";
	const char *GprsStr = "AT$MYNETREAD=0,2048\r"; /// read max data len with GPRS module, 0-2047
	const char *CdmaStr = "AT$MYNETREAD=0,1460\r"; /// read max data len with CDMA module, 0-1045
	//----------------------------------------------------------------------------

	if (fd < 0){
		fprintf(stdout, "fatal error: neoway string receive fd is invalid\n");
		exit(-1);
	}

	iRet = AT_IsReceive(fd, timeout);

	if (iRet <= 0) {
		PRINTF("%s: iRet RECEIVE LENGTH error", __FUNCTION__);
		return (iRet);
	}

	memset(Rec, 0, RecMax);

	if (IsCdma == TRUE) {
		write_serial(fd, CdmaStr, strlen(CdmaStr), AT_TIMEOUT);
	} else {
		write_serial(fd, GprsStr, strlen(GprsStr), AT_TIMEOUT);
	}

	msleep(300);

	if ((g_terminated > 0) || (wait_for_ready(fd, AT_TIMEOUT, 0) <= 0)) {
		return (0);
	}

	iRet = read_serial(fd, Rec, RecMax, AT_TIMEOUT);

	if (iRet > 0) {

		iRet = 0;

		s = strstr((char *) Rec, RcvStr);

		if (s != NULL) {

			s += strlen(RcvStr);

			s++;

			if (*s == ',') {
				s++;
				iRet = NeowayStringToNumber(s);
				s += NeowayNumberBits(iRet);
				s++;
				s++;
				memcpy(Rec, s, iRet);
			}
		}
	}
	return (iRet);
}

UINT8 *AT_GetRcvBuffer(void) {
	return ((UINT8 *) tAT.RcvBuffer);
}

/*
 * cm180.c
 *
 *  Created on: 2015-8-11
 *      Author: Johnnyzhang
 */
#include <AT.h>
#include "common.h"
#include "cm180.h"
#include "f_param.h"
#include "serial.h"


#define NEOWAY_IP_MAXSIZ 64
#define NEOWAY_READ_TIMEOUT (10 * 1000)
#define NEOWAY_WRITE_TIMEOUT (2 * 1000)
#define IsDigit(c)   ( ((c) >= '0') && ((c) <= '9') )
#define GPRS_MODULE_CONNECT 0 // to identify the connect used in gprs module

enum e_neoway_module_type {
	e_module_type_m590 = 0, e_module_type_cm180, e_module_type_unknown
};

void *g_cm180_resource = NULL;
static UINT8 g_ucNeowayModuleType = e_module_type_m590;
static char ucNeoIpStr[NEOWAY_IP_MAXSIZ] = { 0 };


int NeowayGprsSetApn(int fd, UINT8 PDN, const char *Type, const char *APN) {

	ApnArg_t ARG = { 0 };

	ARG.PDN = PDN;

	memcpy(ARG.APN, APN, strlen(APN));

	memcpy(ARG.Type, Type, strlen(Type));
	//----------------------------------------------------------------------------
	return (AT_CmdSend(fd, AT_C_SET_APN, (void *) &ARG));
}

int NeowayGprsSetAuth(int fd, UINT8 CID, UINT8 AuthType, const char *Name,
		const char *PSW) {

	AuthArg_t ARG = { 0 };
	//----------------------------------------------------------------------------
	ARG.CID = CID;

	ARG.AuthType = AuthType;

	memcpy(ARG.PSW, PSW, strlen(PSW));

	memcpy(ARG.Name, Name, strlen(Name));
	//----------------------------------------------------------------------------
	return (AT_CmdSend(fd, AT_C_SET_AUTH, (void *) &ARG));
}

int NeowayGprsGetVersion(int fd) {
	if (AT_CmdSend(fd, AT_C_GET_VER, NULL) < 0) {

		PRINTF("%s : Neoway GPRS/CDMA Get Version Abort!\n", __FUNCTION__);

		return (-1);
	}

	if (strstr((const char*) AT_GetRcvBuffer(), "CM180") != NULL) { /// cm180

		g_ucNeowayModuleType = e_module_type_cm180;

		PRINTF("%s : Neoway GPRS/CDMA Type : CM180!\n", __FUNCTION__);
	} else if (strstr((const char*) AT_GetRcvBuffer(), "M590 R2") != NULL) {

		g_ucNeowayModuleType = e_module_type_m590;

		PRINTF("%s : Neoway GPRS/CDMA Type : M590 R2!\n", __FUNCTION__);
	} else {

		g_ucNeowayModuleType = e_module_type_unknown;

		PRINTF("%s : Neoway GPRS/CDMA Type Unknown!\n", __FUNCTION__);

		return (-1);
	}

	return (0);
}

e_remote_module_status cm180_init(int fd) {
	UINT8 AuthType = AUTH_NONE;
	char apn_id[32], apn_user_id[32], apn_user_password[32];
	//------------------------------------------------------------------------
	fparam_get_value(FPARAMID_APN_ID, apn_id, sizeof(apn_id));
	fparam_get_value(FPARAMID_APN_USER_ID, apn_user_id, sizeof(apn_user_id));
	fparam_get_value(FPARAMID_APN_USER_PASSWD, apn_user_password,
			sizeof(apn_user_password));

	if (fd < 0) {
		return (e_modem_st_deivce_abort); /// rarely
	}

	PRINTF("%s : Neoway GPRS/CDMA Init!\n", __FUNCTION__);

	if (AT_CmdSend(fd, AT_C_GET_TYPE, NULL) < 0) { /// GPRS or CDMA type
		PRINTF("%s : Neoway GPRS/CDMA Get Type Abort!\n", __FUNCTION__);
		return (e_modem_st_deivce_abort);
	}

	if (NeowayGprsGetVersion(fd) < 0) { /// module information

		PRINTF("%s : Neoway GPRS/CDMA Get Version Abort!\n", __FUNCTION__);

		return (e_modem_st_deivce_abort);
	}

	if (AT_CmdSend(fd, AT_C_GET_SIM, NULL) < 0) { /// SIM status

		PRINTF("%s : Neoway GPRS/CDMA Get Sim Card Abort!\n", __FUNCTION__);

		return (e_modem_st_deivce_abort);
	}

	if (AT_CmdSend(fd, AT_C_GET_CID, NULL) < 0) { /// sim card number

		PRINTF("%s : Neoway GPRS/CDMA Get CCID Abort!\n", __FUNCTION__);

		return (e_modem_st_deivce_abort);
	}

	if (AT_CmdSend(fd, AT_C_GET_CSQ, NULL) < 0) { /// signal intensity

		PRINTF("%s : Neoway GPRS/CDMA Get CSQ Abort!\n", __FUNCTION__);

		return (e_modem_st_deivce_abort);
	}

	if (AT_CmdSend(fd, AT_C_GET_GSM, NULL) < 0) { /// network registeration

		PRINTF("%s : Neoway GPRS/CDMA Get Gsm State Abort!\n", __FUNCTION__);

		return (e_modem_st_deivce_abort);
	}

	if (NeowayGprsSetApn(fd, 1, "IP", apn_id) < 0) { /// 

		PRINTF("%s : Neoway GPRS/CDMA Set Apn Abort!\n", __FUNCTION__);

		return (e_modem_st_deivce_abort);
	}

	if (NeowayGprsSetAuth(fd, 1, AUTH_PAP, apn_user_id, apn_user_password)
			< 0) { /// AUTH_PAP ?
		PRINTF("%s Neoway GPRS/CDMA Set Auth Abort!\n", __FUNCTION__);
		return (e_modem_st_deivce_abort);
	}

	AuthType = AUTH_PAP; ///  AUTH：鉴权类型， 0： NONE； 1： PAP； 2： CHAP，默认为 1

	if (AT_CmdSend(fd, AT_C_SET_AUTH_TYPE, &AuthType) < 0) {

		PRINTF("%s : Neoway GPRS/CDMA Set Auth Type Abort!\n", __FUNCTION__);

		return (e_modem_st_deivce_abort);
	}

	PRINTF("%s : Neoway GPRS/CDMA Init Finish!\n", __FUNCTION__);
	//----------------------------------------------------------------------------
	return (e_modem_st_normal);
}

int cm180_ppp_connect(const char *device_name, const char *lock_name,
		const char *baudstr) {
	int fd = -1;

	UINT8 *Rsp = NULL;

	UINT8 *End = NULL;

	UINT8 *Start = NULL;

	bool bActCmdSend = FALSE;
	//-----------------------------------------------------------------------------------
	if ((fd = open_modem_device(device_name, lock_name, MODEM_DEFAULT_BAUD))
			< 0) { /// ? force to callback it's not secure

		return (-1);
	}
	if (AT_CmdSend(fd, AT_C_INTERNAL_PROTOCOL_STACK_ENABLE, NULL) < 0) {
		PRINTF("%s : Neoway GPRS/CDMA Internal Protocol Stack Enable Abort!\n",
				__FUNCTION__);
		close_serial(fd); /// failed and close fd
		return (-1);
	}

	CHECK_CONNECT_IP:
	memset(ucNeoIpStr, 0, NEOWAY_IP_MAXSIZ);
	if (AT_CmdSend(fd, AT_C_GET_PPP_ST, NULL) < 0) {
		PRINTF("%s : Neoway GPRS/CDMA Get PPP State Abort!\n", __FUNCTION__);
		////close_modem_device(lock_name);
		close_serial(fd);
		return (-1);
	}
	{
		//$MYNETACT: 0,1,"113.115.253.42"

		Rsp = AT_GetRcvBuffer();

		Start = (UINT8*)strstr((char*)Rsp, "\"");

		if (Start != NULL) {

			Start++;

			End = (UINT8*)strstr((char*)Start, "\"");

			memcpy(ucNeoIpStr, Start, (End - Start)); /// getip AT-COMMAND
		}

		if (strstr((char *)Rsp, "0.0.0.0") != NULL) {

			if (bActCmdSend == FALSE) {

				if (AT_CmdSend(fd, AT_C_CNCT_PPP, NULL) < 0) {

					PRINTF("%s : Neoway GPRS/CDMA PPP Connect Abort!\n",
							__FUNCTION__);

					////close_modem_device(lock_name);
					close_serial(fd); /// cannot modify upper data structure easily

					return (-1);
				}

				bActCmdSend = TRUE;

				goto CHECK_CONNECT_IP;
			} else {

				PRINTF("%s : Neoway GPRS/CDMA IP Invalid!\n", __FUNCTION__);

				////close_modem_device(lock_name);
				close_serial(fd); /// cannot modify upper data structure easily

				return (-1);
			}

		}

	}

	msleep(500);

	PRINTF("%s : Neoway GPRS/CDMA IP = %s!\n", __FUNCTION__, ucNeoIpStr);
	//-------------------------------------------------------------------------------
	return (fd);
}

void GetStrPort(char *String, UINT16 Port) {
	UINT8 k = 0;
	UINT8 ucBit_0 = 0; /// 
	UINT8 ucBit_1 = 0;
	UINT8 ucBit_2 = 0;
	UINT8 ucBit_3 = 0;
	UINT8 ucBit_4 = 0;
	//----------------------------------------------------------------------------
	/*  12345 */
	ucBit_0 = (UINT8) (Port % 10);
	Port /= 10; //1234, 5
	ucBit_1 = (UINT8) (Port % 10);
	Port /= 10; //123,  4
	ucBit_2 = (UINT8) (Port % 10);
	Port /= 10; //12,   3
	ucBit_3 = (UINT8) (Port % 10);
	Port /= 10; //1,    2
	ucBit_4 = (UINT8) Port;					 //0,    1

	///snprintf(String, 5, "%d", Port);

	if (ucBit_4 > 0) {
		String[k++] = ucBit_4 + '0';
		String[k++] = ucBit_3 + '0';
		String[k++] = ucBit_2 + '0';
		String[k++] = ucBit_1 + '0';
		String[k++] = ucBit_0 + '0';
	} else if (ucBit_3 > 0) {
		String[k++] = ucBit_3 + '0';
		String[k++] = ucBit_2 + '0';
		String[k++] = ucBit_1 + '0';
		String[k++] = ucBit_0 + '0';
	} else if (ucBit_2 > 0) {
		String[k++] = ucBit_2 + '0';
		String[k++] = ucBit_1 + '0';
		String[k++] = ucBit_0 + '0';
	} else if (ucBit_1 > 0) {
		String[k++] = ucBit_1 + '0';
		String[k++] = ucBit_0 + '0';
	} else {
		String[k++] = ucBit_0 + '0';
	}

	String[k] = '\0';
}

bool cm180_update_time_via_gprs(int fd, time_t *time);
int cm180_tcp_connect(int fd, const char *IP, int Port, int timeout) {
	//int Ret = 0;
	TcpSetupArg_t ARG = { 0 };
	//------------------------------------------------------------------------------
	ARG.Link = 0;

	memcpy(ARG.IP, IP, strlen(IP));  /// get IP

	GetStrPort((char *) ARG.Port, Port);
	//-------------------------------------------------------------------------------------------------

	PRINTF("%s : Tcp Setup : IP = %s, Port = %s!\n", __FUNCTION__, ARG.IP,
			ARG.Port); /// print arg ip port

	if (AT_CmdSend(fd, AT_C_TCP_SETUP, (void *) &ARG) < 0) {
		PRINTF("%s : Neoway GPRS/CDMA Tcp Setup Err!\n", __FUNCTION__);
		return (-1);
	}

	if (AT_CmdSend(fd, AT_C_TCP_CONNECT, (void *) &ARG.Link) < 0) {
		if(AT_CmdSend(fd, AT_C_TCP_CLOSE, (void *) &ARG.Link)<0){ // close connection OK
			PRINTF("%s : Neoway GPRS/CDMA failed to close connection!\n", __FUNCTION__);
			return (-1);
		}else{
			PRINTF("%s : Neoway GPRS/CDMA Close TCP CONNECTION!\n", __FUNCTION__);
			if(AT_CmdSend(fd, AT_C_TCP_CONNECT, (void *) &ARG.Link) < 0){
				PRINTF("%s : Neoway GPRS/CDMA Tcp Connect Err!\n", __FUNCTION__);
				return -1;
			}
		}
	}else{
		//return fd;
		//PRINTF("%s : Neoway GPRS/CDMA Tcp Connect OK!\n", __FUNCTION__); // no need to show log
	}
	//------------------------------------------------------------------------------

	/*
	if(fd > 0){
		///PRINTF("update the time test\n");
		if(cm180_update_time_via_gprs(fd,NULL))
			PRINTF("Update time by gprs successfuly\n");
		else
			PRINTF("Update time failed, go on\n");
	}else{
		PRINTF("fatal error: fd is invalid\n");
		exit(1);
	}
	*/

	return (fd);
}

int cm180_udp_connect(int fd, const char *addr, int port) {

	PRINTF("%s : Neoway GPRS/CDMA UDP UnSupport!\n", __FUNCTION__);

	return (0);
}

int cm180_send(int fd, const BYTE *buffer, int length, int *response) {
	/*
	 // before send to check time 
	 if(!time_update){
	 if(cm180_update_time_via_network(fd, time_server_string)){
	 time_update = true;
	 }else{
	 PRINTF("!! update time failed, and still set time_update = true\n");
	 time_update = true;
	 }
	 time_update = true;
	 }else{
	 // do nothing
	 }
	 */
	TcpSendArg_t Arg = { 0 };
	//----------------------------------------------------------------------------
	Arg.Link = 0;

	Arg.Length = length;

	Arg.Buffer = (UINT8 *) buffer;
	//----------------------------------------------------------------------------
	if (AT_CmdSend(fd, AT_C_TCP_SEND, (void *) &Arg) < 0) {

		*response = REMOTE_MODULE_RW_ABORT; /// read write abort

		PRINTF("%s : Neoway GPRS/CDMA Send Setup Err!\n", __FUNCTION__);
	}

	if (AT_CmdSend(fd, AT_C_TCP_SEND_DAT, (void *) &Arg) < 0) {

		*response = REMOTE_MODULE_RW_ABORT;

		PRINTF("%s : Neoway GPRS/CDMA Send Data Err!\n", __FUNCTION__);
	}

	msleep(2000);

	*response = REMOTE_MODULE_RW_NORMAL;
	//-----------------------------------------------------------------------------
	return (length);
}

UINT8 NeowayNumberBits(UINT16 Number) {
	UINT8 ucBits = 0;
	//----------------------------------------------------------------------------
	if (Number < 9)
		ucBits = 1;
	else if (Number < 99)
		ucBits = 2;
	else if (Number < 999)
		ucBits = 3;
	else if (Number < 9999)
		ucBits = 4;
	else
		ucBits = 5;
	//----------------------------------------------------------------------------
	return (ucBits);
}

UINT32 NeowayStringToNumber(const char *String) {

	UINT32 uwResult = 0;

	const char *s = String;
	//----------------------------------------------------------------------------
	if (s == NULL) {

		return (0);
	}

	while ((*s != '\0') && (IsDigit(*s))) {

		uwResult = ((uwResult * 10) + (*s - '0'));

		s++;
	}
	//----------------------------------------------------------------------------
	return (uwResult);
}

int cm180_receive(int fd, BYTE *Rec, int RecMax, int timeout, int *Response) {
	int uwRet = 0;
	//-----------------------------------------------------------------------------
	*Response = REMOTE_MODULE_RW_NORMAL;

	if (fd < 0) {

		*Response = (-1); /// REMOTE_MODULE_RW_UNORMAL

		PRINTF("%s : Neoway GPRS/CDMA fd Err!\n", __FUNCTION__);

		return (-1);
	}

	if (g_ucNeowayModuleType == e_module_type_m590) { /// gprs /// m590 not cdma

		uwRet = AT_Receive(fd, Rec, RecMax, timeout, FALSE);
		if (uwRet < 0) {
			PRINTF("%s: RECEIVE ERROR\n", __FUNCTION__);
		}
	} else { // cdma

		uwRet = AT_Receive(fd, Rec, RecMax, timeout, TRUE);
		if (uwRet < 0) {
			PRINTF("%s: RECEIVE ERROR\n", __FUNCTION__);
		}
	}

	if (uwRet < 0) {
		PRINTF("%s : Neoway GPRS/CDMA Receive Err!\n", __FUNCTION__);
		*Response = (-1);
	}
	//----------------------------------------------------------------------------
	return (uwRet);
}

int cm180_shutdown(int fd) {
	UINT8 ucLink = 0;
	//-----------------------------------------------------------------------------
	if (AT_CmdSend(fd, AT_C_TCP_CLOSE, (void *) &ucLink) < 0) {

		return (FALSE);
	}

	msleep(500);
	//----------------------------------------------------------------------------
	return ( TRUE);
}

BOOL cm180_getip(int fd, char *ipstr) {

	if (strlen(ucNeoIpStr) > 0) {

		if (ipstr) {

			PRINTF(ucNeoIpStr);

			strcpy(ipstr, ucNeoIpStr);
		}

		return ( TRUE);
	} else {
		return ( FALSE);
	}
}

bool cm180_update_time_via_gprs(int fd, time_t *time) {

	UINT8 *rbp;
	//int delta_sec = 7;

	if (AT_CmdSend(fd, AT_C_CCLK, NULL) == 0) {
	} else {
		return false;
	}

	if (AT_CmdSend(fd, AT_C_SET_AUTH_UPDATE_TIME, NULL) == 0) {
	} else {
		return false;
	}

	if (AT_CmdSend(fd, AT_NET_URC, NULL) == 0) {
	} else {
		return false;
	}

	if (AT_CmdSend(fd, AT_C_SET_MYNET_ACT,NULL) ==0) { /// PDB activated
	} else {
		return false;
	}

	if (AT_CmdSend(fd, AT_MYTIME_UPDATE, NULL) == 0) {
		//ok
	} else {
		return false;
	}

	if (AT_CmdSend(fd, AT_C_CCLK, NULL) == 0) {
		rbp = AT_GetRcvBuffer();
		if(rbp == NULL){
			perror("Fatal error in getting receive buffer and pointer\n");
		}
		// TODO string to time and set the time
		//PRINTF("print the string get: %s\n", rbp);
		// print result: AT+CCLK?\rCCLK: "17/04/20,10;43:16"\rOK
	} else {
		return false;
	}

	/*
	if(AT_CmdSend(fd, AT_C_CNCT_PPP, NULL) == 0){
		PRINTF("exception: need to be paied attention to\n");
	}else{
		PRINTF("set back the mynetact to 0,1\n");
	}
	*/

	return true;
}

/*
 * MODEM:STARTUP
 *
 * +PBREADY
 *
 * AT
 * OK
 *
 * AT+GMR
 * +GMR: M590_1250_l9s63000_v023
 *
 * AT+MYGMR
 * NEO6
 * M590 R2
 * v023
 * 190515
 * V1.2
 * 240214
 * OK
 *
 * AT+CPIN?
 * +CPIN: READY
 * OK
 *
 * AT$MYCCID
 * $MYCCID: "898602b6111600119496"
 * OK
 *
 * AT+CSQ
 * +CSQ: 30,0
 * OK
 *
 * AT+CREG?
 * +CREG: 0,5
 * OK
 *
 * AT$MYNETURC=1
 * OK
 *
 * AT$MYNETACT=0,1
 * OK
 * $MYNETACT: 0,1,"10.32.187.2"
 *
 * AT$MYNETACT?
 * $MYNETACT: 0,1,"10.32.187.2"
 * OK
 *
 * AT$MYNETSRV=0,0,0,0,"120.25.147.230:12345"
 * OK
 *
 * AT$MYNETOPEN=0
 * $MYNETOPEN: 0,2000
 * OK
 *
 * AT$MYNETWRITE=0,10
 * $MYNETWRITE: 0,10
 * (0123456789)
 * OK
 *
 * AT+MYNETACK=0
 * $MYNETACK: 0,0,2047
 * OK
 *
 * $MYURCREAD: 0 (objective)
 *
 * AT$MYNETREAD=0,2048
 * $MYNETREAD: 0,33
 * (RECEIVED DATA)
 * OK
 *
 * AT$MYNETCLOSE=0
 * $MYNETCLOSE: 0
 * OK
 *
 * ******time update AT process******
 * AT+CCLK?
 * +CCLK: "04/01/01,00:16:35"
 * OK
 *
 * AT$MYNETCON=1,APN,CMNET
 * OK
 *
 * AT$MYNETURC=1 (OBJECTIVE )
 * OK
 *
 * AT$MYNETACT=1,1
 * ERROR: 902 (ALREADY ADTIVIATED)
 *
 * AT$MYNETACT=0,0
 * OK
 *
 * AT$MYNETACT=0,1
 * OK
 * $MYURCACT: 0,1,"10.32.187.2"
 *
 * AT$MYNETACT=1,1
 * ERROR: 902
 *
 * AT$MYTIMEUPDATE="time.nist.gov" /// 916
 * OK
 *
 * AT+CCLK?
 * +CCLK: "17/04/20,14:17:08"
 * OK
 */



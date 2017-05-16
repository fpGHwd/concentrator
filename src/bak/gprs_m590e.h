
#ifndef __GPRS_M590E_H__
#define __GPRS_M590E_H__

#include <stdbool.h>

#define AT_CMD_SGCC

enum GprsParityBits{
    GPRS_PBIT_N = 'N',
    GPRS_PBIT_n = 'n',
    GPRS_PBIT_O = 'O',
    GPRS_PBIT_o = 'o',
    GPRS_PBIT_E = 'E',
    GPRS_PBIT_e = 'e',
    GPRS_PBIT_S = 'S',
    GPRS_PBIT_s = 's',
};

typedef enum auth_type_e {
	AUTH_NONE = 0,
	AUTH_PAP = 1,
	AUTH_CHAP = 2,
}auth_type_t;

typedef enum tcp_channel_e{
	TCP_CHN_0 = 0,
	TCP_CHN_1,
	TCP_CHN_2,
	TCP_CHN_3,
	TCP_CHN_4,
	TCP_CHN_5,
}tcp_channel_t;

typedef enum tcp_socketId_e{
	TCP_SOCKID_0 = 0,
	TCP_SOCKID_1,
	TCP_SOCKID_2,
	TCP_SOCKID_3,
	TCP_SOCKID_4,
	TCP_SOCKID_5,
}tcp_socketId_t;

typedef enum tcp_net_type_e {
	TCP_Client = 0,
	TCP_Server = 1,
	UDP = 2,
}tcp_net_type_t;

typedef enum tcp_trans_viewMode_e {
	TCP_TRANS_HEX = 0, //(默认)
	TCP_TRANS_TEXT = 1,
}tcp_trans_viewMode_t;

struct gprs_vendor_s {
	char name[7];
	char module[8];
	char version[14];
};
typedef struct gprs_vendor_s gprs_vendor_t;

struct gprs_software_ver_s{
	unsigned char ver[24];
};
typedef struct gprs_software_ver_s  gprs_software_ver_t;

struct gprs_csq_s {
	int signal;
	int rssi;
	int ber;
};
typedef struct gprs_csq_s gprs_csq_t;

struct gprs_pdp_apn_s {
	unsigned int cid;
	char type[17];
	char apn[17];
};
typedef struct gprs_pdp_apn_s gprs_pdp_apn_t;

typedef struct _struct_auth {
  unsigned int cid;
  auth_type_t  auth;
  char  name[32];
  char  psw[16];
} auth_t;


struct gprs_tcp_ip_s {
	tcp_channel_t channel; // 0 - 5 通道
	tcp_socketId_t sockid;  // 0 - 5 通道
	tcp_net_type_t nettype; // 0:TCP Client ,1:TCP Server,2:UDP
	tcp_trans_viewMode_t mode; // 0:HEX(默认),1:TEXT
	unsigned char ip[16];
	unsigned char port[6];
};
typedef struct gprs_tcp_ip_s  gprs_tcp_ip_t;

struct gprs_tcp_status_s {
	tcp_channel_t channel; // 0 - 5 通道
	int status;
};
typedef struct gprs_tcp_status_s gprs_tcp_status_t;

struct gprs_driver_api_s {
	int (*Open)(void);
	int (*Close)(void);
	int (*SetOptions)(enum GprsParityBits parity,unsigned int baudrate);
	int (*SetLine)(void);
	int (*SendAtCmd)(void);
	int (*GetVendorInfo)(gprs_vendor_t *vendor);
	int (*GetSoftVersion)(gprs_software_ver_t *ver);
	int (*GetSignalCSQ)(gprs_csq_t *csq);
	int (*GetRegisterStatus)(void);
	int (*GetSimPinStatus)(void);
	int (*GetGsmCpas)(void);
	int (*SetPdpApn)(gprs_pdp_apn_t *apn);
	int (*GetCGATT)(void);
	int (*SetCGATT)(bool state);
	int (*GetCCID)(void);
	int (*SetNetURC)(bool onoff);
	int (*SetNetCON)(gprs_pdp_apn_t *apn);
	int (*SetAuthType)(auth_t *auth);
	int (*SetPassWord)(auth_t *auth);
	int (*SetNetACT)(gprs_tcp_ip_t *tcp);
	int (*CloseNetAct)(gprs_tcp_ip_t *tcp);
	int (*GetNetActStatus)(gprs_tcp_status_t *status);
	int (*SetTcpIpNetSrv)(gprs_tcp_ip_t *tcp);
	int (*SetTcpIpNetOpen)(gprs_tcp_ip_t *tcp);
	int (*TcpIpClose)(gprs_tcp_ip_t *tcp);
	int (*GetTcpUdpLinkStatus)(void);
	int (*PPPClose)(void);
	int (*Send)(gprs_tcp_ip_t *tcp,unsigned char *TxBuffer,unsigned int TxLength);
	int (*Receive)(gprs_tcp_ip_t *tcp,unsigned char *RxBuffer,unsigned int RxBufSize,unsigned int *RxLength);
};
typedef struct gprs_driver_api_s gprs_driver_api_t;


void GprsDriverInit(void);
gprs_driver_api_t *getGprsDriverContext(void);


void GprsTest(void);




/// add by wd 
void gprs_module_enable(void);
void gprs_module_disable(void);

#endif

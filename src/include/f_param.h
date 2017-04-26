/*
 * f_param.h
 *
 *  Created on: 2015-8-11
 *      Author: Johnnyzhang
 */

#ifndef F_PARAM_H_
#define F_PARAM_H_

#include "typedef.h"

//#define F_PARAM_NAME "f_param.dat"
#define F_PARAM_NAME "/opt/concentrator/data/f_param.dat"

#define FPARAM_PROGRAM_KEY_HOLD_TIME	3
#define FPARAM_PROGRAM_INTERVAL			(10 * 60)

#define CON_ADDRESS_LEN	7

#define APN_LENGTH 32

enum {
	FPARAMID_CON_ADDRESS = 1,
	FPARAMID_COMM_HOST_IP_PRI,
	FPARAMID_COMM_HOST_PORT_PRI,
	FPARAMID_COMM_HOST_IP_MINOR,
	FPARAMID_COMM_HOST_PORT_MINOR,
	FPARAMID_HEARTBEAT_CYCLE,
	FPARAMID_READMETER_FREQ,
	FPARAMID_APN_ID,
	FPARAMID_APN_USER_ID,
	FPARAMID_APN_USER_PASSWD,
	FPARAMID_CON_VERIFY_PASSWD,
	FPARAMID_COMM_CON_IP,
	FPARAMID_COMM_CON_PORT,
};

#define FPARAM_COUNT			128
#define FPARAM_LENGTH			3072
struct param {
	WORD id; /* DI */
	WORD len; /* length of DI */
	WORD offset; /* offset of data */
};
typedef struct f_param { /// concentrator parameters data structure
	struct param params[FPARAM_COUNT]; /// 128
	BYTE data[FPARAM_LENGTH]; /// unsigned char, 3072
} F_PARAM;

void fparam_init(void);
void fparam_flush(void);
WORD fparam_get_value(WORD id, void *buf, INT32 max_len);
WORD fparam_set_value(WORD id, const void *buf, INT32 len);
void fparam_destroy(void);

void fparam_change_program_status(void);
int fparam_get_program_status(void);

int reset_fparam_data(void);  // add by wd 
#endif /* F_PARAM_H_ */

/*
 * protocol_cjt188_data.h
 *
 *  Created on: 2015-8-30
 *      Author: Johnnyzhang
 */

#ifndef PROTOCOL_CJT188_DATA_H_
#define PROTOCOL_CJT188_DATA_H_

#include "typedef.h"
#include "common.h"

typedef struct CJT188_DATA_901F{
	WORD di;
	BYTE ser;
	BYTE flux[5];
	BYTE balance_flux[5];
	BYTE clock[7];
	BYTE st[2];
} GASMETER_CJT188_DATA_901F;

typedef enum {
	GASMETER_READ_STATUS_UNREAD,
	GASMETER_READ_STATUS_NORMAL,
	GASMETER_READ_STATUS_ABORT,
} GASMETER_READ_STATUS;

/**
 * 901F: read meter
 */
typedef struct CJT188_901F{
	GASMETER_READ_STATUS status;
	GASMETER_CJT188_DATA_901F di_data;
	long read_tt;
} GASMETER_CJT188_901F;

/// other protocol are forwarded

/**
 * A017: open valve or shutdown valve
 */
typedef struct CJT188_DATA_A017_S{
	WORD di;
	BYTE ser;
	BYTE opration; // 99 - shutdown ;55 - open
}GASMETER_CJT188_DATA_A017_S;

typedef struct CJT188_DATA_A017_R{
	WORD di;
	BYTE ser;
	BYTE st[2];
}GASMETER_CJT188_DATA_A017_R;

typedef struct CJT188_A017{
	GASMETER_READ_STATUS status;
	GASMETER_CJT188_DATA_A017_S di_data;
	long read_tt;
}GASMETER_CJT188_A017_S;

typedef struct CJT188_A017_R{
	GASMETER_READ_STATUS status;
	GASMETER_CJT188_DATA_A017_R di_data;
	long read_tt;
}GASMETER_CJT188_A017_R;

/**
 * A018: write meter address, concentrator needn't to implement this message
 */

/**
 * A015: check meter time
 */
typedef struct CJT188_DATA_A015_S{
	WORD di;
	BYTE ser;
	BYTE clock[7];
}GASMETER_CJT188_DATA_A015_S;

typedef struct CJT188_DATA_A015_R{
	WORD di;
	BYTE ser;
	//BYTE st[2];
}GASMETER_CJT188_DATA_A015_R;

typedef struct CJT188_A015_S{
	GASMETER_READ_STATUS status;
	GASMETER_CJT188_DATA_A015_S di_data;
	long read_tt;
}GASMETER_CJT188_A015_S;

typedef struct CJT188_A015_R{
	GASMETER_READ_STATUS status;
	GASMETER_CJT188_DATA_A015_R di_data;
	long read_tt;
}GASMETER_CJT188_A015_R;

/**
 * 8515: read time
 */
typedef struct CJT188_DATA_8515_S{
	WORD di;
	BYTE ser;
}GASMETER_CJT188_DATA_8515_S;

typedef struct CJT188_DATA_8515_R{
	WORD di;
	BYTE ser;
	BYTE clock[7];
}GASMETER_CJT188_DATA_8515_R;

typedef struct CJT188_8515{
	GASMETER_READ_STATUS status;
	GASMETER_CJT188_DATA_8515_R di_data;
	long read_tt;
}GASMETER_CJT188_8515_R;

/**
 * 903F:read January to June data
 */
typedef struct TEMP_DATA_8515_S{
	WORD di;
	BYTE ser;
	// BYTE clock[7];
}GASMETER_TEMP_DATA_8515_S;

typedef struct TEMP_DATA_8515_R{
	WORD di;
	BYTE ser;
	BYTE sum1[5]; // last_bill_month_flux_sum
	BYTE time1[7];
	BYTE sum2[5];
	BYTE time2[7];
	BYTE sum3[5];
	BYTE time3[7];
	BYTE sum4[5];
	BYTE time4[5];
	BYTE sum5[6];
	BYTE time5[6];
	BYTE sum6[7];
	BYTE time6[7];
}GASMETER_TEMP_DATA_8515_R;

typedef struct TEMP_8515{
	GASMETER_READ_STATUS status;
	GASMETER_CJT188_DATA_8515_R di_data;
	long read_tt;
}GASMETER_TEMP_8515_R;

/**
 * 904F: read July to December data
 */

/**
 * 913f: read last 5 days history data
 */

/**
 * 914f: read 6-10 days before history data
 */

/**
 * 915f: read 6-10 days before history data
 */

/**
 * 916f: read 6-10 days before history data
 */


/**
 * 917f: read 6-10 days before history data
 */

/**
 * 918f: read 6-10 days before history data
 */

/**
 * ff80: write meter status
 */

/**
 * ff00: read meter status
 */

/**
 * a011: write bill day
 */

/**
 * a012: write read-meter-day
 */

/**
 * 8103: read bill-day
 */

/**
 * 8104: read read-meter-day
 */

/**
 * a019: factory-reset
 */

/**
 * 5599: radio frequency module working-parameters setting
 */

#define MAX_HEXDATA_BUF 64
#define PRINT_ADDRESS(x, a, b) hex_to_str(x, sizeof(x), a, b, FALSE)

void ptl_cjt188_data_print(WORD di, void *p);
void ptl_cjt188_data_init(WORD di, void *p);
BOOL ptl_cjt188_data_format(void *p, WORD di, long tt, const BYTE *buf,
		int buflen);

#endif /* PROTOCOL_CJT188_DATA_H_ */

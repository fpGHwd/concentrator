/*
 * f_gasmeter.h
 *
 *  Created on: 2015-8-11
 *      Author: Johnnyzhang
 */

#ifndef F_GASMETER_H_
#define F_GASMETER_H_

#include "typedef.h"

//#define F_GASMETER_NAME "f_gasmeter.dat"
#define F_GASMETER_NAME "/opt/concentrator/data/f_gasmeter.dat"

#define MAX_GASMETER_NUMBER 1000

typedef struct {
	BOOL b_valid;
	BYTE address[7];
	union {
		BYTE unused[64];
		struct {
		} private; ///
	} u;
} COLLECTOR_DB;

typedef struct {
	BOOL b_valid; ///
	BYTE address[5]; ///
	int collector_idx; ///
	union {
		BYTE unused[64];
		struct {
			WORD wakeup_cycle;
			long gasmeter_tt;
			BOOL b_repeater_valid;
			BYTE repeater[2]; /// repeater
		} private;
	} u;
} GASMETER_DB;

void fgasmeter_open(void);
void fgasmeter_close(void);
int fgasmeter_getidx_by_collector(const BYTE *address);
int fgasmeter_getidx_by_gasmeter(const void *address);
BOOL fgasmeter_getcollector(int index, BYTE *address);
BOOL fgasmeter_getgasmeter(int index, BYTE *address, BYTE *collector);
int fgasmeter_addcollector(const BYTE *address);
BOOL fgasmeter_addgasmeter(const BYTE *address, const BYTE *collector);
BOOL fgasmeter_delcollector(const BYTE *address);
BOOL fgasmeter_delgasmeter(const BYTE *address, const BYTE *collector);
BOOL fgasmeter_setgasmeter_clock(int index, long tt);
long fgasmeter_getgasmeter_clock(int index);
BOOL fgasmeter_setgasmeter_wakeupcycle(int index, WORD wakeupcycle);
WORD fgasmeter_getgasmeter_wakeupcycle(int index);
BOOL fgasmeter_get_repeater(const BYTE *address, BYTE *repeater);
BOOL fgasmeter_set_repeater(const BYTE *address, const BYTE *repeater);
BOOL fgasmeter_is_empty(void);

/* add by wd */
int get_valid_meter_amount_in_database(void);
void fgasmeter_assembly_reading(void);
int valid_meter_sum(void);

#endif /* F_GASMETER_H_ */

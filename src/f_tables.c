/*
 * f_tables.c
 *
 *  Created on: 2015-8-15
 *      Author: Johnnyzhang
 */

#include "f_tables.h"
#include "f_param.h"
#include "f_gasmeter.h"

void open_tables(void) {
	fgasmeter_open();
}

void flush_tables(void) {

}

void close_tables(void) {
	fgasmeter_close();
}

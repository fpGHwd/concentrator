/*
 * save_data.c
 *
 *  Created on: May 3, 2017
 *      Author: nayowang
 */


#include <stdio.h>
#include <sqlite3.h>

#define GASMETER_COLLECTOR "gasmeter_collector"
#define DAY_FLUX "day_flux"

int save_data_test(void)
{
	sqlite3 *db;
	char *zErrMsg = 0;
	int rc;

	rc = sqlite3_open(GASMETER_COLLECTOR, &db);
	if(rc){
		PRINTF("Can't open database: %s\n", GASMETER_COLLECTOR);
		sqlite3_close(db);
		return -1;
	}

	char sql_str[] = "create table GASMETER_COLLECTOR(gasmeter varchar(14),collector varchar(14))";
	rc = sqlite3_exec(db, sql_str, NULL, 0, zErrMsg);
	if(rc != SQLITE_OK){
		PRINTF("SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
	sqlite3_close(db);
	return 0;
}

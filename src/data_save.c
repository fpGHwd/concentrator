/*
 * data_save.c
 *
 *  Created on: May 4, 2017
 *      Author: nayowang
 */

#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <semaphore.h>
#include <assert.h>

#include "data_save.h"

sem_t database_sem;

typedef enum{
	false = 0,
	true = 1,
}bool;

#define CONCENTRATOR_DATABASE "concentrator.db"
#define MAX_METERS 1000

typedef enum{
	HOST_IP,
	HOST_PORT,
	HERAT_BEAT,
	READ_METER_FREQUENCY,
	APN_ID,
	APN_USER_ID,
	APN_USER_PASSWORD,
	PASSWORD,
}parameter_t;
const char values[][50] = {
		"120.25.147.231",
		"12345",
		"302",
		"0021",
		"UNINET",
		"CMNET",
		"CMNET",
		"000000",
};
#define INIT_PARAMETER_DESCRIBE(x) {x, #x, {0}}
struct parameter{
	parameter_t idx;
	const char *describe;
	char value[50];
} concentrator_parameter[] = {
	INIT_PARAMETER_DESCRIBE(HOST_IP),
	INIT_PARAMETER_DESCRIBE(HOST_PORT),
	INIT_PARAMETER_DESCRIBE(HERAT_BEAT),
	INIT_PARAMETER_DESCRIBE(READ_METER_FREQUENCY),
	INIT_PARAMETER_DESCRIBE(APN_ID),
	INIT_PARAMETER_DESCRIBE(APN_USER_ID),
	INIT_PARAMETER_DESCRIBE(APN_USER_PASSWORD),
	INIT_PARAMETER_DESCRIBE(PASSWORD),
};

int create_table(void)
{
	sqlite3 *db;
	char *zErrMsg;
	int rc;
	char sql[1000];
	bool ret = true;
	int i;

	rc = sqlite3_open(CONCENTRATOR_DATABASE,&db);

	sqlite3_exec(db, "begin:", 0, 0, 0);

	snprintf(sql, 999,
			"create table if not exists "
			"parameters(name char(25) primary key, value char(50));"
			"create table if not exists "
			"meters(meter_id char(14) primary key, collector char(14) default '00000000000000');"
			"create table if not exists "
			"collectors(id integer primary key, collector_id char(32));"
			"create table if not exists "
			"current_data(id integer primary key, current_data real, time char(25));"
			"create table if not exists "
			"month_data(id integer primary key, month_data real, time char(25));"
			"create table if not exists "
			"day_index(id integer primary key, date char(25));");
	rc = sqlite3_exec(db,sql,NULL,NULL,&zErrMsg);
	if (rc != SQLITE_OK)
	{
		fprintf(stderr,"%s, %d: create tables failed: %s\n", __func__, __LINE__, zErrMsg );
		sqlite3_free(zErrMsg);
		sqlite3_close(db);
		return -1;
	}

	for(i = 0; i< 60; i++){
		snprintf(sql, 100, "create table if not exists "
				"day_%03d(id integer primary key, day_data real, time char(25));", i);
		rc = sqlite3_exec(db,sql,NULL,NULL,&zErrMsg);
		if(rc != SQLITE_OK){
			fprintf(stderr, "%s: sqlite roll back: %s, i(%d)\n",__func__, zErrMsg, i);
			sqlite3_exec(db, "rollback:",0,0,0);
			break;
		}
	}
	sqlite3_exec(db, "commit:", 0, 0,0);
	sqlite3_free(zErrMsg);
	sqlite3_close(db);
	return 0;
}

bool initiate_parameter(void)
{
	sqlite3 *db;
	const char *zErrMsg;
	int rc;
	bool ret = true;
	char sql[100];
	parameter_t p;

	for(p = HOST_IP; p <= PASSWORD; p++){
		memcpy(concentrator_parameter[p].value, values[p], 50);
	}

	rc = sqlite3_open(CONCENTRATOR_DATABASE,&db);
	if (rc != SQLITE_OK){
		fprintf(stderr, "open database error\n");
		return -1;
	}

	sqlite3_exec(db, "begin:", 0, 0, 0);
	for(p = HOST_IP; p <= PASSWORD; p++){
		snprintf(sql, 100-1, "insert or replace into parameters(name, value) "
				"values('%s','%s');",
				concentrator_parameter[p].describe, concentrator_parameter[p].value);
		rc = sqlite3_exec(db,sql,NULL,NULL,&zErrMsg);
		if(rc != SQLITE_OK){
			fprintf(stderr, "sqlite roll back: %s, i(%d)\n", zErrMsg, p);
			sqlite3_exec(db, "rollback:",0,0,0);
			ret = -1;
			break;
		}
	}
	ret = 0;
	sqlite3_exec(db, "commit:", 0, 0,0);
	//sqlite3_free(zErrMsg);
	sqlite3_close(db);
	return ret;
}

/**
 * get parameter
 * @param para
 * @param value
 * @return
 */
bool get_parameter(const parameter_t para, void *value)
{
	sqlite3 *db;
	sqlite3_stmt *stmt;
	const char *zErrMsg;
	int count = 0;
	bool ret = false;
	int rc;

	rc = sqlite3_open(CONCENTRATOR_DATABASE, &db);
	if (rc != SQLITE_OK){
		fprintf(stderr, "open database error\n");
		return false;
	}

	sqlite3_prepare(db, "select * from parameter where name=?", -1, &stmt, &zErrMsg);
	sqlite3_bind_text(stmt, 1, (const char*)concentrator_parameter[para].describe, -1,SQLITE_STATIC);

	count = sqlite3_column_count(stmt); /// columns of the table
	rc = sqlite3_step(stmt);
	if(rc == SQLITE_ROW){
		strncpy(value, (const char*) sqlite3_column_text(stmt,1), 25);
		ret = true;
	}else{
		ret = false;
	}
	sqlite3_finalize(stmt);
	sqlite3_close(db);
	return ret;
}

bool if_meter_in_meters(unsigned char *meter)
{
	bool ret = false;
	int rc;
	sqlite3 *db;
	sqlite3_stmt *stmt;
	const char *zErrMsg;

	if(meter == NULL)
		return false;

	rc = sqlite3_open(CONCENTRATOR_DATABASE, &db);
	if(rc != SQLITE_OK){
		fprintf(stderr,"open database error\n");
		return false;
	}

	sqlite3_prepare(db, "select * from meters where meter_id=?;" ,
			-1, &stmt, &zErrMsg);
	sqlite3_bind_text(stmt, 1, (const char*)meter, -1, SQLITE_STATIC);

	do{
		rc = sqlite3_step(stmt);
		if(rc == SQLITE_ROW){
			ret = true;
			break;
		}
	}while(rc != SQLITE_DONE && rc != SQLITE_ERROR);

	sqlite3_finalize(stmt);
	//sqlite3_free(zErrMsg);
	sqlite3_close(db);

	return ret;
}

/**
 * when add_flag = 1, add meter; add_flag = 0, remove meter, in the database
 * @param meter
 * @param add_flag
 * @return
 */
bool add_or_remove_a_meter(unsigned char *meter, int add_flag)
{
	bool ret = true;
	int rc;
	sqlite3 *db;
	sqlite3_stmt *stmt;
	const char *zErrMsg;

	//assert(meter != NULL);
	if(meter == NULL)
		return false;

	rc = sqlite3_open(CONCENTRATOR_DATABASE, &db);
	if(rc != SQLITE_OK){
		fprintf(stderr,"open database error\n");
		return false;
	}

	if(add_flag == 1)
		sqlite3_prepare(db, "insert into meters(meter_id) values(?);", -1, &stmt, &zErrMsg);
	else if(add_flag == 0)
		sqlite3_prepare(db, "delete from meters where meter_id=?;", -1, &stmt, &zErrMsg);
	sqlite3_bind_text(stmt, 1, (const char *)meter, -1, SQLITE_STATIC);
	rc = sqlite3_step(stmt);
	if(rc == SQLITE_DONE){
		fprintf(stdout, "%s meter(%s) successfully\n",(add_flag?"add":"remove"), meter);
		ret = true;
	}else{
		fprintf(stderr, "%s meter(%s) failed, rc(%d)\n",(add_flag?"add":"remove"), meter, rc);
		ret = false;
	}
	sqlite3_finalize(stmt);
	sqlite3_close(db);
	return ret;
}

#define SQLITE_NORMAL (SQLITE_OK || SQLITE_ROW || SQLITE_DONE)

/**
 * add meter table
 * @param meter
 * @param add_flag
 * @return
 */
bool add_or_remove_meter_table(unsigned char *meter, int add_flag)
{
	bool ret = true;
	int rc;
	sqlite3 *db;
	sqlite3_stmt *stmt;
	const char *zErrMsg;
	char tablename[20];
	char sql[100];

	//assert(meter != NULL);
	if(meter == NULL)
		return false;

	rc = sqlite3_open(CONCENTRATOR_DATABASE, &db);
	if(rc != SQLITE_OK){
		fprintf(stderr,"open database error\n");
		return false;
	}

	if(add_flag == 1)
		snprintf(sql, 100, "create table if not exists meter%s(date_string char(10), flux long, time_t long);", meter);
	else
		snprintf(sql, 100, "drop table meter%s;", meter);
	//snprintf(tablename, 20, "meter%s", meter);
	//tablename[19] = 0;
	//if(add_flag == 1){
		sqlite3_prepare(db, sql/*"create table if not exists ?(date_string char(10), flux long, time_t long);"*/, -1, &stmt, &zErrMsg);
	//}else{
		//sqlite3_prepare(db, "drop table ?;", -1, &stmt, &zErrMsg);
	//}
	sqlite3_bind_text(stmt, 1, (const char *)tablename, -1, SQLITE_STATIC);
	rc = sqlite3_step(stmt);
	if(rc == SQLITE_DONE){
		fprintf(stdout, "%s table(meter%s) successfully\n",(add_flag?"add":"remove"), meter);
		ret = true;
	}else{
		fprintf(stderr, "%s table(meter%s) failed: rc = %d\n",(add_flag?"add":"remove"), meter, zErrMsg, rc);
		ret = false;
	}
	sqlite3_finalize(stmt);
	sqlite3_close(db);
	return ret;
}

/**
 * update collector for meter
 * @param meter
 * @param collector
 * @return
 */
bool update_collector_for_meter(unsigned char *meter, unsigned char *collector)
{
	bool ret = true;
	int rc;
	sqlite3 *db;
	sqlite3_stmt *stmt;
	const char *zErrMsg;
	char sql[100];

	if(meter == NULL || collector == NULL)
		return false;

	rc = sqlite3_open(CONCENTRATOR_DATABASE, &db);
	if(rc != SQLITE_OK){
		fprintf(stderr,"open database error\n");
		return false;
	}

	snprintf(sql, 100, "update meters set collector='%s' where meter_id='%s';",
			collector, meter);
	//rc = sqlite3_exec(db, sql, -1, &stmt, &zErrMsg);
	sqlite3_prepare(db, sql, -1, &stmt, &zErrMsg);
	sqlite3_bind_text(stmt, 1, (const char *)meter, -1, SQLITE_STATIC);
	rc = sqlite3_step(stmt);
	if(rc == SQLITE_ERROR){
		fprintf(stderr, "update collector(%s) for meter(%s) failed\n",
				collector,meter);
		ret = false;
	}else{
		fprintf(stdout, "update collector(%s) for meter(%s) successfully\n",
				collector,meter);
		ret = true;
	}
	sqlite3_finalize(stmt);
	sqlite3_close(db);
	return ret;
}

void print_parameter_test(void)
{
	parameter_t k;
	char value[25];
	for(k = HOST_IP; k <= PASSWORD; k++){
		if(get_parameter(k, value))
			fprintf(stdout,"%s value is %s\n", concentrator_parameter[k].describe, value);
	}
}

void meter_and_collector_test(void)
{
	unsigned char *meter = "23051705000008";
	char collector[15] = "00000000000013";
	// update collector
	// update_collector_for_meter(meter, collector);

	/*
	if(!if_meter_in_meters(meter_id))
		fprintf(stdout, "meter(%s) is not in database\n", meter_id);
	else
		fprintf(stdout, "meter(%s) is in database\n", meter_id);
	*/

	/*
	if(add_a_meter(meter_id))
		fprintf(stdout, "Add meter successful\n");
	else
		fprintf(stdout, "Add meter failed\n");
	*/

	/*
	if(add_or_remove_a_meter(meter_id, 0))
		fprintf(stdout, "remove successfully\n");
	*/
}

void add_meters_set(int zero_to_int, bool add)
{
	char *prefix = "23051705";
	int i;
	char meter_s[15];

	//initiate_parameter();

	memcpy(meter_s, prefix, 8);
	for(i=1; i<zero_to_int+1; i++){
		snprintf(meter_s + 8, 7, "%06d", i); /// integer to string;; sscanf: string->integer
		if(add_or_remove_a_meter(meter_s, add))
			add_or_remove_meter_table(meter_s, add);
	}
}

void data_save_test(void)
{
	add_meters_set(100, true);
	//add_meters_set(100, false);
	return;
}

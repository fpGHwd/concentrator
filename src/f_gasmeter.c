/*
 * f_gasmeter.c
 *
 *  Created on: 2015-8-11
 *      Author: Johnnyzhang
 */

#include "f_gasmeter.h" /// f_gasmeter>
#include "common.h"
#include "main.h"

typedef struct {
	COLLECTOR_DB collector_db[MAX_GASMETER_NUMBER];
	GASMETER_DB gasmeter_db[MAX_GASMETER_NUMBER];
}GASMETER_COLLECTOR_DB;

#define COLLECTOR_OFFSET offsetof(GASMETER_COLLECTOR_DB, collector_db) 
#define GASMETER_OFFSET offsetof(GASMETER_COLLECTOR_DB, gasmeter_db)

typedef struct {
	GASMETER_COLLECTOR_DB db; /// meters & collectors db
	sem_t sem_db;
	sem_t sem_f_gasmeter;
	int fd;
} GASMETER_INFO;

static GASMETER_INFO gasmeter_info; /// meters(not a meter), the biggest conception concrete, help you work

static void fgasmeter_init_collector(COLLECTOR_DB *pdb)
{
	if (!pdb)
		return;
	memset(pdb, 0, sizeof(COLLECTOR_DB));
}

static void fgasmeter_init_gasmeter(GASMETER_DB *pdb)
{
	if (!pdb)
		return;
	memset(pdb, 0, sizeof(GASMETER_DB));
	pdb->collector_idx = -1;
}

void test_add_a_meter(void);
void fgasmeter_open(void)
{
	int i, size,m;
	const char *name = F_GASMETER_NAME;
	GASMETER_INFO *pinfo = &gasmeter_info;

	size = sizeof(GASMETER_COLLECTOR_DB);
	sem_init(&pinfo->sem_db, 0, 1);
	sem_init(&pinfo->sem_f_gasmeter, 0, 1);
	if (!check_file(name, size)) {
		PRINTF("File %s is created, size:%d\n", name, size);
		pinfo->fd = open(name, O_CREAT | O_RDWR | O_TRUNC, 0600);
		for (i = 0; i < MAX_GASMETER_NUMBER; i++) {
			fgasmeter_init_collector(&pinfo->db.collector_db[i]);
			fgasmeter_init_gasmeter(&pinfo->db.gasmeter_db[i]);
		}
		safe_write(pinfo->fd, &pinfo->db, size);
		fdatasync(pinfo->fd);
		close(pinfo->fd);
	}
	pinfo->fd = open(name, O_RDWR);
	if (pinfo->fd < 0)
		return;
	safe_read(pinfo->fd, &pinfo->db, size);

	/*
	if(debug_ctrl.gasmeter_test){
		PRINTF("test meter: add a meter first, which is already in the meter database\n");
		test_add_a_meter();
	}
	*/
}

static void fgasmeter_flush(void) {
	fdatasync(gasmeter_info.fd);
}

static void fgasmeter_update_collector(int index, BOOL flush_flag)
{
	GASMETER_INFO *pinfo = &gasmeter_info;

	sem_wait(&pinfo->sem_f_gasmeter);
	lseek(pinfo->fd, COLLECTOR_OFFSET + index * sizeof(COLLECTOR_DB), SEEK_SET);
	safe_write(pinfo->fd, pinfo->db.collector_db + index, sizeof(COLLECTOR_DB));
	if (flush_flag) {
		fgasmeter_flush();
	}
	sem_post(&pinfo->sem_f_gasmeter);
}

static void fgasmeter_update_gasmeter(int index, BOOL flush_flag) {
	GASMETER_INFO *pinfo = &gasmeter_info; /// get static variable content

	sem_wait(&pinfo->sem_f_gasmeter);
	lseek(pinfo->fd, GASMETER_OFFSET + index * sizeof(GASMETER_DB), SEEK_SET);
	safe_write(pinfo->fd, pinfo->db.gasmeter_db + index, sizeof(GASMETER_DB)); /// pinfo->db.gasmeter_db + index
	if (flush_flag) {
		fgasmeter_flush();
	}
	sem_post(&pinfo->sem_f_gasmeter);
}

void fgasmeter_close(void) {
	GASMETER_INFO *pinfo = &gasmeter_info;

	fdatasync(pinfo->fd);
	close(pinfo->fd);
	sem_destroy(&pinfo->sem_f_gasmeter);
	sem_destroy(&pinfo->sem_db);
}

int fgasmeter_getidx_by_collector(const BYTE *address)
{
	int i, index = -1;
	GASMETER_INFO *pinfo = &gasmeter_info;
	COLLECTOR_DB *pdb;

	sem_wait(&pinfo->sem_db);
	for (i = 0; i < MAX_GASMETER_NUMBER; i++) {
		pdb = &pinfo->db.collector_db[i];
		if (!pdb->b_valid)
			continue;
		if (memcmp(address, pdb->address, 5) == 0) {
			index = i;
			break;
		}
	}
	sem_post(&pinfo->sem_db);
	return index;
}

int fgasmeter_getidx_by_gasmeter(const void *address)
{
	int i, index = -1;
	GASMETER_INFO *pinfo = &gasmeter_info;
	GASMETER_DB *pdb;

	address = (const BYTE*)address;

	sem_wait(&pinfo->sem_db);
	for (i = 0; i < MAX_GASMETER_NUMBER; i++) {
		pdb = &pinfo->db.gasmeter_db[i];
		if (!pdb->b_valid)
			continue;
		if (memcmp(address, pdb->address, 7) == 0) {
			index = i;
			break;
		}
	}
	sem_post(&pinfo->sem_db);
	return index;
}

BOOL fgasmeter_getcollector(int index, BYTE *address)
{
	GASMETER_INFO *pinfo = &gasmeter_info;
	COLLECTOR_DB *pdb;

	if (index < 0)
		return FALSE;
	sem_wait(&pinfo->sem_db);
	pdb = &pinfo->db.collector_db[index];
	if (!pdb->b_valid) {
		sem_post(&pinfo->sem_db);
		return FALSE;
	}
	if (address) {
		memcpy(address, pdb->address, 7);
	}
	sem_post(&pinfo->sem_db);
	return index;
}

BOOL fgasmeter_getgasmeter(int index, BYTE *address, BYTE *collector)
{
	GASMETER_INFO *pinfo = &gasmeter_info;
	GASMETER_DB *pdb1;
	COLLECTOR_DB *pdb2;

	if (index < 0)
		return FALSE;
	sem_wait(&pinfo->sem_db);
	pdb1 = &pinfo->db.gasmeter_db[index];
	if (!pdb1->b_valid) {
		sem_post(&pinfo->sem_db);
		return FALSE;
	}
	if (address) {
		memcpy(address, pdb1->address, 7);
	}
	if (collector) {
		if (pdb1->collector_idx
				>= 0&& pdb1->collector_idx < MAX_GASMETER_NUMBER) {
			pdb2 = &pinfo->db.collector_db[pdb1->collector_idx];
			if (!pdb2->b_valid) {
				memcpy(collector, pdb2->address, 5);
			} else {
				memset(collector, 0, 5);
			}
		} else {
			memset(collector, 0, 5);
		}
	}
	sem_post(&pinfo->sem_db);
	return TRUE;
}

static int fgasmeter_addcollector_sub(const BYTE *address, BOOL lock)
{
	int i, index = -1;
	GASMETER_INFO *pinfo = &gasmeter_info;
	COLLECTOR_DB *pdb;
	BOOL b_add_success = FALSE;

	if (address == NULL)
		return -1;
	if (lock) {
		sem_wait(&pinfo->sem_db);
	}
	for (i = 0; i < MAX_GASMETER_NUMBER; i++) {
		pdb = &pinfo->db.collector_db[i];
		if (pdb->b_valid && memcmp(pdb->address, address, 5) == 0) { // fixme: maybe 7
			b_add_success = TRUE;
			index = i;
			break;
		}
	}
	if (!b_add_success) {
		for (i = 0; i < MAX_GASMETER_NUMBER; i++) {
			pdb = &pinfo->db.collector_db[i];
			if (!pdb->b_valid) {
				fgasmeter_init_collector(pdb);
				memcpy(pdb->address, address, 5);
				pdb->b_valid = TRUE;
				fgasmeter_update_collector(i, TRUE);
				index = i;
				b_add_success = TRUE;
				break;
			}
		}
	}
	if (lock) {
		sem_post(&pinfo->sem_db);
	}
	if (b_add_success)
		return index;
	else
		return -1;
}

int fgasmeter_addcollector(const BYTE *address) {
	return fgasmeter_addcollector_sub(address, TRUE);
}

BOOL fgasmeter_delcollector(const BYTE *address) {
	int i;
	GASMETER_INFO *pinfo = &gasmeter_info;
	COLLECTOR_DB *pdb;
	BOOL b_del_success = FALSE;

	if (address == NULL)
		return FALSE;
	sem_wait(&pinfo->sem_db);
	for (i = 0; i < MAX_GASMETER_NUMBER; i++) {
		pdb = &pinfo->db.collector_db[i];
		if (pdb->b_valid && memcmp(pdb->address, address, 5) == 0) {
			fgasmeter_init_collector(pdb);
			fgasmeter_update_collector(i, TRUE);
			b_del_success = TRUE;
			break;
		}
	}
	sem_post(&pinfo->sem_db);
	return b_del_success;
}

void start_read_gasmeter(void);
BOOL fgasmeter_addgasmeter(const BYTE *address, const BYTE *collector)
{
	int i;
	GASMETER_INFO *pinfo = &gasmeter_info;
	GASMETER_DB *pdb;
	BOOL b_add_success = FALSE;

	if (address == NULL || collector == NULL)
		return FALSE;
	sem_wait(&pinfo->sem_db);
	for (i = 0; i < MAX_GASMETER_NUMBER; i++) {
		pdb = &pinfo->db.gasmeter_db[i];
		if (pdb->b_valid && memcmp(pdb->address, address, 7) == 0) {
			pdb->collector_idx = fgasmeter_addcollector_sub(collector, FALSE);
			fgasmeter_update_collector(pdb->collector_idx, TRUE);
			b_add_success = TRUE;
			break;
		}
	}
	if (!b_add_success) { // if not found
		for (i = 0; i < MAX_GASMETER_NUMBER; i++) {
			pdb = &pinfo->db.gasmeter_db[i];
			if (!pdb->b_valid) { // find a least number of invalid gasmeter and replace it with new gasmeter adress
				fgasmeter_init_gasmeter(pdb);
				memcpy(pdb->address, address, 7);
				pdb->collector_idx = fgasmeter_addcollector_sub(collector,
						FALSE);
				fgasmeter_update_collector(pdb->collector_idx, TRUE);
				pdb->b_valid = TRUE;
				fgasmeter_update_gasmeter(i, TRUE);
				b_add_success = TRUE;
				break;
			}
		}
	}
	sem_post(&pinfo->sem_db);

	/** cancel read meter after adding meter */
	/*
	if (b_add_success)
		start_read_gasmeter();
		*/

	return b_add_success;
}

BOOL fgasmeter_delgasmeter(const BYTE *address, const BYTE *collector)
{
	int i;
	GASMETER_INFO *pinfo = &gasmeter_info;
	GASMETER_DB *pdb1;
	COLLECTOR_DB *pdb2;
	BOOL b_del_success = FALSE;

	if (address == NULL)
		return FALSE;
	sem_wait(&pinfo->sem_db);
	for (i = 0; i < MAX_GASMETER_NUMBER; i++) {
		pdb1 = &pinfo->db.gasmeter_db[i];
		if (pdb1->b_valid && memcmp(pdb1->address, address, 7) == 0) {
			if (collector
					&& pdb1->collector_idx
							>= 0&& pdb1->collector_idx < MAX_GASMETER_NUMBER) {
				pdb2 = &pinfo->db.collector_db[pdb1->collector_idx];
				if (pdb2->b_valid && memcmp(pdb2->address, collector, 7) == 0) {
					fgasmeter_init_gasmeter(pdb1);
					fgasmeter_update_gasmeter(i, TRUE);
					b_del_success = TRUE;
				} else
					continue;
			} else {
				fgasmeter_init_gasmeter(pdb1);
				fgasmeter_update_gasmeter(i, TRUE);
				b_del_success = TRUE;
				break;
			}
		}
	}
	sem_post(&pinfo->sem_db);
	return b_del_success;
}

BOOL fgasmeter_setgasmeter_clock(int index, long tt)
{
	GASMETER_INFO *pinfo = &gasmeter_info;
	GASMETER_DB *pdb;

	if (index < 0 || index >= MAX_GASMETER_NUMBER)
		return FALSE;
	sem_wait(&pinfo->sem_db);
	pdb = &pinfo->db.gasmeter_db[index];
	pdb->u.private.gasmeter_tt = tt;
	fgasmeter_update_gasmeter(index, TRUE);
	sem_post(&pinfo->sem_db);
	return TRUE;
}

long fgasmeter_getgasmeter_clock(int index)
{
	GASMETER_INFO *pinfo = &gasmeter_info;
	GASMETER_DB *pdb;
	long tt;

	if (index < 0 || index >= MAX_GASMETER_NUMBER)
		return -1;
	sem_wait(&pinfo->sem_db);
	pdb = &pinfo->db.gasmeter_db[index];
	tt = pdb->u.private.gasmeter_tt;
	sem_post(&pinfo->sem_db);
	return tt;
}

BOOL fgasmeter_setgasmeter_wakeupcycle(int index, WORD wakeupcycle)
{
	GASMETER_INFO *pinfo = &gasmeter_info;
	GASMETER_DB *pdb;

	if (index < 0 || index >= MAX_GASMETER_NUMBER)
		return FALSE;
	sem_wait(&pinfo->sem_db);
	pdb = &pinfo->db.gasmeter_db[index];
	pdb->u.private.wakeup_cycle = wakeupcycle;
	fgasmeter_update_gasmeter(index, TRUE);
	sem_post(&pinfo->sem_db);
	return TRUE;
}

WORD fgasmeter_getgasmeter_wakeupcycle(int index) {
	GASMETER_INFO *pinfo = &gasmeter_info;
	GASMETER_DB *pdb;
	WORD wakeupcycle;

	if (index < 0 || index >= MAX_GASMETER_NUMBER)
		return -1;
	sem_wait(&pinfo->sem_db);
	pdb = &pinfo->db.gasmeter_db[index];
	wakeupcycle = pdb->u.private.wakeup_cycle;
	sem_post(&pinfo->sem_db);
	return wakeupcycle;
}

BOOL fgasmeter_get_repeater(const BYTE *address, BYTE *repeater)
{

	GASMETER_INFO *pinfo = &gasmeter_info;
	GASMETER_DB *pdb;
	int index = -1;

	if (address == NULL || repeater == NULL)
		return FALSE;
	sem_wait(&pinfo->sem_db);
	for (index = 0; index < MAX_GASMETER_NUMBER; index++) {
		pdb = &pinfo->db.gasmeter_db[index];
		if (memcmp(address, pdb->address, 7) == 0
				&& pdb->u.private.b_repeater_valid) {
			memcpy(repeater, pdb->u.private.repeater, 2);
			///PRINTB("fgasmeter_get_repeater: ", address, 7);
			break;
		}
	}
	sem_post(&pinfo->sem_db);
	return TRUE;
}

BOOL fgasmeter_set_repeater(const BYTE *address, const BYTE *repeater)
{
	GASMETER_INFO *pinfo = &gasmeter_info;
	GASMETER_DB *pdb;
	int index = -1;

	///PRINTB("address byte: ", address, 7);
	///PRINTB("repeater byte: ", repeater, 2);

	if (address == NULL || repeater == NULL)
		return FALSE;
	sem_wait(&pinfo->sem_db);
	for (index = 0; index < MAX_GASMETER_NUMBER; index++) {
		pdb = &pinfo->db.gasmeter_db[index];
		if (memcmp(address, pdb->address, 7) == 0) { /// get the meter
			memcpy(pdb->u.private.repeater, repeater, 2);
			pdb->u.private.b_repeater_valid = TRUE; /// valid 已经为true
			fgasmeter_update_gasmeter(index, TRUE);
			break;
		}
	}
	sem_post(&pinfo->sem_db);
	if (index >= 0 && index < MAX_GASMETER_NUMBER)
		return TRUE; /// get the index of meter
	else
		return FALSE;
}

BOOL fgasmeter_is_empty(void) {
	int i, index = -1;
	GASMETER_INFO *pinfo = &gasmeter_info;
	GASMETER_DB *pdb;

	sem_wait(&pinfo->sem_db);
	for (i = 0; i < MAX_GASMETER_NUMBER; i++) {
		pdb = &pinfo->db.gasmeter_db[i];
		if (pdb->b_valid) {
			index = i;
			break;
		}
	}
	sem_post(&pinfo->sem_db);
	return (index < 0) ? TRUE : FALSE;
}

int get_valid_meter_amount_in_database(void) /// added by wd
{
	int idx, ret;

	BYTE address[7], collector[5];

	ret = 0;
	for (idx = 0; idx < MAX_GASMETER_NUMBER; idx++) {
		if (fgasmeter_getgasmeter(idx, address, collector)) {
			ret++;
		} else {
			continue;
		}
	}
	return ret;
}

int gasmeter_read_di(const BYTE *address, const BYTE *collector, WORD di,
		BYTE *buf, int max_len);
void fgasmeter_assembly_reading(void) {
	int /*valid_meters,*/ idx;
	//GASMETER_DB *pdb;
	//GASMETER_INFO *pinfo = &gasmeter_info;
	char buf[20 + 1];

	int resp_len;
	BYTE address[7], collector[5];
	BYTE resp_buf[512];

	for (idx = 0; idx < MAX_GASMETER_NUMBER; idx++) {
		if (fgasmeter_getgasmeter(idx, address, collector)) {
			// read_a_meter
			PRINTF(buf);
			resp_len = gasmeter_read_di(address, collector, 0x901F, resp_buf,
					sizeof(resp_buf));
			/// save data
		} else {
			continue;
		}
	}
}

int valid_meter_sum(void) {
	int ret;
	unsigned short idx;

	ret = 0;
	for (idx = 0; idx < MAX_GASMETER_NUMBER; idx++) {
		if (fgasmeter_getgasmeter(idx, NULL, NULL)) {
			ret++;
		}
	}

	return ret;
}

void test_add_a_meter(void) {
	BYTE address[7] = { 0x23, 0x05, 0x16, 0x06, 0x00, 0x00, 0x02 };
	BYTE collector[7] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55 };

	if(fgasmeter_addgasmeter(address, collector)){
		printf("add a meter for test successfully\n");
	}

	return;
}

int reset_gasmeter_data(void) {
	GASMETER_INFO *gas_info = &gasmeter_info;
	int ret;

	sem_wait(&gas_info->sem_db);
	if (remove(F_GASMETER_NAME) == 0) {
		ret = 0;
	} else {
		ret = -1;
	}
	fgasmeter_open();
	sem_post(&gas_info->sem_db);

	return ret;
}

/*
 * f_gasmeter.c
 *
 *  Created on: 2015-8-11
 *      Author: Johnnyzhang
 */

#include "f_gasmeter.h" /// f_gasmeter>
#include "common.h"

typedef struct {
	COLLECTOR_DB collector_db[MAX_GASMETER_NUMBER];
	GASMETER_DB gasmeter_db[MAX_GASMETER_NUMBER];
} GASMETER_COLLECTOR_DB; /// collector database

#define COLLECTOR_OFFSET offsetof(GASMETER_COLLECTOR_DB, collector_db) 
#define GASMETER_OFFSET offsetof(GASMETER_COLLECTOR_DB, gasmeter_db)

typedef struct {
	GASMETER_COLLECTOR_DB db; /// meters & collectors db
	sem_t sem_db;
	sem_t sem_f_gasmeter;
	int fd;
} GASMETER_INFO; /// domain info and file name, gas meter info

static GASMETER_INFO gasmeter_info; /// meters(not a meter), the biggest conception concrete, help you work

static void fgasmeter_init_collector(COLLECTOR_DB *pdb) /// initiate collector db
{
	if (!pdb)
		return;
	memset(pdb, 0, sizeof(COLLECTOR_DB));
}

static void fgasmeter_init_gasmeter(GASMETER_DB *pdb) /// initiate gasmeter db
{
	if (!pdb)
		return;
	memset(pdb, 0, sizeof(GASMETER_DB));
	pdb->collector_idx = -1;
}

#define TEST
void test_add_a_meter(void);
void fgasmeter_open(void) /// open and initiate gasmeter_info
{
	int i, size;
	const char *name = F_GASMETER_NAME; ///
	GASMETER_INFO *pinfo = &gasmeter_info;

	size = sizeof(GASMETER_COLLECTOR_DB);
	sem_init(&pinfo->sem_db, 0, 1); /// not shared, set sem_db = 1
	sem_init(&pinfo->sem_f_gasmeter, 0, 1); /// not shared, set sem_f_gasmeter = 1
	if (!check_file(name, size)) { /// check file
		PRINTF("File %s is created, size:%d\n", name, size);
		pinfo->fd = open(name, O_CREAT | O_RDWR | O_TRUNC, 0600);
		for (i = 0; i < MAX_GASMETER_NUMBER; i++) {
			fgasmeter_init_collector(&pinfo->db.collector_db[i]);
			fgasmeter_init_gasmeter(&pinfo->db.gasmeter_db[i]);
		}
		safe_write(pinfo->fd, &pinfo->db, size);
		fdatasync(pinfo->fd); /// flush
		close(pinfo->fd);
	}
	pinfo->fd = open(name, O_RDWR);
	if (pinfo->fd < 0)
		return;
	safe_read(pinfo->fd, &pinfo->db, size); /// &pinfo->db
#ifdef TEST
	test_add_a_meter();
	return;
#else
	return;
#endif

}

static void fgasmeter_flush(void) {
	fdatasync(gasmeter_info.fd); ///
}

static void fgasmeter_update_collector(int index, BOOL flush_flag) /// sem_t indicates a territory
/// update fix-index collector
{
	GASMETER_INFO *pinfo = &gasmeter_info; /// get static variable content

	sem_wait(&pinfo->sem_f_gasmeter);
	lseek(pinfo->fd, COLLECTOR_OFFSET + index * sizeof(COLLECTOR_DB), SEEK_SET); /// COLLECTOR_OFFSET
	safe_write(pinfo->fd, pinfo->db.collector_db + index, sizeof(COLLECTOR_DB));
	if (flush_flag) {
		fgasmeter_flush();
	}
	sem_post(&pinfo->sem_f_gasmeter);
}

static void fgasmeter_update_gasmeter(int index, BOOL flush_flag) {
	GASMETER_INFO *pinfo = &gasmeter_info; /// get static variable content

	sem_wait(&pinfo->sem_f_gasmeter); /// ?
	lseek(pinfo->fd, GASMETER_OFFSET + index * sizeof(GASMETER_DB), SEEK_SET);
	safe_write(pinfo->fd, pinfo->db.gasmeter_db + index, sizeof(GASMETER_DB)); /// pinfo->db.gasmeter_db + index
	if (flush_flag) {
		fgasmeter_flush();
	}
	sem_post(&pinfo->sem_f_gasmeter); /// ?
}

void fgasmeter_close(void) {
	GASMETER_INFO *pinfo = &gasmeter_info;

	fdatasync(pinfo->fd);
	close(pinfo->fd);
	sem_destroy(&pinfo->sem_f_gasmeter);
	sem_destroy(&pinfo->sem_db);
}

int fgasmeter_getidx_by_collector(const BYTE *address) /// find the address index in RAM
{
	int i, index = -1;
	GASMETER_INFO *pinfo = &gasmeter_info;
	COLLECTOR_DB *pdb;

	sem_wait(&pinfo->sem_db);
	for (i = 0; i < MAX_GASMETER_NUMBER; i++) {
		pdb = &pinfo->db.collector_db[i];
		if (!pdb->b_valid) ///
			continue;
		if (memcmp(address, pdb->address, 5) == 0) { /// compare 5 bytes /// read source codes in little C project
			index = i;
			break;
		}
	}
	sem_post(&pinfo->sem_db); /// ?
	return index;
}

int fgasmeter_getidx_by_gasmeter(const BYTE *address) /// 可以查询是否有gasmeter
{
	//PRINTB("fgasmeter_getidx_by_gasmeter", address, 7);
	int i, index = -1;
	GASMETER_INFO *pinfo = &gasmeter_info;
	GASMETER_DB *pdb;

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
	return index; /// index = -1; return;
}

BOOL fgasmeter_getcollector(int index, BYTE *address) /// get gasmeter
{
	GASMETER_INFO *pinfo = &gasmeter_info;
	COLLECTOR_DB *pdb;

	if (index < 0)
		return FALSE;
	sem_wait(&pinfo->sem_db); /// set memory wait /// river structure make us clear and release, while other complex structure do not
	pdb = &pinfo->db.collector_db[index]; /// collector struct
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

BOOL fgasmeter_getgasmeter(int index, BYTE *address, BYTE *collector) /// 
{
	GASMETER_INFO *pinfo = &gasmeter_info; /// has been refreshed
	GASMETER_DB *pdb1; /// a gasmter
	COLLECTOR_DB *pdb2; /// a_collecter

	if (index < 0)
		return FALSE;
	sem_wait(&pinfo->sem_db);
	pdb1 = &pinfo->db.gasmeter_db[index];
	if (!pdb1->b_valid) {
		sem_post(&pinfo->sem_db); /// 信号量-
		return FALSE;
	}
	if (address) { /// pdb1->b_valid = valid
		memcpy(address, pdb1->address, 7);
	}
	if (collector) {
		if (pdb1->collector_idx
				>= 0&& pdb1->collector_idx < MAX_GASMETER_NUMBER) {
			pdb2 = &pinfo->db.collector_db[pdb1->collector_idx]; ///
			if (!pdb2->b_valid) { /// if pdb2->b_valid = false; 
				memcpy(collector, pdb2->address, 5); /// this is set the gasemter
			} else { /// if 
				memset(collector, 0, 5); /// no collector
			}
		} else {
			memset(collector, 0, 5); // ?
		}
	}
	sem_post(&pinfo->sem_db);
	return TRUE;
}

static int fgasmeter_addcollector_sub(const BYTE *address, BOOL lock) /// add sub collector in the planted tree(gasmeter_info or its pointer)
{
	int i, index = -1; /// supporting description
	GASMETER_INFO *pinfo = &gasmeter_info; ///
	COLLECTOR_DB *pdb; ///
	BOOL b_add_success = FALSE;

	if (address == NULL) /// check bottom condition
		return -1;
	if (lock) { /// need lock flag
		sem_wait(&pinfo->sem_db);
	}
	for (i = 0; i < MAX_GASMETER_NUMBER; i++) {
		pdb = &pinfo->db.collector_db[i];
		if (pdb->b_valid && memcmp(pdb->address, address, 5) == 0) { /// valid and address exists
			b_add_success = TRUE;
			index = i;
			break;
		}
	}
	if (!b_add_success) {
		for (i = 0; i < MAX_GASMETER_NUMBER; i++) {
			pdb = &pinfo->db.collector_db[i];
			if (!pdb->b_valid) {
				fgasmeter_init_collector(pdb); ///  if not valid, set collector info 0: rewrite it address
				memcpy(pdb->address, address, 5);
				pdb->b_valid = TRUE; /// change valid flag
				fgasmeter_update_collector(i, TRUE); ///?
				index = i;
				b_add_success = TRUE;
				break;
			}
		}
	}
	if (lock) {
		sem_post(&pinfo->sem_db); /// if lock, then post
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
	BOOL b_del_success = FALSE; /// set flag to indicate
	/// varieables district, interesting, we just complement the result varieable

	if (address == NULL)
		return FALSE;
	sem_wait(&pinfo->sem_db); /// set others waiting in multiple threads
	for (i = 0; i < MAX_GASMETER_NUMBER; i++) { /// tree-like logic and word-description operation
		pdb = &pinfo->db.collector_db[i];
		if (pdb->b_valid && memcmp(pdb->address, address, 5) == 0) {
			fgasmeter_init_collector(pdb);
			fgasmeter_update_collector(i, TRUE); /// make it valid and set its address as 0 // we need target, when you get target, then you can complement it
			b_del_success = TRUE;
			break;
		}
	}
	sem_post(&pinfo->sem_db);
	return b_del_success;
}

void start_read_gasmeter(void);
BOOL fgasmeter_addgasmeter(const BYTE *address, const BYTE *collector) /// add gas meter
{
	int i;
	GASMETER_INFO *pinfo = &gasmeter_info;
	GASMETER_DB *pdb;
	BOOL b_add_success = FALSE;

	if (address == NULL || collector == NULL) /// bottom check
		return FALSE;
	sem_wait(&pinfo->sem_db); /// this set, why need , more  logic(because one gasmeter_info) instinct
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
			if (!pdb->b_valid) { /// find a least number of invalid gasmeter and replace it with new gasmeter adress
				fgasmeter_init_gasmeter(pdb);
				memcpy(pdb->address, address, 7);
				pdb->collector_idx = fgasmeter_addcollector_sub(collector,
						FALSE); ///  allocate collector
				fgasmeter_update_collector(pdb->collector_idx, TRUE);
				pdb->b_valid = TRUE;
				fgasmeter_update_gasmeter(i, TRUE);
				b_add_success = TRUE;
				break;
			}
		}
	}
	sem_post(&pinfo->sem_db);
	if (b_add_success)
		start_read_gasmeter();

	return b_add_success;
}

BOOL fgasmeter_delgasmeter(const BYTE *address, const BYTE *collector) /// delete gas meter, why collector?
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

BOOL fgasmeter_setgasmeter_clock(int index, long tt) /// index and time(long)
{
	GASMETER_INFO *pinfo = &gasmeter_info;
	GASMETER_DB *pdb;

	if (index < 0 || index >= MAX_GASMETER_NUMBER) /// overlimit check
		return FALSE;
	sem_wait(&pinfo->sem_db);
	pdb = &pinfo->db.gasmeter_db[index];
	pdb->u.private.gasmeter_tt = tt;
	fgasmeter_update_gasmeter(index, TRUE); /// update index
	sem_post(&pinfo->sem_db);
	return TRUE;
}

long fgasmeter_getgasmeter_clock(int index) /// get
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

BOOL fgasmeter_setgasmeter_wakeupcycle(int index, WORD wakeupcycle) /// wake up cycle
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

BOOL fgasmeter_get_repeater(const BYTE *address, BYTE *repeater) /// gas meteraddress - repeater
{

	GASMETER_INFO *pinfo = &gasmeter_info;
	GASMETER_DB *pdb;
	int index = -1;

	////PRINTB("fgasmeter_get_repeater: ", address, 7);
	if (address == NULL || repeater == NULL)
		return FALSE;
	sem_wait(&pinfo->sem_db);
	for (index = 0; index < MAX_GASMETER_NUMBER; index++) {
		pdb = &pinfo->db.gasmeter_db[index];
		if (memcmp(address, pdb->address, 7) == 0
				&& pdb->u.private.b_repeater_valid) { // valid repeater
			memcpy(repeater, pdb->u.private.repeater, 2);
			break;
		}
	}
	sem_post(&pinfo->sem_db);
	return TRUE; /// return FALSE; /// return FALSE FOR NOW IT'S NO REPEATER NOW;
}

BOOL fgasmeter_set_repeater(const BYTE *address, const BYTE *repeater) /// meter repeater, repeater in gasmeter struct //// 设置表的中继是否可用
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
	GASMETER_DB *pdb;
	GASMETER_INFO *pinfo = &gasmeter_info;
	BYTE address[7], collector[5];

	ret = 0;
	for (idx = 0; idx < MAX_GASMETER_NUMBER; idx++) {
		if (fgasmeter_getgasmeter(idx, address, collector)) {
			ret++;
			///printf("%d meter: ", ret); /// printf("%d meter: ") ,,, 0 meter
			///PRINTB("which address is ", address, sizeof(address));///
		} else {
			continue;
		}
	}
	return ret;
}

void fgasmeter_assembly_reading(void) {
	int valid_meters, idx;
	GASMETER_DB *pdb;
	GASMETER_INFO *pinfo = &gasmeter_info;
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

int valid_meter_sum(void) { //// valid meter sum, if use fgasmeter_getgasmeter(idx, NULL, NULL)
	int ret;
	unsigned short idx;
	//BYTE address[7], collector[5];/// address = null, collector null

	ret = 0;
	for (idx = 0; idx < MAX_GASMETER_NUMBER; idx++) {
		if (fgasmeter_getgasmeter(idx, NULL, NULL)) {
			ret++;
		} else {
			; ///do nothing
		}
	}

	return ret;
}

void test_add_a_meter(void) {
	BYTE address[7] = { 0x23, 0x05, 0x17, 0x74, 0x00, 0x00, 0x05 };
	BYTE collector[7] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55 };

	fgasmeter_addgasmeter(address, collector);
	printf("add a meter successfully\n");
	return;
}

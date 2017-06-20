/*
 * f_param.c
 *
 *  Created on: 2015-8-11
 *      Author: Johnnyzhang
 */

#include "f_param.h"
#include "common.h"

#define CONCENTRATOR_ADDRESS "\x00\x00\x23\x01\x00\x00\x01"

static F_PARAM f_param;
static F_PARAM *p_f_param = &f_param;
static int fd_f_param;
static sem_t sem_f_param;

static int fparam_find(WORD id) {
	int i;
	struct param *ptr = p_f_param->params;

	for (i = 0; i < FPARAM_COUNT; i++) {
		if (ptr->id == id)
			return i;
		ptr++;
	}
	return -1;
}

static void fparam_update(int idx, int head_flag, int flush_flag) {
	struct param *ptr = p_f_param->params + idx;

	if (head_flag) {
		lseek(fd_f_param, offsetof(F_PARAM, params), SEEK_SET);
		safe_write(fd_f_param, p_f_param->params, sizeof(p_f_param->params));
	}
	lseek(fd_f_param, offsetof(F_PARAM, data) + ptr->offset, SEEK_SET);
	safe_write(fd_f_param, p_f_param->data + ptr->offset, ptr->len);
	if (flush_flag)
		fparam_flush();
}

static void fparam_lock(void) {
	sem_wait(&sem_f_param);
}

static void fparam_unlock(void) {
	sem_post(&sem_f_param);
}

static void fparam_add(int idx, WORD id, WORD len, WORD offset, const void *buf) {
	struct param *head = p_f_param->params + idx;
	unsigned char *data = p_f_param->data + offset;

	head->id = id;
	head->len = len;
	head->offset = offset;
	memcpy(data, buf, len);
	PRINTF("Add parameter of concentrator(ID: %04d)\n", id);
}

static void fparam_default_init(void) {
	int idx = 0;
	WORD offset = 0;

	const char *test_ip = "\x78\x19\x93\xE6"; /// 120.25.147.230
	const char *test_port = "\x30\x39"; /// 12345

	fparam_add(idx++, FPARAMID_CON_ADDRESS, 7, offset,
			CONCENTRATOR_ADDRESS);
	offset += 7;
	fparam_add(idx++, FPARAMID_COMM_HOST_IP_PRI, 4, offset, test_ip);
	offset += 4;
	fparam_add(idx++, FPARAMID_COMM_HOST_PORT_PRI, 2, offset, test_port);
	offset += 2;
	fparam_add(idx++, FPARAMID_COMM_HOST_IP_MINOR, 4, offset,
			"\x2D\x4E\x2A\xBD");
	offset += 4;
	fparam_add(idx++, FPARAMID_COMM_HOST_PORT_MINOR, 2, offset, "\x01\xBB");
	offset += 2;
	fparam_add(idx++, FPARAMID_HEARTBEAT_CYCLE, 2, offset, "\x78\x00");
	offset += 2;
	fparam_add(idx++, FPARAMID_READMETER_FREQ, 2, offset, "\x00\x21");
	offset += 2;
	fparam_add(idx++, FPARAMID_APN_ID, APN_LENGTH, offset, "UNINET");
	offset += APN_LENGTH;
	fparam_add(idx++, FPARAMID_APN_USER_ID, APN_LENGTH, offset, "CMNET");
	offset += APN_LENGTH;
	fparam_add(idx++, FPARAMID_APN_USER_PASSWD, APN_LENGTH, offset, "CMNET");
	offset += APN_LENGTH;
	fparam_add(idx++, FPARAMID_CON_VERIFY_PASSWD, 3, offset, "\x00\x00\x00");
	offset += 3;
	// TODO: repeater function
}

void fparam_init(void) {
	int size = sizeof(F_PARAM);
	const char *name = F_PARAM_NAME;

	sem_init(&sem_f_param, 0, 1);
	if (!check_file(name, size)) {
		PRINTF("File %s is created, size:%d\n", name, size);
		fd_f_param = open(name, O_CREAT | O_RDWR | O_TRUNC, 0600);
		fparam_default_init();
		safe_write(fd_f_param, p_f_param, size);
		fdatasync(fd_f_param);
		close(fd_f_param);
	}
	fd_f_param = open(name, O_RDWR);
	if (fd_f_param < 0)
		return;
	safe_read(fd_f_param, p_f_param, size);
}

void fparam_flush(void) {
	fdatasync(fd_f_param);
}

void fparam_destroy(void)
{
	fdatasync(fd_f_param);
	close(fd_f_param);
	sem_destroy(&sem_f_param);
}

WORD fparam_get_value(WORD id, void *buf, INT32 max_len) {
	int idx;
	struct param *ptr = p_f_param->params;

	fparam_lock();
	if ((idx = fparam_find(id)) >= 0) {
		ptr += idx;
		if (ptr->len <= max_len) {
			memcpy(buf, p_f_param->data + ptr->offset, ptr->len);
			fparam_unlock();
			return ptr->len;
		}
	}
	fparam_unlock();
	return 0;
}

WORD fparam_set_value(WORD id, const void *buf, INT32 len) {
	int idx;
	struct param *ptr = p_f_param->params;

	fparam_lock();
	if ((idx = fparam_find(id)) >= 0) {
		ptr += idx;
		if (ptr->len <= len) {
			if (memcmp(p_f_param->data + ptr->offset, buf, ptr->len) != 0) {
				memcpy(p_f_param->data + ptr->offset, buf, ptr->len);
				fparam_update(idx, 0, 1);
			}
			fparam_unlock();
			return ptr->len;
		}
	}
	fparam_unlock();
	return 0;
}

static int program_status;
static time_t program_valid_uptime;

void fparam_change_program_status(void) {
	program_status = (program_status ? 0 : 1);
	if (program_status) {
		program_valid_uptime = uptime();
	}
}

int fparam_get_program_status(void) {
	return program_status;
}

void fparam_update_program_status(void) {
	if (program_status) {
		if (uptime() - program_valid_uptime >= FPARAM_PROGRAM_INTERVAL) {
			program_status = 0;
		}
	}
}

int reset_fparam_data(void)
{
	int ret;
	int size = sizeof(F_PARAM);
	const char *name = F_PARAM_NAME;
	BYTE concentrator_address[7] = {0};

	fparam_get_value(FPARAMID_CON_ADDRESS, concentrator_address, 7);

	fparam_lock();
	if (remove(F_PARAM_NAME) == 0) {
		ret = 0;
	} else {
		ret = -1;
	}

	if (!check_file(name, size)) {
		PRINTF("File %s is created, size:%d\n", name, size);
		fd_f_param = open(name, O_CREAT | O_RDWR | O_TRUNC, 0600);
		fparam_default_init();
		safe_write(fd_f_param, p_f_param, size);
		fdatasync(fd_f_param);
		close(fd_f_param);
	}
	fd_f_param = open(name, O_RDWR);
	if (fd_f_param < 0)
		ret = -1;
	safe_read(fd_f_param, p_f_param, size);
	fparam_unlock();
	fparam_set_value(FPARAMID_CON_ADDRESS, concentrator_address, 7);

	return ret;
}

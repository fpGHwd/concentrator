/*
 * common.h
 *
 *  Created on: 2015-8-11
 *      Author: Johnnyzhang
 */

#ifndef COMMON_H_
#define COMMON_H_

#include "typedef.h"

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

#ifdef IMX28
#define LOG_NAME			"opt/concentrator/log/gascon.log"
#define ERR_NAME			"opt/concentrator/log/gascon.err"
#elif defined(AM335X)
#define LOG_NAME			"opt/concentrator/log/gascon.log"
#define ERR_NAME			"opt/concentrator/log/gascon.err"
#endif

#define GSTDEBUG_EN				1
#define GSTDEBUG_ALLOCATE_EN	1 
#define GSTASSERT_EN			1

#if GSTDEBUG_EN
#define GSTDEBUG(s) \
	do { \
		PRINTF("%sFile: %s, Function: %s, Line: %d\n", s == NULL ? "" : s, __FILE__, __FUNCTION__, __LINE__);\
	} while(0)
#define ASSERT(x) assert(x) // nayowang
#else
#define GSTDEBUG(s) do {;} while(0)
#define ASSERT(x) do{;}while(0) //nayowang
#endif

#if GSTDEBUG_ALLOCATE_EN
#define LOG_PRINTF_ALLOCATE(...) LOG_PRINTF(__VA_ARGS__)
#else
#define LOG_PRINTF_ALLOCATE(...) do {;} while(0)
#endif

/*
#if GSTASSERT_EN
#define ASSERT(x) assert(x) // nayowang
#else
#define ASSERT(x) do{;}while(0) //nayowang
#endif
*/

BYTE check_sum(const void *buf, int len);

void PRINTB(const char *name, const void *buf, int len);
void PRINTF(const char *format, ...);
void LOG_PRINTF(const char *format, ...);
void ERR_PRINTF(const char *format, ...);
void ERR_PRINTB(const char *prompt, short mtidx, const void *data, int data_len);

int wait_for_ready(int fd, int msec, int flag);
void wait_delay(int msec);

unsigned char bcd_to_bin(unsigned char val);
unsigned char bin_to_bcd(unsigned char val);
WORD ctos_be(const BYTE *buf);
void stoc_be(BYTE *buf, WORD val);
DWORD ctol_be(const BYTE *buf);
void ltoc_be(BYTE *buf, DWORD val);
WORD ctos(const BYTE *buf);
void stoc(BYTE *buf, WORD val);
DWORD ctol(const BYTE *buf);
void ltoc(BYTE *buf, DWORD val);
DWORD bcds_to_bin(const BYTE *buf, int len);
void bin_to_bcds(BYTE *buf, int len, DWORD val);
int bcd_ctos(const BYTE *buf, WORD *val); // Little endian
int bcd_ctol(const BYTE *buf, int *val);
void bcd_stoc(void *buf, WORD val);
void bcd_ltoc(void *buf, int val);

int bcd_be_ctos(const BYTE *buf, WORD *val); // Big endian
int bcd_be_ctol(const BYTE *buf, int *val);
void bcd_be_stoc(void *buf, WORD val);
///void bcd_be_stoc(BYTE *buf, WORD val);
void bcd_be_ltoc(BYTE *buf, int val);

int check_file(const char *name, int size);
int create_file(const char *name, int size, int log);

int safe_read_timeout(int fd, void *buf, int len, int timeout);
int safe_write_timeout(int fd, const void *buf, int len, int timeout);
int safe_read(int fd, void *buf, int len);
int safe_write(int fd, const void *buf, int len);

void sys_time(struct tm *tm);
long uptime(void);
void msleep(int msec);
unsigned long get_diff_ms(struct timeval * tv1, struct timeval * tv2);

void close_all(int from);
int find_pid(const char *prg_name);

void read_rtc(void);
void set_rtc(void);
int check_rtc(void);

int is_leap_year(int year);
void previous_day(BYTE *year, BYTE *month, BYTE *day);
void next_day(BYTE *year, BYTE *month, BYTE *day);
void previous_month(BYTE *year, BYTE *month);
void next_month(BYTE *year, BYTE *month);

void set_prog_name(const char *name);
void get_prog_name(char *name, int len);

void hexstr_to_str(void *dst, const void *src, int src_len);
void str_to_hexstr(void *dst, const void *src, int src_len);
const char *hex_to_str(char *strings, int maxlen, const BYTE *data, int len,
		BOOL b_reverse);

int get_network_addr(const char *interface, char *addr, char *dstaddr);

/// add by wd
bool if_year_month_str_is_valid(const char *year_month_str);
bool if_date_str_is_valid(const char *date_string);
unsigned int reverse_byte_array2bcd(BYTE *byte, int len);
unsigned int byte2bcd(BYTE byte);
void get_date(char *time_str);
int a_day_later(const char* data_string);
///extern inline int min_inline(int x, int y){ return ((x) <= (y)?(x):(y));} 

#endif /* COMMON_H_ */

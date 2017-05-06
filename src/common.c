/*
 * common.c
 *
 *  Created on: 2015-8-11
 *      Author: Johnnyzhang
 */

#include "common.h"
#include "main.h"
#include "threads.h"

unsigned char bcd_to_bin(unsigned char val) // bcd to binary // 0x23(bcd) -> 23(binary)
{
	return (val >> 4) * 10 + (val & 0x0f);
}

unsigned char bin_to_bcd(unsigned char val) /// 23 -> 0x23(35) // binary(23) to bcd (0x23)
{
	return ((val / 10) << 4) + (val % 10);
}

WORD ctos_be(const BYTE *buf) /// 0x01, 0x00 -> 256 (0~65525), char to string?
{
	return buf[1] + (buf[0] << 8); // buf[0] * 256 + buf[1]
}

void stoc_be(BYTE *buf, WORD val) // 256(val) -> 0x01, 0x00
{
	buf[1] = val; ///
	buf[0] = val >> 8; /// 2B(0~65525) -> 0x00 0x00 
}

DWORD ctol_be(const BYTE *buf) /// byte(4B) -> value
{
	return buf[3] + (buf[2] << 8) + (buf[1] << 16) + (buf[0] << 24);
}

void ltoc_be(BYTE *buf, DWORD val) /// long(DWORD, 4B) to char*( byte[0], byte[1], byte[2], byte[4]) 
{
	buf[3] = val;
	buf[2] = val >> 8;
	buf[1] = val >> 16;
	buf[0] = val >> 24;
}

WORD ctos(const BYTE *buf) /// char to short
{
	return buf[0] + (buf[1] << 8);
}

void stoc(BYTE *buf, WORD val) /// short(WORD, unsigend short, 0~65525) to char
{
	buf[0] = val; ///BYTE val[2] = 0x901F,
	buf[1] = val >> 8;
}

DWORD ctol(const BYTE *buf) /// char to long(double word)
{
	return buf[0] + (buf[1] << 8) + (buf[2] << 16) + (buf[3] << 24);
}

void ltoc(BYTE *buf, DWORD val)  /// long(double word, bin) to char
{
	buf[0] = val;
	buf[1] = val >> 8;
	buf[2] = val >> 16;
	buf[3] = val >> 24;
}

DWORD bcds_to_bin(const BYTE *buf, int len) /// byte to dword(value) /// triple -> double link
{
	DWORD val = 0;
	int i;

	if (len > 4) // len < 4
		return 0;
	for (i = 0; i < len; i++) {
		val += bcd_to_bin(buf[i]) * pow(100, i); /// 0x23, 0x05 ... -> 2305(int)
	}
	return val; /// 2305
}

void bin_to_bcds(BYTE *buf, int len, DWORD val) // bin to bcd
{
	int i;

	for (i = 0; i < min(len, sizeof(DWORD)); i++) { // 2305(unsigned short val, 0~65525)
		buf[i] = bin_to_bcd(val % 100); //  2305 -> 0x23, 0x05
		val /= 100;
	}
}

int bcd_ctos(const BYTE *buf, WORD *val) // buf(array) -> val(int 0~65525)
{
	const BYTE *ptr = buf;
	BYTE ch, high, low, str[4];
	int i;

	*val = 0; /// val = 0
	for (i = 0; i < 2; i++) {
		ch = *ptr++;
		high = (ch >> 4) & 0x0f;
		low = ch & 0x0f;
		if (high >= 10 || low >= 10)
			return 0;
		str[i] = (high * 10 + low);
	}
	*val = str[0] + str[1] * 100;
	return 1;
} // 0x23 0x05 -> 2505(unsigned short val) // bcd_ctos(bcd char to short)

int bcd_ctol(const BYTE *buf, int *val) // bcd_ctol(bcd char to long, 4B) 4,200,000,000
{
	const BYTE *ptr = buf;
	BYTE ch, high, low, str[4];
	int i;

	*val = 0;	// buf = 0x23, 0x05, 0x16, 0x06
	for (i = 0; i < 4; i++) {
		ch = *ptr++; // ch = 23
		high = (ch >> 4) & 0x0f; /// 0~15 // high = 2
		low = ch & 0x0f; // 0~15 // low = 3
		if (high >= 10 || low >= 10)
			return 0;
		str[i] = (high * 10 + low); // str[0] = 23 
	}
	*val = str[0] + str[1] * 100 + str[2] * 100 * 100
			+ str[3] * 100 * 100 * 100;
	return 1;
}

void bcd_stoc(void *buf, WORD val) /// short to char(bcd)
{
	BYTE ch, high, low, *ptr = (BYTE*)buf;
	int i;

	for (i = 0; i < 2; i++) { // i = 0 // i = 1 (unsigned shrot val = 2305; or WORD val = 2305)
		ch = val % 100; // ch = 05 // ch = 23
		high = ch / 10; // high = 0 // high = 2
		low = ch % 10; // low = 5 // low = 3
		*ptr++ = (high << 4) + low; /// *ptr++ = 0x05; // *ptr++ = 0x23
		val = val / 100;  /// val = 23 // val = 0
	}
}

void bcd_ltoc(void *buf, int val) /// long(4B) to char
{
	BYTE ch, high, low, *ptr = (BYTE*)buf; /// int val = 23051606
	int i;

	for (i = 0; i < 4; i++) { /// i = 0 // ..
		ch = val % 100; /// ch = 06 // .. ch = 16 // ch = 05 // ch = 23
		high = ch / 10; /// high = 0 // .. // high = 1 // high = 0 // high = 2
		low = ch % 10; /// low = 6 // .. // low = 6 // low = 5  // high = 3
		*ptr++ = (high << 4) + low; /// *ptr = 0x06 // .. // ptr = 0x16  // ptr = 0x05 // ptr = 0x23
		val = val / 100; // val = 230516 // .. // val = 2305 // val = 23 // val = 0
	}
}

int bcd_be_ctos(const BYTE *buf, WORD *val) // char(bcd) to short // what's be
{
	const BYTE *ptr = buf; // *buf = 0x23 0x05 ..
	BYTE ch, high, low, str[4];
	int i;

	*val = 0; // 
	for (i = 0; i < 2; i++) {
		ch = *ptr++; // ch = 0x23(35)
		high = (ch >> 4) & 0x0f;  // high = 2
		low = ch & 0x0f; // low = 3
		if (high >= 10 || low >= 10)
			return 0;
		str[i] = (high * 10 + low); // str[0] = 23, str[1] = 05
	}
	*val = str[1] + str[0] * 100; // unsigned short val = 2305
	return 1;
}

int bcd_be_ctol(const BYTE *buf, int *val) //// char to long(bcd)
{
	const BYTE *ptr = buf; /// *buf = 0x23 0x25 0x16 0x06; *buf = 0x00, 0x00, 0x00, 0x3F
	BYTE ch, high, low, str[4];
	int i;

	*val = 0;
	for (i = 0; i < 4; i++) { // i = 0
		ch = *ptr++; // ch = 23
		high = (ch >> 4) & 0x0f;
		low = ch & 0x0f;
		if (high >= 10 || low >= 10)
			return 0; /// (*buf)can not be saw as bcd, return
		str[i] = (high * 10 + low); // str[0] = 23 // str[1] = 05 // str[2] = 16 // str[3] = 06
	}
	*val = str[3] + str[2] * 100 + str[1] * 100 * 100
			+ str[0] * 100 * 100 * 100; // val = 23051606(unsigned short)
	return 1;
}

///
void bcd_be_stoc(void *buf, WORD val) /// val = 2004 /// bcd, short to char
{
	BYTE ch[2], high, low, *ptr = (BYTE*)buf; /// val = 2305
	int i;

	for (i = 0; i < 2; i++) {
		ch[i] = val % 100; /// ch[0] = 4; ch[1] = 20 /// ch[0] = 05 // ch[1] = 23
		val = val / 100;	/// val = 20;  val = 20 /// val = 23 // val = 0
	}
	for (i = 0; i < 2; i++) {
		high = ch[1 - i] / 10; /// high = 2; high = 0 /// high = 2,  /// high = 0
		low = ch[1 - i] % 10; /// low = 0; low = 4 /// low = 3, /// low = 5
		*ptr++ = (high << 4) + low; /// *ptr++ = 0x20, ptr++ = 0x04;  + 0x20 0x04 /// ptr = 23, ptr = 05
	}
}

void bcd_be_ltoc(BYTE *buf, int val) /// long(4B, val) to char(buf, BYTE*)
{
	BYTE ch[4], high, low, *ptr = buf;
	int i;

	for (i = 0; i < 4; i++) {
		ch[i] = val % 100;
		val = val / 100;
	}
	for (i = 0; i < 4; i++) {
		high = ch[3 - i] / 10;
		low = ch[3 - i] % 10;
		*ptr++ = (high << 4) + low;
	}
}

BYTE check_sum(const void *buf, int len)
{
	const BYTE *ptr = buf;
	BYTE sum = 0;

	while (len-- > 0)
		sum += *ptr++;
	return sum;
}

int wait_for_ready(int fd, int msec, int flag)
{
	int ret;
	fd_set fds;
	struct timeval tv;

	while (!g_terminated) {
		tv.tv_sec = 0;
		tv.tv_usec = msec * 1000;
		if (fd < 0) {
			ret = select(0, NULL, NULL, NULL, &tv);
		} else {
			FD_ZERO(&fds);
			FD_SET(fd, &fds);
			if (!flag)
				ret = select(fd + 1, &fds, NULL, NULL, &tv); /// check read status
			else
				ret = select(fd + 1, NULL, &fds, NULL, &tv); /// check write
		}
		if (ret < 0 && errno == EINTR)
			continue;
		return ret;
	}
	return 0;
}

int safe_read_timeout(int fd, void *buf, int len, int timeout) {
	int ret;
	void *ptr = buf;

	while (len > 0 && wait_for_ready(fd, timeout, 0) > 0) {
		notify_watchdog();
		ret = read(fd, ptr, len);
		if (ret < 0 && errno == EINTR)
			continue;
		if (ret < 0)
			return ret;
		if (ret == 0)
			break;
		ptr += ret;
		len -= ret;
	}
	return ptr - buf;
}

int safe_write_timeout(int fd, const void *buf, int len, int timeout) {
	int ret;
	const void *ptr = buf;

	while (len > 0 && wait_for_ready(fd, timeout, 1) > 0) {
		notify_watchdog();
		ret = write(fd, ptr, len); /// ptr
		if (ret < 0 && errno == EINTR)
			continue;
		if (ret < 0)
			return ret;
		if (ret == 0)
			break;
		ptr += ret;
		len -= ret;
	}
	return ptr - buf;
}

int safe_read(int fd, void *buf, int len) {
	int ret;
	void *ptr = buf;

	while (len > 0) {
		ret = read(fd, ptr, len);
		if (ret < 0 && errno == EINTR)
			continue;
		if (ret < 0)
			return ret;
		if (ret == 0)
			break;
		ptr += ret;
		len -= ret;
	}
	return ptr - buf;
}

int safe_write(int fd, const void *buf, int len) /// save p_buf to fd
{
	int ret;
	const void *ptr = buf;

	while (len > 0) {
		ret = write(fd, ptr, len);
		if (ret < 0 && errno == EINTR)
			continue;
		if (ret < 0)
			return ret;
		if (ret == 0)
			break;
		ptr += ret;
		len -= ret;
	}
	return ptr - buf;
}

int check_file(const char *name, int size) {
	struct stat buf;
	return stat(name, &buf) == 0 && buf.st_size == size;
}

int create_file(const char *name, int size, int log) {
	int fd, buf_size;
	char buf[1024 * 100];

	if (log)
		LOG_PRINTF("Create file %s, size:%d\n", name, size);
	if ((fd = open(name, O_CREAT | O_RDWR | O_TRUNC, 0600)) >= 0) {
		buf_size = sizeof(buf);
		memset(buf, 0, buf_size);
		while (size > 0) {
			safe_write(fd, buf, min(size, buf_size));
			size -= buf_size;
		}
		fdatasync(fd);
		close(fd);
	}
	return fd >= 0;
}

static void print_head(FILE *fp) {
	struct tm tm;
	struct timeval tv;
	char buf[1024];
	int len;
	const char *thread_name = NULL;

	gettimeofday(&tv, NULL);
	localtime_r(&tv.tv_sec, &tm);
	thread_name = get_thread_name(); /// how can you comment this sentence
	len = snprintf(buf, sizeof(buf),
			"[%04d-%02d-%02d %02d:%02d:%02d.%03ld] %s\t", tm.tm_year + 1900,
			tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec,
			tv.tv_usec / 1000, thread_name ? thread_name : "main\t");
	if (len > 0) {
		buf[len] = 0x0;
		fprintf(fp, "%s", buf);
	}
}

static int stdout_error(void) {
	static int stdout_err = 0;
	int ret, fd, usec;
	fd_set fds;
	struct timeval tv;

	if (stdout_err == 0) {
		fd = 1;
		usec = 1000 * 1000 * 60 * 4;
		do {
			tv.tv_sec = 0;
			tv.tv_usec = usec;
			FD_ZERO(&fds);
			FD_SET(fd, &fds);
			ret = select(fd + 1, NULL, &fds, NULL, &tv);
		} while (ret < 0 && errno == EINTR);
		if (ret <= 0)
			stdout_err = 1;
	}
	return stdout_err;
}

static void do_truncate(const char *name, int delta) {
	int fd, size, new_size, sys_ret;
	char *buf;

	if ((fd = open(name, O_RDWR, 0600)) >= 0) {
		sys_ret = lockf(fd, F_LOCK, 0L); /// lock fd
		size = lseek(fd, 0L, SEEK_END) - delta; /// locate end - offset_int
		if (size > 0 && (buf = malloc(size)) != NULL) {
			lseek(fd, delta, SEEK_SET); /// ? should be size
			new_size = safe_read(fd, buf, size); /// 
			if (new_size > 0) {
				lseek(fd, 0L, SEEK_SET);
				safe_write(fd, buf, new_size);
				sys_ret = ftruncate(fd, new_size);
			}
			free(buf);
		}
		lseek(fd, 0L, SEEK_SET);
		sys_ret = lockf(fd, F_ULOCK, 0L);
		close(fd);
	}
}

static void do_append(const char *name, const char *data, int data_len) {
	int fd, buf_len, sys_ret;
	char buf[256];
	struct tm tm;
	struct timeval tv;

	if ((fd = open(name, O_RDWR | O_CREAT, 0600)) >= 0) {
		gettimeofday(&tv, NULL);
		localtime_r(&tv.tv_sec, &tm);
		buf_len = snprintf(buf, sizeof(buf), "[%04d-%02d-%02d %02d:%02d] ",
				tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour,
				tm.tm_min);
		sys_ret = lockf(fd, F_LOCK, 0L);
		lseek(fd, 0L, SEEK_END); /// end + 0
		sys_ret = write(fd, buf, buf_len);
		sys_ret = write(fd, data, data_len);
		sys_ret = lockf(fd, F_ULOCK, 0L);
		close(fd);
	}
}

static void do_log(const char *log_file, int log_max, int log_delta,
		const char *data, int data_len) {
	struct stat st;
	int prompt = 24;

	if (stat(log_file, &st) == 0 && st.st_size + prompt + data_len > log_max) {
		log_delta = st.st_size + prompt + data_len - (log_max - log_delta);
		do_truncate(log_file, log_delta);
	}
	do_append(log_file, data, data_len);
}

void PRINTB(const char *name, const void *data, int data_len) /// printf bytes...
{
	int i, j;
	const unsigned char *ptr = data;

	if (g_silent || data_len <= 0 || stdout_error())
		return;
	print_head(stdout);
	printf("%s[%d Byte%s]\n\t", name, data_len, data_len > 1 ? "s" : "");
	j = 0;
	for (i = 0; i < data_len; i++) {
		printf("%02X ", ptr[i]);
		j++;
		if (j == 16 && i < data_len - 1) {
			printf("\n\t");
			notify_watchdog();
			j = 0;
		}
	}
	printf("\n");
}

void PRINTF(const char *format, ...) {
	va_list ap;

	if (g_silent || stdout_error())
		return;
	print_head(stdout);
	va_start(ap, format);
	vprintf(format, ap);
	va_end(ap);
}

#define ERR_LOG_SIZE	(40 * 1024)
#define ERR_STEP_SIZE	(1 * 1024)

void ERR_PRINTB(const char *prompt, short mtidx, const void *data, int data_len) {
	char buf[4 * 1024], *ptr;
	const char *tmp;
	int i, len;
	BYTE val;

	if (data_len > 0) {
		if (data_len > 1024)
			data_len = 1024;
		len = snprintf(buf, sizeof(buf), "%s, mtidx:%d, data:", prompt, mtidx);
		ptr = buf + len;
		tmp = data;
		for (i = 0; i < data_len; i++) {
			val = *tmp++;
			snprintf(ptr, 4, "%02x ", val);
			ptr += 3;
		}
		*ptr++ = '\n';
		*ptr = 0x0;
		len = ptr - buf;
		do_log(ERR_NAME, ERR_LOG_SIZE, ERR_STEP_SIZE, buf, len);
	}
}

void ERR_PRINTF(const char *format, ...) {
	va_list ap;
	char buf[1024];
	int len;

	va_start(ap, format);
	len = vsnprintf(buf, sizeof(buf), format, ap);
	do_log(ERR_NAME, ERR_LOG_SIZE, ERR_STEP_SIZE, buf, len);
	va_end(ap);
	PRINTF("%s", buf);
}

void LOG_PRINTF(const char *format, ...) {
	va_list ap;
	char buf[1024];
	int len;

	va_start(ap, format);
	if (!g_silent && !stdout_error()) {
		print_head(stdout);
		vprintf(format, ap);
	}
	va_end(ap);

	va_start(ap, format);
	len = vsnprintf(buf, sizeof(buf), format, ap);
	do_log(LOG_NAME, 1024 * 512, 10 * 1024, buf, len);
	va_end(ap);
}

long uptime(void) {
	struct sysinfo info;

	sysinfo(&info);
	return info.uptime; /* Seconds since boot */
}

void close_all(int from) {
	int i;

	for (i = from; i < 1024; i++)
		close(i);
}

int find_pid(const char *prg_name) /// find pid /// it's useful
{
	DIR *dir;
	struct dirent *entry;
	char status[32], buf[1024], *name;
	int fd, len, pid, pid_found;

	pid_found = 0;
	dir = opendir("/proc");
	while ((entry = readdir(dir)) != NULL) {
		name = entry->d_name;
		if (!(*name >= '0' && *name <= '9'))
			continue;
		pid = atoi(name);
		sprintf(status, "/proc/%d/stat", pid);
		if ((fd = open(status, O_RDONLY)) < 0)
			continue;
		len = safe_read(fd, buf, sizeof(buf) - 1);
		close(fd);
		if (len <= 0)
			continue;
		buf[len] = 0x0;
		name = strrchr(buf, ')');
		if (name == NULL || name[1] != ' ')
			continue;
		*name = 0;
		name = strrchr(buf, '(');
		if (name == NULL)
			continue;
		if (strncmp(name + 1, prg_name, 16 - 1) == 0) {
			pid_found = pid;
			break;
		}
	}
	closedir(dir);
	return pid_found;
}

void msleep(int msec) {
	int ret;
	struct timeval tv;

	tv.tv_sec = 0;
	tv.tv_usec = msec * 1000;
	do {
		ret = select(0, NULL, NULL, NULL, &tv);
	} while (ret < 0 && errno == EINTR);
}

// read hardware clock
void read_rtc(void) {
	int sys_ret;
	sys_ret = system("/sbin/hwclock -su");
}

// set hardware clock
void set_rtc(void) {
	int sys_ret;
	sys_ret = system("/sbin/hwclock -wu");

}

int check_rtc(void) {
	int fd, ret = 0;
	struct tm tm;

	if ((fd = open("/dev/rtc0", O_RDONLY)) >= 0) {
		PRINTF("Open /dev/rtc0 success\n");
		if (ioctl(fd, RTC_RD_TIME, &tm) >= 0)
			ret = 1;
		close(fd);
	} else {
		PRINTF("Error in open clock\n");
	}
	return ret;
}

void wait_delay(int msec) /// useful function
{
	int tmp;

	while (!g_terminated && msec > 0) {
		tmp = (msec > 1000) ? 1000 : msec;
		msleep(tmp);
		msec -= tmp;
	}
}

void sys_time(struct tm *tm) /// read current time by struct tm
{
	time_t tt;

	time(&tt);
	localtime_r(&tt, tm);
}

int is_leap_year(int year) {
	return ((year % 4) == 0 && (year % 100) != 0) || (year % 400) == 0;
}

void previous_day(BYTE *year, BYTE *month, BYTE *day) {
	static const BYTE month_day[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31,
			30, 31 };
	BYTE max_day;

	if (*day == 1) {
		if (*month == 1) {
			*month = 12;
			if (*year == 0)
				*year = 99;
			else
				*year = *year - 1;
		} else {
			*month = *month - 1;
		}
		max_day = month_day[*month - 1];
		if (*month == 2 && is_leap_year(*year + 2000))
			max_day++;
		*day = max_day;
	} else {
		*day = *day - 1;
	}
}

void next_day(BYTE *year, BYTE *month, BYTE *day) /// next day...
{
	static const BYTE month_day[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31,
			30, 31 };
	BYTE max_day;

	max_day = month_day[*month - 1];
	if (*month == 2 && is_leap_year(*year + 2000))
		max_day++;
	if (*day == max_day) {
		*day = 1;
		if (*month == 12) {
			*month = 1;
			*year = *year + 1;
		} else {
			*month = *month + 1;
		}
	} else {
		*day = *day + 1;
	}
}

void previous_month(BYTE *year, BYTE *month) {
	if (*month == 1) {
		*month = 12;
		*year = *year - 1;
	} else {
		*month = *month - 1;
	}
}

void next_month(BYTE *year, BYTE *month) {
	if (*month == 12) {
		*month = 1;
		*year = *year + 1;
	} else {
		*month = *month + 1;
	}
}

static char prog[PATH_MAX]; /// PATH_MAX
void set_prog_name(const char *name) /// return the canonicalized absolute pathname
{
	char *sys_ptr;

	sys_ptr = realpath(name, prog);
}

void get_prog_name(char *name, int len) // len stands for the length of name
{
	strncpy(name, prog, len - 1); //
}

void hexstr_to_str(void *dst, const void *src, int src_len) /// buff, 23051606000102, 14
{
	int i;
	BYTE low, high, ch;
	BYTE *dst_ptr = dst;
	const BYTE *src_ptr = src;

	for (i = 0; i < src_len / 2; i++) { //// 23 05 16 06 00 01 02 /// src_len = 7
		ch = *src_ptr++; /// 
		if (ch >= '0' && ch <= '9')
			high = ch - '0';
		else if (ch >= 'A' && ch <= 'F')
			high = ch - 'A' + 10;
		else if (ch >= 'a' && ch <= 'f')
			high = ch - 'a' + 10;
		else
			break;
		ch = *src_ptr++;
		if (ch >= '0' && ch <= '9')
			low = ch - '0';
		else if (ch >= 'A' && ch <= 'F')
			low = ch - 'A' + 10;
		else if (ch >= 'a' && ch <= 'f')
			low = ch - 'a' + 10;
		else
			break;
		*dst_ptr++ = high * 16 + low; //
	}
}

void str_to_hexstr(void *dst, const void *src, int src_len) ///
{
	int i;
	BYTE low, high, ch;
	BYTE *dst_ptr = dst;
	const BYTE *src_ptr = src;
	const char *str = "0123456789ABCDEF";

	for (i = 0; i < src_len; i++) {
		ch = *src_ptr++;
		high = (ch >> 4) & 0x0f;
		low = ch & 0x0f;
		*dst_ptr++ = str[high];
		*dst_ptr++ = str[low];
	}
}

const char *hex_to_str(char *strings, int maxlen, const BYTE *data, int len,
		BOOL b_reverse) {
	int i;

	if (strings == NULL || data == NULL)
		return "NULL";
	strings[0] = 0;
	for (i = 0; i < len; i++) {
		if (b_reverse) {
			snprintf(strings + strlen(strings), maxlen - strlen(strings),
					"%02X", data[len - i - 1]);
		} else {
			snprintf(strings + strlen(strings), maxlen - strlen(strings),
					"%02X", data[i]);
		}
	}
	return strings;
}

unsigned long get_diff_ms(struct timeval * tv1, struct timeval * tv2) {
	return (tv1->tv_sec - tv2->tv_sec) * 1000
			+ (tv1->tv_usec - tv2->tv_usec) / 1000;
}

int get_network_addr(const char *interface, char *addr, char *dstaddr) /// 获取socket的相关结构体的参数
{
	int i, fd, ret = 0;
	char buf[1024];
	struct ifconf ifconf;
	struct ifreq *ifreq;
	struct sockaddr_in *in_addr; /// <netinet/in.h> 

	fd = socket(AF_INET, SOCK_DGRAM, 0); /// fixed-length, connectionless, unreliable messages
	ifconf.ifc_buf = buf; /// buffer to ifconfig
	ifconf.ifc_len = sizeof(buf);
	if (ioctl(fd, SIOCGIFCONF, &ifconf) == 0) { /// SIOCGIFCONF: get list of all interfaces
		ifreq = ifconf.ifc_req; /// request
		for (i = 0; i < ifconf.ifc_len / sizeof(struct ifreq); i++) {
			if (!strncmp(ifreq->ifr_name, interface, strlen(interface))) { /// if sf
				if (ioctl(fd, SIOCGIFFLAGS, ifreq) == 0 /// SIOCGIFFLAGS: get interface flags	
				&& (ifreq->ifr_flags & IFF_UP)) {
					in_addr = (struct sockaddr_in *) &ifreq->ifr_addr; /// ifrequest address
					if (ioctl(fd, SIOCGIFDSTADDR, ifreq) == 0)
						///inet_ntop(AF_INET, &in_addr->sin_addr, dstaddr, INET_ADDRSTRLEN); /// INET_ADDRSTRLEN // ipv4
						inet_ntop(AF_INET, &in_addr->sin_addr, dstaddr, 16); /// The inet_ntop function converts a binary address in network byte order into a text string;
					if (ioctl(fd, SIOCGIFADDR, ifreq) == 0) {
						///inet_ntop(AF_INET, &in_addr->sin_addr, addr, INET_ADDRSTRLEN); /// INET6_ADDRSTRLEN
						inet_ntop(AF_INET, &in_addr->sin_addr, addr, 16);
						ret = 1;
					}
				}
				break;
			}
			ifreq++;
		}
	}
	close(fd);
	return ret;
}

bool if_year_month_str_is_valid(const char *year_month_str) {
	int i;
	//time_t tt;
	//struct tm t_m;
	//char buff[6 + 1];
	int month, year;

	i = atoi(year_month_str);

	year = i / 100;
	month = i % 100;

	if (year <= 1900)
		return false;

	if (month > 12)
		return false;

	return true;
}

bool if_date_str_is_valid(const char *date_string) /// "20161130"
{
	int year, month, day, i;

	i = atoi(date_string);
	year = i / 10000;
	month = (i % 10000) / 100;
	day = i % 100;

	if (month == 0 || day == 0)
		return false;

	if (year >= 1900) {
		switch (month) {
		case 1:
		case 3:
		case 5:
		case 7:
		case 8:
		case 10:
		case 12:
			if (day <= 31) {
				return true;
			} else {
				return false;
			}
		case 4:
		case 6:
		case 9:
		case 11:
			if (day <= 30) {
				return true;
			} else {
				return false;
			}
		case 2:
			if ((year % 400 == 0) || ((year % 100 != 0) && (year % 4 == 0))) {
				if (day <= 29) {
					return true;
				} else {
					return false;
				}
			} else {
				if (day <= 28) {
					return true;
				} else {
					return false;
				}
			}
		default:
			return false;
		}
	} else { //本身的时间结构合理性
		return false;
	}
}

unsigned int byte2bcd(BYTE byte) {
	int high;
	int low;

	high = ((byte & 0xF0) >> 4);
	low = byte & 0x0F;
	return high * 10 + low;
}

unsigned int reverse_byte_array2bcd(BYTE *byte, int len) //// n bytes(len)
{
	int ret, ret_byte;
	int i, j;

	ret = 0;
	for (i = 0; i < len; i++) {
		ret_byte = byte2bcd(byte[i]);
		for (j = 0; j < i; j++) {
			ret_byte = ret_byte * 100;
			//printf("ret_byte = %d\n",ret_byte);
		}
		ret += ret_byte;
	}

	return ret;
}

void get_date(char *time_str) {
	char buff[10 + 1];
	time_t tt;
	struct tm tm;

	time(&tt);
	localtime_r(&tt, &tm);
	snprintf(buff, sizeof(buff), "%04d-%02d-%02d", tm.tm_year + 1900,
			tm.tm_mon + 1, tm.tm_mday); /// 2016-12-14

	memcpy(time_str, buff, sizeof(buff));

	return;
}

int a_day_later(const char* data_string) /// void next_day(BYTE * year, BYTE * month, BYTE * day)
{
	int i, year, month, day;

	i = atoi(data_string);
	year = i / 10000;
	month = i % 10000 / 100;
	day = i % 100;

	switch (month) {
	case 1:
	case 3:
	case 5:
	case 7:
	case 8:
	case 10:
		if (day == 31) {
			month++;
			day = 1;
		} else {
			day++;
		}
		break;
	case 12:
		if (day == 31) {  /// 20161231
			year++;
			month = 1;
			day = 1;
		} else {
			day++;
		}
		break;
	case 2:
		if ((year % 400 == 0) || ((year % 100 != 0) && (year % 4 == 0))) {
			if (day == 29) {
				month++;
				day = 1;
			} else if (day == 28) {
				month++;
				day = 1;
			}
		} else {
			day++;
		}
		break;
	case 4:
	case 6:
	case 9:
	case 11:
		if (day == 30) {
			month++;
			day = 1;
		} else {
			day++;
		}
		break;
	default:
		return 0; // make sure that input time is no problem
	}
	return year * 10000 + month * 100 + day;
}

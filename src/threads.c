/*
 * threads.c
 *
 *  Created on: 2015-8-11
 *      Author: Johnnyzhang
 */

#include "threads.h"
#include "common.h"
#include "main.h"

#include <linux/watchdog.h>
#include "devices.h"

#define SIGWATCHDOG		SIGRTMIN

int watchdog_fd; /// wd

static int pid_watchdog = 0;

typedef void * (*FUNC)(void *arg);

static pthread_t th[THREADS_COUNT];

static FUNC func[THREADS_COUNT] = { th_upgprscdma, th_upeth, th_downcomm,
		th_hmisys, th_gasmeter, th_gasmeter_event, th_alarm, };

static const char *thread_name[THREADS_COUNT] = { "th_upgprscdma", "th_upeth",
		"th_downcomm", "th_hmisys", "th_gasmeter", "th_gasmeter_event",
		"th_alarm", };

void threads_create(void)
{
	int i;

#if 0 // for test the gprscdma thread
	pthread_create(&th[0], NULL, func[0], NULL);
	return;
#endif


	for (i = 0; i < THREADS_COUNT; i ++) {
		pthread_create(&th[i], NULL, func[i], NULL);
		PRINTF("Create thread %s\n", thread_name[i]);
	}

}

void threads_join(void) {
	int i;


#if 0 // for test the gprscdma thread
	pthread_join(th[0], NULL);
	PRINTF("Exiting thread %s\n", thread_name[0]);
	return;
#else

	for (i = 0; i < THREADS_COUNT; i++) {
		pthread_join(th[THREADS_COUNT - 1 - i], NULL);
		PRINTF("Exiting thread %s\n", thread_name[THREADS_COUNT - 1 - i]);
	}

#endif
}

int which_thread(void) {
	int i;
	pthread_t self;

	self = pthread_self();
	for (i = 0; i < THREADS_COUNT; i++) {
		if (pthread_equal(self, th[i]))
			return i;
	}
	return 0;
}

void init_watchdog(void) {
	int interval = 60, timeout;
	int bootstatus;
	int pid_watchdog = -1;

	pid_watchdog = find_pid("watchdog");
	if(pid_watchdog > 0)
		PRINTF("watchdog pid = %d\n", pid_watchdog);

	watchdog_fd = open(watch_dog_device, O_RDWR);
	if (watchdog_fd > 0){
		fprintf(stdout, "Open watchdog successed\n");
	}
	else{
		fprintf(stdout, "Open watchdog failed\n");
	}

	// get watch dog seconds
	ioctl(watchdog_fd, WDIOC_GETTIMEOUT, &timeout);
	if(timeout < 60)
		PRINTF("the timeout of wactchdog is less than 60 seconds, set it 60 senconds\n");
		if(ioctl(watchdog_fd, WDIOC_SETTIMEOUT, &interval) != 0){
			fprintf(stderr, "Error: Set watchdog interval failed\n");
			//exit(EXIT_FAILURE);
		}
	else{
		PRINTF("the timeout of wactchdog is not less than 60 seconds\n");
	}

	if (ioctl(watchdog_fd, WDIOC_GETTIMEOUT, &interval) == 0) {
		//fprintf(stdout, "Current watchdog interval is %d\n", interval);
		if(interval != 60)
			fprintf(stderr, "Error: set watchdog interval failed\n");
		else
			; // nothing exceptional, go on
	} else {
		fprintf(stderr, "Error: Cannot read watchdog interval\n");
		exit(EXIT_FAILURE);
	}

	if (ioctl(watchdog_fd, WDIOC_GETBOOTSTATUS, &bootstatus) == 0) {
		//fprintf(stdout, "Last boot is caused by : %s\n",(bootstatus != 0) ? "Watchdog" : "Power-On-Reset");
	} else {
		fprintf(stderr, "Error: Cannot read watchdog status\n");
		exit(EXIT_FAILURE);
	}

	return;
}

int kill_watchdog(void) {
	if (pid_watchdog > 0) {
		kill(pid_watchdog, SIGTERM);
		PRINTF("Close watchdoh\n");
		return 1;
	}
	/*
	 *  close watchdog in tutorial in loT-A28LI
	 */
	/*
	write(watchdog_fd, "V", 1);
	close(watchdog_fd);
	*/
	return 0;
}


#include "main.h"
void notify_watchdog(void) {
	if(debug_ctrl.watchdog_enable)
		ioctl(watchdog_fd, WDIOC_KEEPALIVE, NULL);

#if 0
	int i;
	pthread_t self;
	/*static long last_uptime[THREADS_COUNT];
	 struct tm last_tm, now_tm;
	 static struct timeval last_tv[THREADS_COUNT];
	 struct timeval now_tv;*/

	if (pid_watchdog > 0) {
		self = pthread_self();
		for (i = 0; i < THREADS_COUNT; i ++) {
			if (pthread_equal(self, th[i])) {
				/*if (uptime() - last_uptime[i] >= 5) {
				 gettimeofday(&now_tv, NULL);
				 localtime_r(&now_tv.tv_sec, &now_tm);
				 if (!g_silent) {
				 printf("%02d-%02d %02d:%02d:%02d.%03ld %s\t", now_tm.tm_mon + 1, now_tm.tm_mday,
				 now_tm.tm_hour, now_tm.tm_min, now_tm.tm_sec, now_tv.tv_usec / 1000,
				 thread_name[i]);
				 localtime_r(&last_tv[i].tv_sec, &last_tm);
				 printf("%s: thread index: %d, \n\t\t\t\tlast tm: %04d-%02d-%02d %02d:%02d:%02d:%03ld, sig: %d\n",
				 __func__, i, last_tm.tm_year + 1900, last_tm.tm_mon + 1,
				 last_tm.tm_mday, last_tm.tm_hour, last_tm.tm_min, last_tm.tm_sec,
				 last_tv[i].tv_usec / 1000, SIGWATCHDOG + i);
				 }
				 if (uptime() - last_uptime[i] > 60) {
				 if (!g_silent) {
				 printf("%02d-%02d %02d:%02d:%02d.%03ld %s\t", now_tm.tm_mon + 1, now_tm.tm_mday,
				 now_tm.tm_hour, now_tm.tm_min, now_tm.tm_sec, now_tv.tv_usec / 1000,
				 thread_name[i]);
				 printf("Warning in %s for over 1 min, \n\t\t\t\tlast tm: %04d-%02d-%02d " \
									"%02d:%02d:%02d:%03ld, sig: %d\n", __func__,
				 last_tm.tm_year + 1900, last_tm.tm_mon + 1,	last_tm.tm_mday,
				 last_tm.tm_hour, last_tm.tm_min, last_tm.tm_sec,
				 last_tv[i].tv_usec / 1000, SIGWATCHDOG + i);
				 }
				 }
				 last_uptime[i] = uptime();
				 }*/
				kill(pid_watchdog, SIGWATCHDOG + i);
				//gettimeofday(&last_tv[i], NULL);
				break;
			}
		}
	}
#endif
}

const char *get_thread_name(void) {
	int i;
	pthread_t self;

	self = pthread_self();
	for (i = 0; i < THREADS_COUNT; i++) {
		if (pthread_equal(self, th[i])) {
			return thread_name[i];
		}
	}
	return NULL;
}

void print_thread_info(void) {
	INT32 i;
	pthread_t self;

	self = pthread_self();
	for (i = 0; i < THREADS_COUNT; i++) {
		if (pthread_equal(self, th[i])) {
			PRINTF("Thread name: %s\n", thread_name[i]);
		}
	}
}

bool check_thread(char *th_name) {
	pthread_t self;
	int i;

	self = pthread_self();
	for (i = 0; i < THREADS_COUNT; i++) {
		if (pthread_equal(self, th[i])) {
			if (strcmp(thread_name[i], th_name) == 0) {
				// success or match
				return TRUE;
			}
		}
	}
	return FALSE;
}

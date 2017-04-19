/*
 * typedef.h
 *
 *  Created on: 2015-8-11
 *      Author: Johnnyzhang
 */

#ifndef TYPEDEF_H_
#define TYPEDEF_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>
#include <stdarg.h>
#include <sys/ioctl.h>
#include <linux/rtc.h>
#include <limits.h>
#include <dirent.h>
#include <semaphore.h>
#include <sys/time.h>
#include <time.h>
#include <signal.h>
#include <sys/wait.h>
#include <string.h>
#include <math.h>
#include <sys/sysinfo.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stddef.h>
#include <linux/if.h>

// add by wd 
#include <stdbool.h> /// bool.h

typedef unsigned char BITS; /// 1B /// 最小寻址单位一个字节
typedef unsigned char BOOL; /// 1B
typedef unsigned char BYTE; /// 1B
typedef unsigned short WORD; /// 2B
typedef unsigned int DWORD; /// 4B

typedef unsigned char UINT8; /// 1B
typedef unsigned short UINT16; /// 2B
typedef unsigned int UINT32; /// 4B

typedef signed char STRING; /// 1B
typedef char INT8; ///1B
typedef signed short INT16; ///2B
typedef int INT32; ///4B
typedef long long INT64; ///16B

#ifndef min
#define min(x,y)	((x)<=(y)?(x):(y))
#define max(x,y)	((x)>=(y)?(x):(y))
#endif

#ifndef FALSE /// complement
#define	FALSE		0
#define TRUE		1
#endif

#endif /* TYPEDEF_H_ */


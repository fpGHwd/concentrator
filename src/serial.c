/*
 * serial.c
 *
 *  Created on: 2015-8-11
 *      Author: Johnnyzhang
 */

#include "serial.h"
#include "common.h"

int open_serial(const char * serial, int baud, int char_size, int parity) {
	int fd;
	struct termios newtios;

	if ((fd = open(serial, O_RDWR | O_NDELAY | O_NOCTTY)) > 0) {
		fcntl(fd, F_SETFL, O_RDWR);
		PRINTF("Open %s, fd: %d, baud: %d, data bits: %d.\n", serial, fd, baud,
				char_size);
		tcgetattr(fd, &newtios);
		cfmakeraw(&newtios);
		if (char_size == 7)
			newtios.c_cflag = (newtios.c_cflag & ~CSIZE) | (CS7);
		newtios.c_cflag |= parity;
		tcsetattr(fd, TCSANOW, &newtios);
		tcflush(fd, TCIOFLUSH);
		set_baudrate(fd, baud);
	} else {
		PRINTF("Can not open %s, baud: %d, data bits: %d.\n", serial, baud,
				char_size);
		return -1;
	}
	return fd;
}

void close_serial(int fd) {
	if (fd >= 0) {
		/// PRINTF("Close the serial port, whose fd is %d\n", fd);
		tcflush(fd, TCIOFLUSH);
		close(fd);
	} else {
		PRINTF("Trying to close an INVALID serial port, whose fd is %d\n", fd);
	}
}

void set_baudrate(int fd, int baud) {
	struct termios newtios;

	if (fd >= 0) {
		tcgetattr(fd, &newtios);  // get attr
		cfsetospeed(&newtios, baud);  // output speed
		cfsetispeed(&newtios, baud); // input speed
		tcsetattr(fd, TCSADRAIN, &newtios);
		PRINTF("Change baud to %d, fd is %d\n", baud, fd);
	}
}

int write_serial(int fd, const void *buf, int len, int timeout) {
	return safe_write_timeout(fd, buf, len, timeout);
}

int read_serial(int fd, void *buf, int len, int timeout) {
	return safe_read_timeout(fd, buf, len, timeout);
}

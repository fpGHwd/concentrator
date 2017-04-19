/*
 * serial.h
 *
 *  Created on: 2015-8-11
 *      Author: Johnnyzhang
 */

#ifndef SERIAL_H_
#define SERIAL_H_

#include "typedef.h"

int open_serial(const char * serial, int baud, int char_size, int parity);
void close_serial(int fd);
void set_baudrate(int fd, int baud);
int write_serial(int fd, const void *buf, int len, int timeout);
int read_serial(int fd, void *buf, int len, int timeout);

#endif /* SERIAL_H_ */

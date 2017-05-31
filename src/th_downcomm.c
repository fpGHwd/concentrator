/*
 * th_downcomm.c
 *
 *  Created on: 2015-8-11
 *      Author: Johnnyzhang
 */

#include "threads.h"
#include <stdbool.h>
#include "main.h"

void *th_downcomm(void * arg) {
	while (!g_terminated) {
		sleep(1);
	}
	return NULL;
}

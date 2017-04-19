#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <linux/input.h>
#include "key_hw.h"

/*
 按键值的定义在以下头文件中
 linux-devkit/sysroots/x86_64-arago-linux/usr/arm-linux-gnueabihf/libc/usr/include$ vi linux/input.h   
 */

/*
 jzq系统所注册的按键有:
 KEY_UP、KEY_DOWN、KEY_LEFT、KEY_RIGHT、KEY_OK、KEY_CANCEL
 */

#define KEY_PATH      "/dev/input/event0"

int key_fd = -1;

////
int KeyOpen(void) {
	if (key_fd == -1) {
		key_fd = open(KEY_PATH, O_RDWR); //// O_RDWR - linux descriptor
		if (key_fd < 0) {
			//printf("open key driver failed\n");
			return 0;
		}
	}
	return 1;
}

int KeyClose(void) {
	if (key_fd > 0) {
		close(key_fd);
		key_fd = -1;
	}

	return 1;
}

int KeyRead(struct key_msg_t *msg) {
	static struct input_event data;
	int ret;

	msg->code = 0;
	msg->type = 0;

	if (key_fd > 0) {

		///printf("wait here to read 1 the key value\n");
		ret = read(key_fd, &data, sizeof(data));

		if (data.type == EV_KEY) /// EV_KEY
		{
			msg->code = data.code;
			memset(&data, 0x0, sizeof(data)); /// driver not specified
			///printf("wait here to read 2 the key value\n");
			ret = read(key_fd, &data, sizeof(data));
			///printf("wait here to read 3 the key value\n");
			ret = read(key_fd, &data, sizeof(data));
			if (data.type == EV_MSC)  /// EV_MSC
			{
				msg->type = (enum key_type_t) data.code;
				return 1;
			}
		}
		return 0;
	}
}

void testkey(void) {
	struct key_msg_t msg = { 0 };

	KeyOpen();
	while (1) {
		if (KeyRead(&msg) == 1)
			printf("key:code = %x,type=%x\n", msg.code, msg.type);
	}
	KeyClose();
}

void key_exit(void) {
	KeyClose();
}

void key_initiate(void) {
	int ret;
	ret = KeyOpen();
	if (ret != 0)
		fprintf(stderr, "key initiated error\n");
}

#ifndef __KEY_H__
#define __KEY_H__

enum key_type_t {
	KEY_PRESS_DOWN = 0x0001,
	KEY_PRESS_SHORT_UP = 0x0002,
	KEY_PRESS_LONG_UP = 0x0004,
};

struct key_msg_t {
	unsigned int code; /// code
	enum key_type_t type; /// type
};

void testkey(void);
int KeyOpen(void);
int KeyClose(void);
int KeyRead(struct key_msg_t *msg);
void key_initiate(void);
void key_exit(void);

#endif


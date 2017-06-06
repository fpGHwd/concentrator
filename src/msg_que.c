/*
 * msg_que.c
 *
 *  Created on: 2015-8-11
 *      Author: Johnnyzhang
 */

#include "msg_que.h"

struct node {
	int stamp;
	int size;
	char *data;
	struct node *next;
};

struct queue {
	struct node *head;
	struct node *tail;
	int count;
};

static struct queue msg_que[MSG_QUE_MAX];
static sem_t msg_sem[MSG_QUE_MAX];
static int cur_size[MSG_QUE_MAX];
static const int max_size[MSG_QUE_MAX] = { 1024 * 1024, 1024 * 1024, 1024
		* 1024, 1024 * 1024, 512 * 1024, 512 * 1024, 512 * 1024, 512 * 1024, 512
		* 1024, };

static void queue_init(struct queue *queue) {
	queue->count = 0;
	queue->head = NULL;
	queue->tail = NULL;
}

static void queue_add(struct queue *queue, struct node *elm) {
	if (queue->count == 0) {
		queue->head = elm;
		queue->tail = elm;
	} else {
		queue->tail->next = elm;
		queue->tail = elm;
	}
	elm->next = NULL;
	queue->count++;
}

static void queue_del(struct queue *queue, struct node **elm, int stamp) /// stamp
{
	struct node *node, *prev;

	*elm = NULL;
	for (prev = node = queue->head; node != NULL;
			prev = node, node = node->next) {
		if (stamp == 0 || node->stamp == stamp) { // stamp = 0 matches all
			*elm = node;
			queue->count--;
			/*queue modified*/
			if (queue->count == 0)
				queue->head = queue->tail = NULL;
			else if (node == queue->head)
				queue->head = node->next;
			else if (node == queue->tail)
				queue->tail = prev;
			else
				prev->next = node->next;
			(*elm)->next = NULL;
			///element next pointer
			break;
		}
	}
}

static int queue_is_empty(struct queue *queue, int stamp)
{
	int ret;
	struct node *node;

	ret = 1;
	for (node = queue->head; node != NULL; node = node->next) {
		if (stamp == 0 || node->stamp == stamp) // stamp = 0 matches all
			ret = 0;
	}
	return ret;
}

void msg_que_init(void) {
	int idx;

	for (idx = 0; idx < MSG_QUE_MAX; idx++) {
		sem_init(&msg_sem[idx], 0, 1);
		queue_init(&msg_que[idx]);
		cur_size[idx] = 0;
	}
}

void msg_que_destroy(void)
{
	int idx;
	struct node *elm;

	for (idx = 0; idx < MSG_QUE_MAX; idx++) {
		sem_wait(&msg_sem[idx]);
		while (1) {
			queue_del(&msg_que[idx], &elm, 0);
			if (elm == NULL)
				break;
			free(elm);
		}
		cur_size[idx] = 0;
		sem_post(&msg_sem[idx]);
		sem_destroy(&msg_sem[idx]);
	}
}

int msg_que_is_empty(int idx, int stamp) {
	int ret = 1;

	if (idx >= 0 && idx < MSG_QUE_MAX) {
		sem_wait(&msg_sem[idx]);
		ret = queue_is_empty(&msg_que[idx], stamp);
		sem_post(&msg_sem[idx]);
	}
	return ret;
}

int msg_que_get(int idx, void *buf, int max_len, int *len, int stamp) {
	int ret = -1, size;
	struct node *elm;

	*len = 0;
	if (buf == NULL)
		return -1;
	if (idx >= 0 && idx < MSG_QUE_MAX) {
		sem_wait(&msg_sem[idx]);
		queue_del(&msg_que[idx], &elm, stamp);
		if (elm != NULL) {
			size = elm->size + sizeof(struct node);
			cur_size[idx] -= size;
			if (elm->size <= max_len) {
				*len = elm->size;
				memcpy(buf, elm->data, elm->size);
				ret = elm->stamp;
			}
			free(elm);
		}
		sem_post(&msg_sem[idx]);
	}
	return ret;
}

int msg_que_put(int idx, const void *buf, int len, int stamp) {
	int ret = 0, size;
	struct node *elm;

	if (idx >= 0 && idx < MSG_QUE_MAX) {
		sem_wait(&msg_sem[idx]);
		size = len + sizeof(struct node);
		if (max_size[idx] == 0 || cur_size[idx] + size <= max_size[idx]) {
			// max_size = 0 means size has no limit
			if ((elm = malloc(size)) != NULL) {
				cur_size[idx] += size;
				elm->stamp = stamp;
				elm->size = len;
				elm->data = (char *) (elm + 1);
				memcpy(elm->data, buf, len);
				queue_add(&msg_que[idx], elm);
				ret = 1;
			}
		}
		sem_post(&msg_sem[idx]);
	}
	return ret;
}

#ifndef __LINUX_DS2782_BATTERY_H
#define __LINUX_DS2782_BATTERY_H

#include <linux/list.h>

struct ds278x_platform_data {
	int rsns;
	int gpio_enable;
	int gpio_charging;
};

typedef struct QueueItem {
	struct list_head    list;      // Linked List
	void *              item;      // Item of Data
} QueueItem;

typedef struct Queue {
   struct list_head    list;      // List Head
   int currentSize;
   int maxSize;
   int sum;
} Queue;

Queue *  qCreate(int size);
void     qDelete(Queue *q);
void     qEnqueue(Queue *q, void *item);
void *   qDequeue(Queue *q);
int      qIsEmpty(Queue *q);
int      qIsFull(Queue *q);

#endif

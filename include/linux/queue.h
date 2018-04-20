#ifndef _LINUX_QUEUE_H
#define _LINUX_QUEUE_H

#include <linux/list.h>

typedef struct queue{
  	struct list_head list;
   	int current_size;
   	int max_size;
} queue;

typedef struct queue_item{
	struct list_head list;
	void * item;
} queue_item;

static inline queue* queue_create(int _max_size) {
	queue* q;
	q = (queue *) kmalloc(sizeof(queue), GFP_ATOMIC);
	if (!q) {
		printk(KERN_ALERT "qCreate:kmalloc failed\n");
		return q;
	}

	INIT_LIST_HEAD(&(q->list));
	q->current_size = 0;
	q->max_size = _max_size;
	return q;
}

static inline int queue_is_empty(queue *q) {
   return list_empty(&(q->list));
}

static inline int queue_is_full(queue *q) {
   return q->current_size == q->max_size;
}

static inline void* queue_dequeue(queue *q) {
	queue_item *   qi;
	void *        item;

	if (queue_is_empty(q)) {
		return NULL;
	}

	qi = list_first_entry(&(q->list), queue_item, list);
	item = qi->item;
	list_del(&(qi->list));
	kfree((void *) qi);
	q->current_size = q->current_size - 1;

	return item;
}

static inline void* queue_enqueue(queue *q, void *item) {
	queue_item *   qi;
	void *   removed_item;

	if (queue_is_full(q)) {
		removed_item = queue_dequeue(q);
	}
	else {
		removed_item = NULL;
	}

	qi = (queue_item *) kmalloc(sizeof(queue_item), GFP_ATOMIC);
	if (! qi) {
		printk(KERN_ALERT "XXXX queue_enqueue: kmalloc failed\n");
		return;
	}

	qi->item = item;
	list_add_tail(&(qi->list), &(q->list));
	q->current_size = q->current_size + 1;

	return removed_item;
}

static inline void queue_delete(queue *q) {
	while (queue_dequeue(q) ){
		continue;
	}
	kfree((void *) q);
	return;
}

#endif /* _LINUX_QUEUE_H */

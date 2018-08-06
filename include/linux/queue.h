#ifndef _LINUX_QUEUE_H
#define _LINUX_QUEUE_H

#include <linux/list.h>
#include <linux/spinlock.h>

typedef struct queue{
  	struct list_head list;
  	spinlock_t lock;
   	int current_size;
} queue;

typedef struct queue_item{
	struct list_head list;
	void * item;
} queue_item;

static inline int queue_lock(queue *q) {
	spin_lock(&(q->lock));
	return 0;
}

static inline void queue_unlock(queue *q) {
	spin_unlock(&(q->lock));
	return;
}

static inline queue* queue_create() {
	queue* q;
	q = (queue *) kmalloc(sizeof(queue), GFP_ATOMIC);
	if (!q) {
		printk(KERN_ALERT "qCreate:kmalloc failed\n");
		return q;
	}

	INIT_LIST_HEAD(&(q->list));
	q->current_size = 0;
	spin_lock_init(&(q->lock));

	return q;
}

static inline int queue_is_empty(queue *q) {
	int   ret;
	queue_lock(q);
	ret = list_empty(&(q->list));
	queue_unlock(q);
	return ret;
}

inline void* queue_dequeue(queue *q) {
	queue_item *   qi;
	void *        item;

	if (queue_is_empty(q)) {
		return NULL;
	}

	queue_lock(q);
	qi = list_first_entry(&(q->list), queue_item, list);
	item = qi->item;
	list_del(&(qi->list));
	kfree((void *) qi);
	q->current_size = q->current_size - 1;
	queue_unlock(q);

	return item;
}

inline void queue_enqueue(queue *q, void *item) {
	queue_item *   qi;

	qi = (queue_item *) kmalloc(sizeof(queue_item), GFP_ATOMIC);
	if (! qi) {
		printk(KERN_ALERT "XXXX queue_enqueue: kmalloc failed\n");
		return;
	}

	qi->item = item;
	queue_lock(q);
	list_add_tail(&(qi->list), &(q->list));
	q->current_size = q->current_size + 1;
	queue_unlock(q);

	return;
}

static inline void queue_delete(queue *q) {
	while (queue_dequeue(q) ){
		continue;
	}
	kfree((void *) q);
	return;
}

#endif /* _LINUX_QUEUE_H */

#include "queue.h"
#include "order.h"

#define QUEUE_SIZE  10

static order_t order_queue[QUEUE_SIZE];

static uint8_t queue_readposition = 0;
static uint8_t queue_writeposition = 0;
static uint8_t queue_entries = 0;

void queue_init(void) {
	uint8_t i = 0;
	for (; i < (QUEUE_SIZE - 1); i++) {
		order_init(&order_queue[i]);
	}
	queue_readposition = 0;
	queue_writeposition = 0;
	queue_entries = 0;
}

uint8_t queue_push(const order_t * const order) {
	if (queue_entries == QUEUE_SIZE)
		return 0;
	order_queue[queue_writeposition] = *order;
	queue_entries++;
	queue_writeposition++;
	queue_writeposition %= QUEUE_SIZE;
	return 1;
}

const order_t * const queue_get_current_order(void) {
	if (queue_entries != 0) {
		return &order_queue[queue_readposition];
	}
	return 0;
}

void queue_pop(void) {
	if (queue_entries != 0) {
		order_init(&order_queue[queue_readposition]);
		queue_entries--;
		queue_readposition++;
		queue_readposition %= QUEUE_SIZE;
	}
}

uint8_t queue_order_available(void) {
	return queue_entries;
}

void queue_update(void) {}

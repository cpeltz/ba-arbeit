#include "queue.h"
#include "order.h"

#define QUEUE_SIZE  10

static order_t order_queue[QUEUE_SIZE];
static order_t priority_order;
static uint8_t paused;

static uint8_t queue_readposition = 0;
static uint8_t queue_writeposition = 0;
static uint8_t queue_entries = 0;

void queue_init(void) {
	uint8_t i = 0;
	for (; i < (QUEUE_SIZE - 1); i++) {
		order_init(&order_queue[i]);
	}
	order_init(&priority_order);
	queue_readposition = 0;
	queue_writeposition = 0;
	queue_entries = 0;
	paused = 0;
}

uint8_t queue_push_priority(const order_t * const order) {
	priority_order = *order;
	return 1;
}

uint8_t queue_push(const order_t * const order) {
	// Add Order to the Queue if Queue not full
	if (queue_entries == QUEUE_SIZE)
		return 0;
	order_queue[queue_writeposition] = *order;
	queue_entries++;
	queue_writeposition++;
	queue_writeposition %= QUEUE_SIZE;
	return 1;
}

const order_t * const queue_get_current_normal_order(void) {
	if (queue_entries != 0) {
		return &order_queue[queue_readposition];
	}
	return 0;
}

const order_t * const queue_get_current_order(void) {
	// Bypass normal Queue if we have a priority instruction
	if (priority_order.status & ORDER_STATUS_PRIORITY)
		return &priority_order;
	// Execution is paused, don't return an order
	if (pause)
		return 0;
	// Return the current Order
	// TODO Maybe exchange with "return queue_get_current_normal_order();"
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

// TODO: What should be done if pushing failed?
void queue_update(void) {
	order_t local_order;
	if (parser_has_new_order()) {
		order_init(&local_order);
		parser_get_new_order(&local_order);
		if (local_order->status & ORDER_STATUS_PRIORITY)
			queue_push_priority(&local_order);
		else
			queue_push(&local_order);
	}
}

void queue_clear(void) {
	queue_init();
}

void queue_clear_priority(void) {
	order_init(&priority_order);
}

void queue_pause(void) {
	pause = 1;
}

void queue_unpause(void) {
	pause = 0;
}

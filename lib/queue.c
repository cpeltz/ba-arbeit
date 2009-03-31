#include "queue.h"
#include "order.h"

#define QUEUE_SIZE  10

static order_t order_queue[QUEUE_SIZE];
static order_t control_order;
static uint8_t paused;

static uint8_t queue_readposition = 0;
static uint8_t queue_writeposition = 0;
static uint8_t queue_entries = 0;

void queue_init(void) {
	uint8_t i = 0;
	for (; i < (QUEUE_SIZE - 1); i++) {
		order_init(&order_queue[i]);
	}
	order_init(&control_order);
	queue_readposition = 0;
	queue_writeposition = 0;
	queue_entries = 0;
	paused = 0;
}

uint8_t queue_push(const order_t * const order) {
	// Bypass normal queue for Control orders
	if (order->data[0] & ORDER_TYPE_CONTROL) {
		control_order = *order;
		return 1;
	}
	// Add Order to the Queue if Queue not full
	if (queue_entries == QUEUE_SIZE)
		return 0;
	order_queue[queue_writeposition] = *order;
	queue_entries++;
	queue_writeposition++;
	queue_writeposition %= QUEUE_SIZE;
	return 1;
}

const order_t * const queue_get_current_order(void) {
	// Bypass normal Queue if we have a control instruction
	if (control_order.data[0])
		return &control_order;
	// Execution is paused, don't return an order
	if (pause)
		return 0;
	// Return the current Order
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

void queue_clear(void) {
	queue_init();
}

void queue_clear_control(void) {
	order_init(&control_order);
}

void queue_pause(void) {
	pause = 1;
}

void queue_unpause(void) {
	pause = 0;
}

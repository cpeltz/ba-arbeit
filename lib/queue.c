#include "queue.h"
#include "order.h"

/**
 * @defgroup QUEUE_Module Order Queue
 * Maintaining and manupulation of the order queue is
 * what this module provides.
 * @{
 */

/**
 * The order queue buffer.
 */
static order_t order_queue[QUEUE_SIZE];
/**
 * Single priority order.
 */
static order_t priority_order;
/**
 * Indicates whether or not the queue execution is paused.
 */
static uint8_t paused;

/**
 * The position in the buffer, from where to read.
 */
static uint8_t queue_readposition = 0;
/**
 * The position in the buffer, where to write.
 */
static uint8_t queue_writeposition = 0;
/**
 * Number of entries in the queue.
 */
static uint8_t queue_entries = 0;

/**
 * Initializes the queue.
 *
 * Mainly used to init the buffer and set the needed variables.
 */
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

/**
 * Puts a priority order in the queue.
 *
 * @param[in] order The order to be put in the queue.
 * @return <em>uint8_t</em> 1 (can be ignored for now).
 * @todo Either remove the return value or give it meaning.
 */
uint8_t queue_push_priority(const order_t * const order) {
	priority_order = *order;
	return 1;
}

/**
 * Puts a new order in the queue.
 *
 * @param[in] order The order to be put in the queue.
 * @return <em>uint8_t</em> 0 if buffer is full, 1 otherwise.
 */
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

/**
 * Queries the order which is getting executed right now.
 *
 * @return <em>const order_t *</em> A read only pointer to the
 * order that gets executed right now. Needed to be able to
 * answer on the "get current order" register instruction.
 */
const order_t * const queue_get_current_normal_order(void) {
	if (queue_entries != 0) {
		return &order_queue[queue_readposition];
	}
	return 0;
}

/**
 * Gets the current order to be executed.
 *
 * Priority orders will bypass normal orders even if they are
 * already in execution.
 * @return <em>const order_t *</em> The order to be worked with.
 * @todo Remove the const, it will hinder status setting.
 */
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

/**
 * Removes one order from the queue.
 */
void queue_pop(void) {
	if (queue_entries != 0) {
		order_init(&order_queue[queue_readposition]);
		queue_entries--;
		queue_readposition++;
		queue_readposition %= QUEUE_SIZE;
	}
}

/**
 * Check if new orders are available.
 *
 * @return <em>uint8_t</em> The number of orders in the buffer.
 */
uint8_t queue_order_available(void) {
	return queue_entries;
}

/**
 * Updates Queue: Get new orders from parser.
 * @todo Needs failsave when queue is full.
 */
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

/**
 * Clears the queue and removes all orders.
 */
void queue_clear(void) {
	queue_init();
}

/**
 * Removes the priority order.
 */
void queue_clear_priority(void) {
	order_init(&priority_order);
}

/**
 * Pauses the execution of the queue.
 */
void queue_pause(void) {
	pause = 1;
}

/**
 * Unpauses the execution of the queue.
 */
void queue_unpause(void) {
	pause = 0;
}
/*@}*/

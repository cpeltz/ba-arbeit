#ifndef _ORDER_H_
#define _ORDER_H_

#include "types.h"

/**
 * Shows that the order has been initialized.
 */
#define ORDER_STATUS_INITIALIZED            1
/**
 * Shows that an order is a valid order.
 * @todo Is this in use?
 */
#define ORDER_STATUS_VALID                  2
/**
 * Shows that the execution of the order is done.
 */
#define ORDER_STATUS_DONE                   4
/**
 * Shows that, in the case of only one trigger, it has been reached.
 * @todo Check whether or not the drive instructions use this.
 */
#define ORDER_STATUS_TRIGGER_REACHED        8
/**
 * Shows that the trigger for the left wheel has been reached.
 */
#define ORDER_STATUS_TRIGGER_LEFT_REACHED   8
/**
 * Shows that the trigger for the right wheel has been reached.
 */
#define ORDER_STATUS_TRIGGER_RIGHT_REACHED 16
/**
 * Shows that the order has started execution, but is not done.
 */
#define ORDER_STATUS_STARTED               32
/**
 * Shows that the order is a priority order.
 */
#define ORDER_STATUS_PRIORITY              64

#define ORDER_TYPE_EXTENDED  0x00
#define ORDER_TYPE_CONTROL   0x01
#define ORDER_TYPE_QUERY     0x02
#define ORDER_TYPE_DRIVE     0x03
#define ORDER_TYPE_ADV_DRIVE 0x04
#define ORDER_TYPE_SETPID    0x05
#define ORDER_TYPE_OPTION    0x06

void order_init(order_t *order);
void order_copy(const order_t * const from, order_t *to);
void order_process(order_t *order);
void order_check_triggers(order_t *order);
void order_array_init(void);
uint8_t order_size(order_t *order);

#endif

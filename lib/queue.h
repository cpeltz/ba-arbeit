#ifndef QUEUE_H
#define QUEUE_H

#include <inttypes.h>
#include "types.h"

void queue_init(void);
const order_t * const queue_get_current_order(void);
void queue_pop(void);
uint8_t queue_order_available(void);
void queue_update(void);	// TODO Implement
uint8_t queue_push(const order_t * const order);

#endif

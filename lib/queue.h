#ifndef QUEUE_H
#define QUEUE_H

#include <inttypes.h>
#include "types.h"

void queue_init(void);
order_t * queue_get_current_normal_order(void);
order_t * queue_get_current_order(void);
void queue_pop(void);
uint8_t queue_order_available(void);
void queue_update(void);
uint8_t queue_push(const order_t * const order);
void queue_push_priority(order_t * order);
void queue_clear(void);
void queue_clear_priority(void);
void queue_pause(void);
void queue_unpause(void);

#endif

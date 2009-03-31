#ifndef QUEUE_H
#define QUEUE_H

#include <inttypes.h>
#include "types.h"

void queue_init(void);
const order_t * const queue_get_current_order(void);
void queue_pop(void);
uint8_t queue_order_available(void);
void queue_update(void);
uint8_t queue_push(const order_t * const order);
uint8_t queue_push_priority(const order_t * const order);
void queue_clear(void);
void queue_clear_control(void);
void queue_pause(void);
void queue_unpause(void);

#endif

#ifndef _ORDER_H_
#define _ORDER_H_

#include "types.h"

#define ORDER_STATUS_INITIALIZED            1
#define ORDER_STATUS_VALID                  2
#define ORDER_STATUS_DONE                   4
#define ORDER_STATUS_TRIGGER_REACHED        8
#define ORDER_STATUS_TRIGGER_LEFT_REACHED   8
#define ORDER_STATUS_TRIGGER_RIGHT_REACHED 16
#define ORDER_STATUS_STARTED		   32

void order_init(order_t *order);
void order_copy(order_t *from, order_t *to);
void order_process(order_t *order);
void order_check_triggers(order_t *order);

#endif

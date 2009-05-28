#ifndef _ORDER_H_
#define _ORDER_H_

#include "types.h"

#define ORDER_STATUS_INITIALIZED            1
#define ORDER_STATUS_VALID                  2
#define ORDER_STATUS_DONE                   4
#define ORDER_STATUS_TRIGGER_REACHED        8
#define ORDER_STATUS_TRIGGER_LEFT_REACHED   8
#define ORDER_STATUS_TRIGGER_RIGHT_REACHED 16
#define ORDER_STATUS_STARTED               32
#define ORDER_STATUS_PRIORITY              64

#define ORDER_TYPE_CONTROL  0x01
#define ORDER_TYPE_REGISTER 0x02
#define ORDER_TYPE_DRIVE    0x03
#define ORDER_TYPE_SETPID   0x04
#define ORDER_TYPE_PIDDRIVE 0x05

void order_init(order_t *order);
void order_copy(order_t *from, order_t *to);
void order_process(order_t *order);
void order_check_triggers(order_t *order);
void order_array_init(void);
uint8_t order_size(order_t *order);

#endif

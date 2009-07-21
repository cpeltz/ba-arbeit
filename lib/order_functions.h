#ifndef _ORDER_FUNCTIONS_
#define _ORDER_FUNCTIONS_
#include "types.h"

void extended_instruction(order_t *);
void control_instruction(order_t *);
void register_instruction(order_t *);
void drive_instruction(order_t *);
void pid_drive_instruction(order_t *);
void set_pid_instruction(order_t *);

#endif

#ifndef IRQ_H
#define IRQ_H

#include "types.h"

void irq_init(void);
int16_t wheel_read_position(uint8_t wheel);
void wheel_clear_position(uint8_t wheel);
int16_t wheel_read_difference(void);
void wheel_write_difference(int16_t difference);
void wheel_clear_difference(void);

#endif

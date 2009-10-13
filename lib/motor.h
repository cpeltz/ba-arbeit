#ifndef MOTOR_H
#define MOTOR_H

#include <inttypes.h>

void motor_init(void);
void motor_set_speed(uint8_t wheel, int8_t speed);
int8_t motor_read_speed(uint8_t wheel);

#endif

#ifndef MOTOR_H
#define MOTOR_H

#include <inttypes.h>
#include "types.h"

void motor_init(void);
void motor_SetSpeed(uint8_t motor, int8_t speed);
int8_t motor_ReadSpeed(uint8_t motor);
void motor_Break(uint8_t motor);
void motor_status(global_state_t *state);

#endif

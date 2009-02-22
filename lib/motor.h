#ifndef MOTOR_H
#define MOTOR_H

void motor_Init(void);
void motor_SetSpeed(uint8_t motor, int8_t speed);
int8_t motor_ReadSpeed(uint8_t motor);
void motor_Break(uint8_t motor);

#endif

#ifndef TIMER_H
#define TIMER_H

#include <inttypes.h>

void timer_init(void);
void wheel_AddSpeed_W0(void);
void wheel_AddSpeed_W1(void);
void wheel_DelSpeed_W0(void);
void wheel_DelSpeed_W1(void);
int8_t wheel_ReadSpeed(const uint8_t wheel);

#endif

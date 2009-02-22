#ifndef TIMER_H
#define TIMER_H

void timer_Init(void);
void wheel_AddSpeed_W0(void);
void wheel_AddSpeed_W1(void);
void wheel_DelSpeed_W0(void);
void wheel_DelSpeed_W1(void);
int8_t wheel_ReadSpeed(const uint8_t wheel);
void trigger_Set_T(const uint8_t wheel, const uint16_t time);
void trigger_Clear_T(const uint8_t wheel);
uint16_t trigger_Get_T(const uint8_t wheel);

#endif

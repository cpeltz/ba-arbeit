#ifndef LED_H
#define LED_H

#include <inttypes.h>

void led_init(void);
void led_switch(uint8_t led, uint8_t state);
void led_switchoff(void);
void led_test(void);
void led_next_test(uint8_t);
uint16_t led_GetState(void);
void led_ToggleBit(void);
uint8_t led_ReadToggleBit(void);


#endif

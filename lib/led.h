#ifndef LED_H
#define LED_H

void led_init(void);
void led_switch(uint8_t led, uint8_t state);
void led_switchoff(void);
void led_test(void);
uint16_t led_GetState(void);
void led_ToggleBit(void);
uint8_t led_ReadToggleBit(void);


#endif

#ifndef LED_H
#define LED_H

#include "definitions.h"

#define led_switch_on(led) (LED_PORT |= (1 << led))
#define led_switch_off(led) (LED_PORT &= ~(1 << led))
#define led_toggle(led) (LED_PORT ^= (1 << led))

void led_init(void);

#endif

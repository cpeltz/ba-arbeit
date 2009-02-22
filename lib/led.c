#include <avr/io.h>
#include "led.h"
#include "definitions.h"
#include "flags.h"
#include "debug.h"

static uint16_t led_State = 0;

void
led_switchoff(void)
{
  // Alle LEDs ausschalten
  led_State &= (1<<15);
}

void
led_init(void)
{
  // Initialisiere LED Ausgänge und schalte LEDs aus
  LED_DDR |=
    (1 << LED_RED1) | (1 << LED_RED2) | (1 << LED_ORANGE) | (1 << LED_GREEN) | (1 << LED_BLUE);
  led_switchoff();
}

void
led_switch(uint8_t led, uint8_t state)
// LED an/aus schalten
{
  switch(led)
    {
    case LED_RED1:
      led_State &= ~(7<<LED_RED1_BIT);
      led_State |= (state<<LED_RED1_BIT);
      break;
    case LED_RED2:
      led_State &= ~(7<<LED_RED2_BIT);
      led_State |= (state<<LED_RED2_BIT);
      break;
    case LED_ORANGE:
      led_State &= ~(7<<LED_ORANGE_BIT);
      led_State |= (state<<LED_ORANGE_BIT);
      break;
    case LED_GREEN:
      led_State &= ~(7<<LED_GREEN_BIT);
      led_State |= (state<<LED_GREEN_BIT);
      break;
    case LED_BLUE:
      led_State &= ~(7<<LED_BLUE_BIT);
      led_State |= (state<<LED_BLUE_BIT);
      break;
    } 
  //LED_PORT |= (1 << led);
  //LED_PORT &= ~(state << led);
}

void
led_test(void)
// LEDs testen
{
  debug_WriteString_P(PSTR("\nTeste Leuchtdioden...\r\n"));
  while(!(flag_ReadAndClear(TIMER_262MS)));
  led_switch(LED_RED1, ON);
  debug_WriteString_P(PSTR("RED1"));
  while(!(flag_ReadAndClear(TIMER_262MS)));
  led_switch(LED_RED2, ON);
  debug_WriteString_P(PSTR(", RED2"));
  while(!(flag_ReadAndClear(TIMER_262MS)));
  led_switch(LED_ORANGE, ON);
  debug_WriteString_P(PSTR(", ORANGE"));
  while(!(flag_ReadAndClear(TIMER_262MS)));
  led_switch(LED_GREEN, ON);
  debug_WriteString_P(PSTR(", GREEN"));
  while(!(flag_ReadAndClear(TIMER_262MS)));
  led_switch(LED_BLUE, ON);
  debug_WriteString_P(PSTR(", BLUE\r\n"));
  while(!(flag_ReadAndClear(TIMER_262MS)));
}

uint16_t
led_GetState(void)
{
  return led_State;
}

void
led_ToggleBit(void)
{
  led_State ^= (1<<15);
}

uint8_t
led_ReadToggleBit(void)
{
  return (led_State>>15);
}

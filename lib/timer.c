#define __AVR_ATmega2561__

#include <inttypes.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "timer.h"
#include "definitions.h"
#include "flags.h"
#include "led.h"

static int8_t timer_Speed_W0 = 0;
static int8_t timer_Speed_W1 = 0;
static int8_t timer_SpeedSum_W0 = 0;
static int8_t timer_SpeedSum_W1 = 0;
static uint16_t timer_t_trigger_counter[2] = { 0, 0 };
static uint8_t timer_trigger = 0;

static int8_t timer_t_i8 = 0;

#define TIMER_T_TRIGGER_L   0
#define TIMER_T_TRIGGER_R   1

ISR(TIMER1_COMPA_vect) {
	// ISR wird jede Millisekunde ausgeführt
	OCR1A = TCNT1 + 250;
	// Neuen Wert für Compare Match A setzen
	flag_set(TIMER_1MS);
	// und TIMER_1MS Flag setzen
}

ISR(TIMER1_COMPB_vect) {
	// IST wird alle 10 Millisekunden ausgeführt
	OCR1B = TCNT1 + 2500;
	// Neuen Wert für Compare Match B setzen
	flag_set(TIMER_10MS);
	// under TIMER_10MS Flag setzen
}

ISR(TIMER1_COMPC_vect) {
	// ISR wird alle 100 Millisekunden ausgeführt
	OCR1C = TCNT1 + 25000;
	// Neuen Wert für Compare Match C setzen
	flag_set(TIMER_100MS);
	// und TIMER_100MS Flag setzen

	// T_TRIGGER überprüfen
	if (flag_local_read(&timer_trigger, TIMER_T_TRIGGER_L)) {
		// linker T_Trigger aktiv
		if (timer_t_trigger_counter[0] == 0) {
			flag_set(T_TRIGGER_L);
			flag_local_clear(&timer_trigger, TIMER_T_TRIGGER_L);
		} else {
			timer_t_trigger_counter[0]--;
		}
	}
	if (flag_local_read(&timer_trigger, TIMER_T_TRIGGER_R)) {
		// rechter T_Trigger aktiv
		if (timer_t_trigger_counter[1] == 0) {
			flag_set(T_TRIGGER_R);
			flag_local_clear(&timer_trigger, TIMER_T_TRIGGER_R);
		} else {
			timer_t_trigger_counter[1]--;
		}
	}

	// Geschwindigkeiten in timer_Speed_W0/1 übertragen
	timer_Speed_W0 = timer_SpeedSum_W0;
	timer_Speed_W1 = timer_SpeedSum_W1;
	timer_SpeedSum_W0 = 0;
	timer_SpeedSum_W1 = 0;
}

ISR(TIMER1_OVF_vect) {
	uint8_t led_counter = 0;
	// ISR wird alle 262 Millisekunden aufgerufen
	flag_set(TIMER_262MS);
	// TIMER_262MS Flag setzen

	// LEDs schalten
	led_ToggleBit();
  
	for (; led_counter < 5; led_counter++) {
		switch ((led_GetState()&(7<<(led_counter * 3)))>>(led_counter * 3)) {
			case 0:
				LED_PORT |= (1<<led_counter);
				break;
			case 1:
				LED_PORT &= ~(1<<led_counter);
				break;
			case 2:
				if (led_ReadToggleBit())
					LED_PORT ^= (1<<led_counter);
				break;
			case 3:
				LED_PORT ^= (1<<led_counter);
				break;
			case 4:
				if (!(led_ReadToggleBit())) {
					LED_PORT &= ~(1<<led_counter);
					led_switch(led_counter, SINGLEOFF);
				}
				break;
			case 5:
				if (led_ReadToggleBit()) {
					LED_PORT |= (1<<led_counter);
					led_switch(led_counter, OFF);
				}
				break;
		}
	}
}

void timer_init(void) {
	// Compare Matches einstellen
	OCR1A = 250;
	// Compare Match A -> 1kHz -> 1ms
	OCR1B = 2500;
	// Compare Match B -> 100Hz -> 10ms
	OCR1C = 25000;
	// Compare Match C -> 10Hz -> 100ms
	TIMSK1 = (1 << OCIE1C) | (1 << OCIE1B) | (1 << OCIE1A) | (1 << TOIE1);
	// Aktiviere Compare Match A+B+C IRQ und Overflow IRQ
	TCCR1B = (3 << CS10);
	// Prescaler = clkIO/64 -> 250kHz -> 4µs
}

void wheel_AddSpeed_W0(void) {
	if ((timer_SpeedSum_W0 < 127)) {
		timer_SpeedSum_W0++;
	}
}

void wheel_AddSpeed_W1(void) {
	if ((timer_SpeedSum_W1 < 127)) {
		timer_SpeedSum_W1++;
	}
}

void wheel_DelSpeed_W0(void) {
	if ((timer_SpeedSum_W0 > -127)) {
		timer_SpeedSum_W0--;
	}
}

void wheel_DelSpeed_W1(void) {
	if ((timer_SpeedSum_W1 > -127)) {
		timer_SpeedSum_W1--;
	}
}

int8_t wheel_ReadSpeed(const uint8_t wheel) {
	switch (wheel) {
		case 0:
			timer_t_i8 = timer_Speed_W0;
			break;
		case 1:
			timer_t_i8 = timer_Speed_W1;
			break;
	}
	return timer_t_i8;
}

void trigger_Set_T(const uint8_t wheel, const uint16_t time) {
	switch (wheel) {
		case 0:
		case 1:
			timer_t_trigger_counter[wheel] = time;
			flag_local_set(&timer_trigger, TIMER_T_TRIGGER_L + wheel);
			flag_clear(T_TRIGGER_L + wheel);
			break;
		case 2:
			trigger_Set_T(0, time);
			trigger_Set_T(1, time);
			break;
	}
}

void trigger_Clear_T(const uint8_t wheel) {
	switch (wheel) {
		case 0:
		case 1:
			flag_local_clear(&timer_trigger, TIMER_T_TRIGGER_L + wheel);
			flag_clear(T_TRIGGER_L + wheel);
			break;
		case 2:
			trigger_Clear_T(0);
			trigger_Clear_T(1);
			break;
	}
}

uint16_t trigger_Get_T(const uint8_t wheel) {
	uint16_t ret = 0;
	switch (wheel) {
		case 0:
			ret = timer_t_trigger_counter[0];
			break;
		case 1:
			ret = timer_t_trigger_counter[1];
			break;
	}
	return ret;
}

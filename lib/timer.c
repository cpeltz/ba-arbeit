/*#define __AVR_ATmega2561__*/

#include <inttypes.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "timer.h"
#include "definitions.h"
#include "flags.h"
#include "led.h"

/**
 * @defgroup TIMER_Module Timer Module
 * Provides functions and variables for timing purposes.
 * @{
 */

static int8_t timer_Speed_W0 = 0;
static int8_t timer_Speed_W1 = 0;
static int8_t timer_SpeedSum_W0 = 0;
static int8_t timer_SpeedSum_W1 = 0;
uint16_t timer_t_trigger_counter[2] = { 0, 0 };
uint16_t timer_1s_counter = 0;
uint8_t timer_100ms_counter = 0;

/*ISR(TIMER1_COMPA_vect) {
	// ISR wird jede Millisekunde ausgeführt
	OCR1A = TCNT1 + 250;
	// Neuen Wert für Compare Match A setzen
	flag_set(TIMER_1MS);
	// und TIMER_1MS Flag setzen
}*/

/*ISR(TIMER1_COMPB_vect) {
	// IST wird alle 10 Millisekunden ausgeführt
	OCR1B = TCNT1 + 2500;
	// Neuen Wert für Compare Match B setzen
	flag_set(TIMER_10MS);
	// under TIMER_10MS Flag setzen
}*/

ISR(TIMER1_COMPC_vect) {
	// ISR wird alle 100 Millisekunden ausgeführt
	OCR1C = TCNT1 + 25000;
	// Neuen Wert für Compare Match C setzen
	flag_set(TIMER_100MS);
	// und TIMER_100MS Flag setzen

	// linker T_Trigger aktiv
	if (timer_t_trigger_counter[0] != 0) {
		timer_t_trigger_counter[0]--;
	}
	if (timer_t_trigger_counter[1] != 0) {
		timer_t_trigger_counter[1]--;
	}

	// Geschwindigkeiten in timer_Speed_W0/1 übertragen
	timer_Speed_W0 = timer_SpeedSum_W0;
	timer_Speed_W1 = timer_SpeedSum_W1;
	timer_SpeedSum_W0 = 0;
	timer_SpeedSum_W1 = 0;

	// Increase counter
	timer_100ms_counter++;
}

ISR(TIMER1_OVF_vect) {
	// ISR wird alle 262 Millisekunden aufgerufen
	flag_set(TIMER_262MS);
	// TIMER_262MS Flag setzen
	if( timer_100ms_counter > 10 ) {
		timer_1s_counter += timer_100ms_counter / 10;
		timer_100ms_counter %= 10;
	}
}

/**
 * Initializes the timer registers.
 */
void timer_init(void) {
	// Compare Matches einstellen
//	OCR1A = 250;
	// Compare Match A -> 1kHz -> 1ms
//	OCR1B = 2500;
	// Compare Match B -> 100Hz -> 10ms
	OCR1C = 25000;
	// Compare Match C -> 10Hz -> 100ms
	TIMSK1 = (1 << OCIE1C) | /*(1 << OCIE1B) | (1 << OCIE1A) |*/ (1 << TOIE1);
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
		case WHEEL_LEFT:
			return timer_Speed_W0;
			break;
		case WHEEL_RIGHT:
			return timer_Speed_W1;
			break;
	}
	return 0;
}
/*@}*/

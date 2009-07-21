#include "servo.h"

/**
 * Steuert die Position des Servomotors per PWM.
 * Stellt dazu die Frequenz der PWM auf 200 Hz
 * und setzt den Counter auf PWM1 und damit die
 * Länge des HI-Impulses auf die für den
 * Servo erforderliche Länge. Die Länge regelt
 * dabei den einzustellenden Winkel.
 * Parameter:
 * - pos: Einzustellende Grad-Zahl, mit 0 <= pos <= 180
 * Rückgabewert: void
 */
void servo_set_pos(unsigned char pos) {
	if(pos > 180) {
		pos = 180;
	}
	PWMP = 107; // 200 Hz: 107
	PWM1 = 255 - (31 + (pos / 2));
}

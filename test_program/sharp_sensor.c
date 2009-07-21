#include <reg552.h>
#include <timing.h>
#include "sharp_sensor.h"

/**
 * Führt eine Messung mit dem analogen Sharp-Abstandssensor durch.
 * Stößt eine Messung an, wartet auf Resultat, und gibt dieses
 * nach erfolgter Messung zurück.
 * Parameter:
 * - sens: Nummer des analogen Eingangs
 * Rückgabewert: unsigned int: Ausgelesener 10-Bit-Messwert
 */
unsigned int sharpa_measure(byte sens) {
	//wait_ms(100);	  // TODO: ??
	sens %= 8;
	ADCON&=0xE7;
	ADCON = sens | 0x08; // Set software-only start-mode and start measurement
	while(!(ADCON & 0x10));
	return ((ADCH << 2)	| ((ADCON & 0xC0) >> 6));
}

/**
 * Wandelt den Messwert eines analogen Abstandssensors
 * in einen cm-Wert mittels einer Messtabelle und einer
 * linearen Interpolation um.
 * Fängt Messwerte außerhalb des sinnergebenden Bereichs ab.
 * Parameter:
 * - val: Messwert des Sensors
 * Rückgabewert: unsigned char: cm-Abstand
 */
byte samplea_to_cm(unsigned int val, SensorType type) {
	switch(type) {
		case SENSOR_SHORT_DIST:
			if(val > TABLE_SHORT_DIST_MAX) return 4;
			else if(val < TABLE_SHORT_DIST_MIN) return 30;
			else return sample_to_cm(table_short_dist, TABLE_SHORT_DIST_LEN - 1, 1, val);
			break;
		case SENSOR_LONG_DIST:
			if(val > TABLE_LONG_DIST_MAX) return 10;
			else if(val < TABLE_LONG_DIST_MIN) return 70;
			else return sample_to_cm(table_long_dist, TABLE_LONG_DIST_LEN - 1, 5, val);
			break;
	}
}

/**
 * Wandelt den Messwert eines Abstandssensors in einen cm-Wert
 * mittels einer gegebenen Messtabelle und einer linearen
 * Interpolation um.
 * Parameter:
 * - table: gegebene Messtabelle
 * - offset: Index des letzten Elements der Tabelle
 * - val: Messwert des Sensors
 * Rückgabewert: unsigned char: cm-Abstand
 */
 
#pragma OPTIMIZE (4)
byte sample_to_cm(const unsigned int table[], unsigned int offset, unsigned int interval, unsigned int val) {
	unsigned int lower = 0;

	while(offset > 1) {
		if(offset % 2 == 1) offset++;
		offset /= 2;
		if(val <= table[lower+offset]) {
			lower += offset;
		}
	}
	return (lower + 2) * interval + interval * (table[lower] - val) / (table[lower] - table[lower + 1]);
}
#pragma OPTIMIZE (5)

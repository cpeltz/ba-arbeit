#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "driver.h"
#include "i2c_master.h"
#include "helper.h"
#include "timing.h"
#include "lcd.h"
#include "config.h"

/** Kommando-Buffer */
byte xdata driver_buffer[DRIVER_BUFFER_SIZE];

int driver_pos_overflows = 0;
int driver_last_pos = 0;

void add_drive_param(char * str, int value) {
	strcat(str, ",");
	int2str(str + strlen(str), value);
}

static DriverStatus driver_statusStrct;

void driver_drive(signed char speedLeft, signed char speedRight, driver_trigger trigger,
		int triggerValLeft, int triggerValRight) {
	// Setze die Adresse und den Fahrmodus
	strcpy(driver_buffer, "  D");
	driver_buffer[0] = DRIVER_ADDRESS;
	driver_buffer[1] = 0xf3;

	switch (trigger) {
		case TRIGGER_NONE:
			strcat(driver_buffer, "NN");
			break;
		case TRIGGER_TIME:
			// Sanity-check
			if ((triggerValLeft  > 255) || (triggerValLeft  < 1) ||
			    (triggerValRight > 255) || (triggerValRight < 1))
				return;

			strcat(driver_buffer, "TT");
			break;
		case TRIGGER_POSITION:
			strcat(driver_buffer, "PP");
			break;
	}

	// Fuege die Parameter zur Zeichenkette hinzu
	add_drive_param(driver_buffer, speedLeft);
	add_drive_param(driver_buffer, speedRight);
	add_drive_param(driver_buffer, triggerValLeft);
	add_drive_param(driver_buffer, triggerValRight);	

//	lcd_print_string(driver_buffer + 2);
//	wait_ms(5000);
	// Befehlskette an den i²c-Bus senden
	i2c_send(driver_buffer, strlen(driver_buffer) + 1);

	driver_buffer[0] = DRIVER_ADDRESS;
	driver_buffer[1] = 'J';
	i2c_send(driver_buffer, 2);
}

void driver_driveStraight(int speed, driver_trigger trigger, int triggerVal) {
	driver_drive(speed, speed, trigger, triggerVal, triggerVal);
}

void driver_rotate(signed char speed, int degree) {
	char dir = 0;
	char speedRight = speed;
	int degreeRight;

	if (degree < 0) {
		dir = 1;
		degree = -degree;
		speedRight = -speed;
	} else {
		speed = -speed;
	}

	degree = ((degree * 20) + degree * 2) / 10;

	if (dir == 0) {
		degreeRight = degree;
		degree = -degree;
	} else
		degreeRight = -degree;

	driver_drive(speed, speedRight, TRIGGER_POSITION, degree, degreeRight);
}

void driver_stop() {
	// Fahrzeug anhalten und Befehlskette loeschen
	driver_buffer[0] = DRIVER_ADDRESS;	
	driver_buffer[1] = ' ';
	i2c_send(driver_buffer, 2);
}

void driver_pause() {
	driver_buffer[0] = DRIVER_ADDRESS;	
	driver_buffer[1] = 'p';
	i2c_send(driver_buffer, 2);
}

void driver_unpause() {
	driver_buffer[0] = DRIVER_ADDRESS;	
	driver_buffer[1] = 'P';
	i2c_send(driver_buffer, 2);
}

void driver_moveForward() {
	driver_buffer[0] = DRIVER_ADDRESS;
	driver_buffer[1] = 'w';
	i2c_send(driver_buffer, 2);
}

void driver_moveBackward() {
	driver_buffer[0] = DRIVER_ADDRESS;
	driver_buffer[1] = 's';
	i2c_send(driver_buffer, 2);
}

void driver_rotateLeft() {
	driver_buffer[0] = DRIVER_ADDRESS;
	driver_buffer[1] = 'a';
	i2c_send(driver_buffer, 2);
}

void driver_rotateRight() {
	driver_buffer[0] = DRIVER_ADDRESS;
	driver_buffer[1] = 'd';
	i2c_send(driver_buffer, 2);
}

DriverStatus *driver_status() {
	byte numRead;

	driver_buffer[0] = DRIVER_ADDRESS;
	driver_buffer[1] = 0xf0;

	/*// Position des linken Rades
	driver_buffer[2] = 1;
	i2c_send(driver_buffer, 3);
	driver_buffer[3] = DRIVER_ADDRESS;
	driver_buffer[4] = 0xff;
	numRead = i2c_receive(driver_buffer + 3, 2);
	driver_statusStrct.posLeft = driver_buffer[4] << 8;

	driver_buffer[2] = 2;
	i2c_send(driver_buffer, 3);
	driver_buffer[3] = DRIVER_ADDRESS;
	driver_buffer[4] = 0xff;
	numRead = i2c_receive(driver_buffer + 3, 2);
	driver_statusStrct.posLeft += driver_buffer[4];

	// Position des rechten Rades
	driver_buffer[2] = 3;
	i2c_send(driver_buffer, 3);
	driver_buffer[3] = DRIVER_ADDRESS;
	driver_buffer[4] = 0xff;
	numRead = i2c_receive(driver_buffer + 3, 2);
	driver_statusStrct.posRight = driver_buffer[4] << 8;

	driver_buffer[2] = 4;
	i2c_send(driver_buffer, 3);
	driver_buffer[3] = DRIVER_ADDRESS;
	driver_buffer[4] = 0xff;
	numRead = i2c_receive(driver_buffer + 3, 2);
	driver_statusStrct.posRight += driver_buffer[4];*/

	// Geschwindigkeit des linken Rades
	driver_buffer[2] = 5;
	i2c_send(driver_buffer, 3);
	driver_buffer[3] = DRIVER_ADDRESS;
	driver_buffer[4] = 0xff;
	numRead = i2c_receive(driver_buffer + 3, 2);
	driver_statusStrct.speedLeft = driver_buffer[4];

	// Geschwindigkeit des rechten Rades
	driver_buffer[2] = 6;
	i2c_send(driver_buffer, 3);
	driver_buffer[3] = DRIVER_ADDRESS;
	driver_buffer[4] = 0xff;
	numRead = i2c_receive(driver_buffer + 3, 2);
	driver_statusStrct.speedRight = driver_buffer[4];

	// Anzahl der Eintraege in der Queue
	driver_buffer[2] = 10;
	i2c_send(driver_buffer, 3);
	driver_buffer[3] = DRIVER_ADDRESS;
	driver_buffer[4] = 0xff;
	numRead = i2c_receive(driver_buffer + 3, 2);
	driver_statusStrct.mode = driver_buffer[4];

	// Tachowert links
	driver_buffer[2] = 12;
	i2c_send(driver_buffer, 3);
	driver_buffer[3] = DRIVER_ADDRESS;
	driver_buffer[4] = 0xff;
	numRead = i2c_receive(driver_buffer + 3, 2);
	driver_statusStrct.tachoLeft = driver_buffer[4] << 8;

	driver_buffer[2] = 13;
	i2c_send(driver_buffer, 3);
	driver_buffer[3] = DRIVER_ADDRESS;
	driver_buffer[4] = 0xff;
	numRead = i2c_receive(driver_buffer + 3, 2);
	driver_statusStrct.tachoLeft += driver_buffer[4];

	// Tachowert rechts
	driver_buffer[2] = 14;
	i2c_send(driver_buffer, 3);
	driver_buffer[3] = DRIVER_ADDRESS;
	driver_buffer[4] = 0xff;
	numRead = i2c_receive(driver_buffer + 3, 2);
	driver_statusStrct.tachoRight = driver_buffer[4] << 8;

	driver_buffer[2] = 15;
	i2c_send(driver_buffer, 3);
	driver_buffer[3] = DRIVER_ADDRESS;
	driver_buffer[4] = 0xff;
	numRead = i2c_receive(driver_buffer + 3, 2);
	driver_statusStrct.tachoRight += driver_buffer[4];

	return &driver_statusStrct;
}

int driver_get_pos() {
	driver_save_pos();
	return driver_pos_overflows * DRIVER_CM_PER_OVERFLOW + driver_pos_to_cm(driver_last_pos);
}

void driver_save_pos() {
	DriverStatus *ds = driver_status();
	int pos = ds->tachoLeft / 2 + ds->tachoRight / 2;
	if((driver_last_pos < 0 && driver_last_pos < (-32768 + DRIVER_OVERFLOW_DELTA) && pos > (0 - DRIVER_OVERFLOW_DELTA))
			|| (driver_last_pos > 0 && driver_last_pos > (32767 - DRIVER_OVERFLOW_DELTA) && pos < DRIVER_OVERFLOW_DELTA)) { // tick counter of motor has had overflow
		driver_pos_overflows++;
	}
	driver_last_pos = pos;
}

void driver_reset_pos() {
	driver_buffer[0] = DRIVER_ADDRESS;
	driver_buffer[1] = 116; // Send 't' to motor, resets tacho
	i2c_send(driver_buffer, 2);

	driver_pos_overflows = 0;
	driver_last_pos = 0;
}

int driver_pos_to_cm(int pos) {
	return (pos / DRIVER_TICKS_PER_CM);
}

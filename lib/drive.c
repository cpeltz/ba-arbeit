#include <inttypes.h>
#include "definitions.h"
#include "flags.h"
#include "pid.h"
#include "drive.h"
#include "motor.h"
#include "timer.h"
#include "irq.h"
#include "stdlib.h"

// lokale Flag-Variable
static uint8_t drive_flags = 0;

// und die dazu gehörigen Flags
#define DRIVE_TOGGLE            0

static pid_data_t drive_PID[2];

void drive_SetPIDParameter(	const uint8_t wheel, const int16_t Pfactor, const int16_t Ifactor,
							const int16_t Dfactor, const int16_t SErrorMAX) {
	// Läd übergebene Parameter in PID Controller
	switch (wheel) {
		case 0:
		case 1:
			pid_Init(Pfactor, Ifactor, Dfactor, SErrorMAX, &drive_PID[wheel]);
			break;
		case 2:
			drive_SetPIDParameter(0, Pfactor, Ifactor, Dfactor, SErrorMAX);
			drive_SetPIDParameter(1, Pfactor, Ifactor, Dfactor, SErrorMAX);
			break;
    }
}

// Einstellen des derzeitigen Summenfehlers
void drive_SetPIDSumError(const uint8_t wheel, const int16_t sumError) {
	switch (wheel) {
		case 0:
		case 1:
			drive_PID[wheel].sumError = sumError;
			break;
		case 2:
			drive_PID[0].sumError = sumError;
			drive_PID[1].sumError = sumError;
		break;
	}
}

// Fahren mit PID-Regler
void drive_UsePID(const uint8_t wheel, const int8_t speed) {
	int16_t wheel_ModSpeed[2];
	int16_t wheel_Difference;

	switch (wheel) {
		case 0:
			// PID Fahrt für linkes Rad
			if (speed > 0)
				motor_SetSpeed(0, pid_Controller(speed, wheel_ReadSpeed(0), &drive_PID[0]));
			else
				motor_SetSpeed(0, -pid_Controller(-speed, -wheel_ReadSpeed(0), &drive_PID[0]));
			break;
		case 1:
			// PID Fahrt für rechtes Rad
			if (speed > 0)
				motor_SetSpeed(1, pid_Controller(speed, wheel_ReadSpeed(1), &drive_PID[1]));
			else
				motor_SetSpeed(1, -pid_Controller(-speed, -wheel_ReadSpeed(1), &drive_PID[1]));
			break;
		case 2:
			// PID Fahrt für beide Räder mit Differenzausgleich
			wheel_ModSpeed[0] = wheel_ReadSpeed(0);
			wheel_ModSpeed[1] = wheel_ReadSpeed(1);
			wheel_Difference = wheel_ReadDifference();

			// bei ungerader Differenz immer abwechselnd
			if (wheel_Difference % 2) {
				if (flagLocal_ReadAndClear(&drive_flags, DRIVE_TOGGLE)) {
					wheel_ModSpeed[0] += wheel_Difference;
				} else {
					flagLocal_Set(&drive_flags, DRIVE_TOGGLE);
					wheel_ModSpeed[1] -= wheel_Difference;
				}
			}

			// geraden Differenzanteil gerecht verteilen
			wheel_ModSpeed[0] += (wheel_Difference / 2);
			wheel_ModSpeed[1] -= (wheel_Difference / 2);

			// Richtungsunterschied da PID Regler nur positiv arbeitet
			if (speed > 0) {
				// Veränderte Werte begrenzen
				if (wheel_ModSpeed[0] < 0)
					wheel_ModSpeed[0] = 0;
				else if (wheel_ModSpeed[0] > 127)
					wheel_ModSpeed[0] = 127;
				if (wheel_ModSpeed[1] < 0)
					wheel_ModSpeed[1] = 0;
				else if (wheel_ModSpeed[1] > 127)
					wheel_ModSpeed[1] = 127;

				motor_SetSpeed(0, pid_Controller(speed, wheel_ModSpeed[0], &drive_PID[0]));
				motor_SetSpeed(1, pid_Controller(speed, wheel_ModSpeed[1], &drive_PID[1]));
			} else {
				// Veränderte Werte begrenzen
				if (wheel_ModSpeed[0] < -127)
					wheel_ModSpeed[0] = -127;
				else if (wheel_ModSpeed[0] > 0)
					wheel_ModSpeed[0] = 0;
				if (wheel_ModSpeed[1] < -127)
					wheel_ModSpeed[1] = -127;
				else if (wheel_ModSpeed[1] > 0)
					wheel_ModSpeed[1] = 0;

				motor_SetSpeed(0, -pid_Controller(-speed, -wheel_ModSpeed[0], &drive_PID[0]));
				motor_SetSpeed(1, -pid_Controller(-speed, -wheel_ModSpeed[1], &drive_PID[1]));
			}
			break;
	}
}

void drive_status(global_state_t *state) {

}

void drive_break_active() {
	//TODO Implement
}
void drive_break_active_set() {
	//TODO Implement
}

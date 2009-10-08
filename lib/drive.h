#ifndef DRIVE_H
#define DRIVE_H

#include <inttypes.h>

void drive_SetPIDParameter(	const uint8_t wheel, const int16_t Pfactor, const int16_t Ifactor,
							const int16_t Dfactor, const int16_t SErrorMAX);
void drive_SetPIDSumError(const uint8_t wheel, const int16_t sumError);
void drive_UsePID(const uint8_t wheel, const int8_t speed);

void drive_brake_active(void);
void drive_brake_active_set(void);
//void drive_brake_active_set(uint8_t wheel);

void drive_brake_active_left(void);
void drive_brake_active_set_left(void);

void drive_brake_active_right(void);
void drive_brake_active_set_right(void);

#endif

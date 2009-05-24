#ifndef DRIVE_H
#define DRIVE_H

#include <inttypes.h>
#include "types.h"

void drive_SetPIDParameter(	const uint8_t wheel, const int16_t Pfactor, const int16_t Ifactor,
							const int16_t Dfactor, const int16_t SErrorMAX);
void drive_SetPIDSumError(const uint8_t wheel, const int16_t sumError);
void drive_UsePID(const uint8_t wheel, const int8_t speed);
void drive_status(global_state_t *state);

void drive_break_active();
void drive_break_active_set();

#endif

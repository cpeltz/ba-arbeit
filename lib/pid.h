#ifndef PID_H
#define PID_H

#include <inttypes.h>
#include "types.h"

void pid_Init(	const int16_t pfactor, const int16_t ifactor, const int16_t dfactor,
				const int16_t sumErrorMax, pid_data_t * pid);
uint8_t pid_Controller(const uint8_t setPoint, const int16_t processValue, pid_data_t * pid);

#endif

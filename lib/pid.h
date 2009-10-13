#ifndef PID_H
#define PID_H

#include <inttypes.h>
#include "types.h"

void pid_init(const int16_t pfactor, const int16_t ifactor, const int16_t dfactor,
              const int16_t sum_error_max, pid_data_t * pid);
uint8_t pid_controller(const uint8_t set_point, const int16_t process_value, pid_data_t * pid);

#endif

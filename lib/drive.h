#ifndef DRIVE_H
#define DRIVE_H

#include <inttypes.h>

void drive_set_pid_parameter(const uint8_t wheel, const int16_t pfactor, const int16_t ifactor,
                             const int16_t dfactor, const int16_t sum_error_max);
void drive_set_pid_sum_error(const uint8_t wheel, const int16_t sum_error);
void drive_use_pid(const uint8_t wheel, const int8_t speed);

void drive_brake_active(uint8_t wheel);
void drive_brake_active_set(uint8_t wheel);

#endif

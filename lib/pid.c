#include "pid.h"
#include "definitions.h"
#include "debug.h"

/**
 * @defgroup PID_Module PID Control
 *
 * Contains the functions to control the PID controller.
 * @{
 */

/**
 * Initializes the PID Controller with the given parameters.
 *
 * @param[in] pfactor The Proportional factor.
 * @param[in] ifactor The integral factor.
 * @param[in] dfactor The differential factor.
 * @param[in] sum_error_max The max sum of all errors.
 * @param[out] pid The structure to save the pid parameters.
 */
void pid_init(	const int16_t pfactor, const int16_t ifactor, const int16_t dfactor,
				const int16_t sum_error_max, pid_data_t * pid) {
	pid->pfactor = pfactor;
	pid->ifactor = ifactor;
	pid->dfactor = dfactor;

	pid->last_error = 0;
	pid->sum_error = 0;
	pid->sum_error_max = sum_error_max;
}

/**
 * Controls the PID values.
 * @todo Discern what this functions does.
 */
int8_t pid_controller(const int8_t set_point, const int16_t process_value, pid_data_t * pid) {
#if PID_CONTROLLER_ENABLED == 1
	int16_t error = 0;
	int16_t p_term = 0;
	int32_t i_term = 0;
	int16_t d_term = 0;
	int32_t ret = 0;

	error = set_point - process_value;

	pid->sum_error += error;

	if (DEBUG_ENABLE) {
			debug_write_integer(PSTR("pid.c : pid_controller() : set_point = "), set_point);
			debug_write_integer(PSTR("pid.c : pid_controller() : process_value = "), process_value);
			debug_write_integer(PSTR("pid.c : pid_controller() : error = "), error);
	}


	// Fehlersumme begrenzen
	if ((pid->sum_error > pid->sum_error_max))
		pid->sum_error = pid->sum_error_max;
	else if ((pid->sum_error < 0))
		pid->sum_error = 0;

	p_term = pid->pfactor * error;
	i_term = pid->ifactor * pid->sum_error;
	d_term = pid->dfactor * (error - pid->last_error);

	pid->last_error = error;

	ret = (p_term + i_term + d_term) / 100;

	// Rückgabewert begrenzen
	if ((ret > 127))
		ret = 127;
	if ((ret < 0))
		ret = 0;

	return (set_point < 0) ? -ret : ret;
#else
	return set_point;
#endif
}
/*@}*/

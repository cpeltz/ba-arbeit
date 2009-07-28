#include "pid.h"

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
 * @param[in] sumErrorMax The max sum of all errors.
 * @param[out] pid The structure to save the pid parameters.
 */
void pid_Init(	const int16_t pfactor, const int16_t ifactor, const int16_t dfactor,
				const int16_t sumErrorMax, pid_data_t * pid) {
	pid->P_Factor = pfactor;
	pid->I_Factor = ifactor;
	pid->D_Factor = dfactor;

	pid->lastError = 0;
	pid->sumError = 0;
	pid->sumError_Max = sumErrorMax;
}

/**
 * Controls the PID values.
 * @todo Discern what this functions does.
 */
uint8_t pid_Controller(const uint8_t setPoint, const int16_t processValue, pid_data_t * pid) {
	int16_t error = 0;
	int16_t p_term = 0;
	int32_t i_term = 0;
	int16_t d_term = 0;
	int32_t ret = 0;

	error = setPoint - processValue;

	pid->sumError += error;

	// Fehlersumme begrenzen
	if ((pid->sumError > pid->sumError_Max))
		pid->sumError = pid->sumError_Max;
	else if ((pid->sumError < 0))
		pid->sumError = 0;

	p_term = pid->P_Factor * error;
	i_term = pid->I_Factor * pid->sumError;
	d_term = pid->D_Factor * (error - pid->lastError);

	pid->lastError = error;

	ret = (p_term + i_term + d_term) / 100;

	// Rückgabewert begrenzen
	if ((ret > 127))
		ret = 127;
	if ((ret < 0))
		ret = 0;

	return ret;
}
/*@}*/

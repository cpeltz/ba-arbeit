#include "pid.h"

void pid_Init(	const int16_t pfactor, const int16_t ifactor, const int16_t dfactor,
				const int16_t sumErrorMax, pid_data_t * pid) {
	pid->P_Factor = pfactor;
	pid->I_Factor = ifactor;
	pid->D_Factor = dfactor;

	pid->lastError = 0;
	pid->sumError = 0;
	pid->sumError_Max = sumErrorMax;
}

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

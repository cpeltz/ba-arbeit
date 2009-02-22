#ifndef PID_H
#define PID_H

typedef struct PID_DATA
{
  int16_t lastError;
  int16_t sumError;
  int16_t sumError_Max;
  int16_t P_Factor;
  int16_t I_Factor;
  int16_t D_Factor;
} pid_data_t;

void pid_Init(const int16_t pfactor, const int16_t ifactor, const int16_t dfactor,
              const int16_t sumErrorMax, pid_data_t * pid);
uint8_t pid_Controller(const uint8_t setPoint, const int16_t processValue, pid_data_t * pid);

#endif

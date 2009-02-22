#ifndef IRQ_H
#define IRQ_H

typedef struct TACHO
{
  int16_t left;
  int16_t right;
  int16_t difference;
} tacho_t;

void irq_Init(void);
int16_t wheel_ReadPosition(uint8_t wheel);
void wheel_ClearPosition(uint8_t wheel);
int16_t wheel_ReadDifference(void);
void wheel_WriteDifference(int16_t difference);
void wheel_ClearDifference(void);
void trigger_Set_P(const uint8_t wheel, const int16_t trigger_position);
void trigger_Clear_P(const uint8_t wheel);
void wheel_GetTacho(tacho_t * tacho);
void wheel_ClearTacho(void);

#endif

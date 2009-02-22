#include <avr/io.h>
#include <inttypes.h>
#include <stdlib.h>
#include "definitions.h"
#include "motor.h"
#include "flags.h"

static int8_t motor_speed[2];

static void
motor_RunForward_M0(void)
{
  INPUT_PORT |= (1 << INPUT2);
  INPUT_PORT &= ~(1 << INPUT1);
}

static void
motor_RunReverse_M0(void)
{
  INPUT_PORT &= ~(1 << INPUT2);
  INPUT_PORT |= (1 << INPUT1);
}

static void
motor_Break_M0(void)
{
  INPUT_PORT |= (1 << INPUT1) | (1 << INPUT2);
  ENABLE_A_PWM = 255;
}

static void
motor_RunForward_M1(void)
{
  INPUT_PORT &= ~(1 << INPUT4);
  INPUT_PORT |= (1 << INPUT3);
}

static void
motor_RunReverse_M1(void)
{
  INPUT_PORT |= (1 << INPUT4);
  INPUT_PORT &= ~(1 << INPUT3);
}

static void
motor_Break_M1(void)
{
  INPUT_PORT |= (1 << INPUT3) | (1 << INPUT4);
  ENABLE_B_PWM = 255;
}

void
motor_Init(void)
{
  // Ausgänge für L298 Motortreiber initialisieren
  INPUT_DDR |= (1 << INPUT1) | (1 << INPUT2) | (1 << INPUT3) | (1 << INPUT4);
  INPUT_PORT &= ~((1 << INPUT1) | (1 << INPUT2) | (1 << INPUT3) | (1 << INPUT4));

  ENABLE_A_DDR |= (1 << ENABLE_A);
  ENABLE_A_PORT &= ~(1 << ENABLE_A);

  ENABLE_B_DDR |= (1 << ENABLE_B);
  ENABLE_B_PORT &= ~(1 << ENABLE_B);

  // Timer für PWM an ENABLE-Ausgängen initialisieren
  TCCR0A = (1 << COM0A1) | (1 << COM0B1) | (1 << WGM00);
  TCCR0B = (3 << CS00);

  // Timer 0:
  // PWM, Phase correct, Prescaler clkIO/64
  // Clear OC0A / OC0B on compare match when up-counting. Set OC0A / OC0B
  // on compare match when downcounting.
  ENABLE_A_PWM = 0;
  ENABLE_B_PWM = 0;
}

static uint8_t
motor_CalculatePWM(int8_t speed)
    // Wenn speed = 0 -> PWM = 0,
    // 1 < speed < 38 -> PWM = speed + 38,
    // 38 < speed -> PWM = (speed * 2) + 1.
{
  if ((speed < 1))
    {
      return 0;
    }
  else if ((speed < 38))
    {
      return (speed + 38);
    }
  else
    {
      return ((speed * 2) + 1);
    }
}

static void
motor_SetSpeed_M0(int8_t speed)
{
  uint8_t positive;

  positive = abs(speed);
  if (!(speed))
    {
      ENABLE_A_PWM = 0;
    }
  else
    {
      switch ((speed / positive))
        {
        case 1:
          motor_RunForward_M0();
          break;
        case -1:
          motor_RunReverse_M0();
          break;
        }
      ENABLE_A_PWM = motor_CalculatePWM(positive);
    }
}

static void
motor_SetSpeed_M1(int8_t speed)
{
  uint8_t positive;

  positive = abs(speed);
  if (!(speed))
    {
      ENABLE_B_PWM = 0;
    }
  else
    {
      switch ((speed / positive))
        {
        case 1:
          motor_RunForward_M1();
          break;
        case -1:
          motor_RunReverse_M1();
          break;
        }
      ENABLE_B_PWM = motor_CalculatePWM(positive);
    }
}

void
motor_SetSpeed(uint8_t motor, int8_t speed)
{
  switch (motor)
    {
    case 0:
      motor_speed[0] = speed;
      motor_SetSpeed_M0(speed);
      break;
    case 1:
      motor_speed[1] = speed;
      motor_SetSpeed_M1(speed);
      break;
    case 2:
      motor_speed[0] = speed;
      motor_speed[1] = speed;
      motor_SetSpeed_M0(speed);
      motor_SetSpeed_M1(speed);
      break;
    }
}

int8_t
motor_ReadSpeed(uint8_t motor)
{
  return motor_speed[motor];
}

void
motor_Break(uint8_t motor)
{
  switch (motor)
    {
    case 0:
      motor_Break_M0();
      break;
    case 1:
      motor_Break_M1();
      break;
    case 2:
      motor_Break_M0();
      motor_Break_M1();
      break;
    }
}

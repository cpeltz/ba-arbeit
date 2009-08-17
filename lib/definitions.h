#ifndef DEFINITIONS_H
#define DEFINITIONS_H
/**
 * @defgroup DEFINITIONS_Module Definitions
 * This module contains the constant definitions
 * which are used throughout the system.
 * @{
 */

/** \file definitions.h
 *	This file holds many global definitions used through the program.
 */

#define VERSION "Ver. 3.2.0.0000"

#define INPUT_DDR               DDRB
#define INPUT_PORT              PORTB
#define INPUT1                  PB0
#define INPUT2                  PB1
#define INPUT3                  PB2
#define INPUT4                  PB3

#define ENABLE_A_DDR            DDRB
#define ENABLE_A_PORT           PORTB
#define ENABLE_A                PB7
#define ENABLE_A_PWM            OCR0A

#define ENABLE_B_DDR            DDRG
#define ENABLE_B_PORT           PORTG
#define ENABLE_B                PG5
#define ENABLE_B_PWM            OCR0B

// Definitionen für Drehgeber
#define IRQ_DDR                 DDRE
#define IRQ_PORT                PORTE
#define IRQ_PIN                 PINE
#define IRQ_A0                  PE4
#define IRQ_B0                  PE5
#define IRQ_A1                  PE6
#define IRQ_B1                  PE7

/**
 * Flag set every 100ms.
 */
#define TIMER_100MS             1
/**
 * Flag set every 262ms.
 */
#define TIMER_262MS             2

// LEDs
#define LED_DDR                 DDRG
#define LED_PORT                PORTG
#define LED_RED1                PG0
#define LED_RED1_BIT            0
#define LED_RED2                PG1
#define LED_RED2_BIT            3
#define LED_ORANGE              PG2
#define LED_ORANGE_BIT          6
#define LED_GREEN               PG3
#define LED_GREEN_BIT           9
#define LED_BLUE                PG4
#define LED_BLUE_BIT            12
#define OFF                     0
#define ON                      1
#define FLASHSLOW               2
#define FLASHFAST               3
#define SINGLE                  4
#define SINGLEOFF               5

// DIP
#define DIP_PORT                PORTD
#define DIP_PIN                 PIND
#define DIP_DDR                 DDRD
#define DIP1                    PD4
#define DIP2                    PD5
#define DIP3                    PD6
#define DIP4                    PD7

// Wheels
/**
 * Define to identify the left wheel.
 */
#define WHEEL_LEFT              0
/**
 * Define to identify the right wheel.
 */
#define WHEEL_RIGHT             1
/**
 * Convinience define to use in functions, which can operate on both wheels.
 */
#define WHEEL_BOTH              2

/*@}*/
#endif

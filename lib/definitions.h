#ifndef DEFINITIONS_H
#define DEFINITIONS_H

// Definitionen für Motortreiber
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

// Flaggen (0 - 31)
#define TIMER_1MS               0
#define TIMER_10MS              1
#define TIMER_100MS             2
#define TIMER_262MS             3
#define UART_RX_OVERFLOW        5
#define PARSE_COMMAND_COMPLETE  6

#define T_TRIGGER_L             7
#define T_TRIGGER_R             8
#define P_TRIGGER_L             9
#define P_TRIGGER_R             10

#define INTERFACE_UART          11
#define INTERFACE_TWI           12
#define DEBUG_ENABLE            13
#define HALT_DRIVE              14
#define LCD_PRESENT             15
#define RESET_TWI               16

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

#endif

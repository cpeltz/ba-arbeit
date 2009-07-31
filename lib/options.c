#include "options.h"
#include <inttypes.h>

/**
 * The speed with which position-correction during active braking will occur.
 *
 * Valid values are 1 to 127. NEVER use anything outside
 * this range, as it will break the active braking feature.
 * When in active brake mode the system will try to hold it's
 * last known position. If it is deviated it will use it's wheels
 * with a speed equal to this constant to correct it.
 */
uint8_t ACTIVE_BRAKE_AMOUNT = ACTIVE_BRAKE_AMOUNT_DEFAULT;

/**
 * Enables or disables the entire active braking feature.
 *
 * May be set through the user with the option instruction.
 * @todo maybe use the global flag storage 
 */
uint8_t ACTIVE_BRAKE_ENABLE = 1;

/**
 * Enables the active braking feature for single wheels, if a the trigger for
 * it has been reached.
 *
 * Only has effect when ACTIVE_BRAKE_ENABLE is set to 1.
 */
uint8_t ACTIVE_BRAKE_WHEN_TRIGGER_REACHED = 1;

/**
 * Enables the active brake feature for both wheels, when no order is beeing processed.
 *
 * Only has effect when ACTIVE_BRAKE_ENABLE is set to 1.
 */
uint8_t ACTIVE_BRAKE_WHEN_IDLE = 1;

uint8_t DEBUG_ENABLE;
uint8_t INTERFACE_TWI;
uint8_t LCD_PRESENT;

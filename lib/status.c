#include "status.h"
#include "motor.h"

/**
 * @defgroup STATUS_Module Status Module
 * Defines functions used to get various status information
 * about the system.
 * @{
 */

/**
 * Global status variable.
 */
global_state_t global_state;

/**
 * Keeps the global status up to date.
 */
void status_update() {
	motor_status( &global_state );
	drive_status( &global_state );
}

/**
 * Sets the global status to beginning values.
 */
void status_init(void) {
	global_state.state_left = 0;
	global_state.state_right = 0;
	global_state.speed_left = 0;
	global_state.speed_right = 0;
	global_state.position_left = 0;
	global_state.position_right = 0;
	global_state.difference = 0;
	global_state.flags = 0;
}

/**
 * Reads the current global status.
 *
 * @param[out] status The status variable where the state will be saved in.
 */
void status_read(global_state_t * status) {
	*status = global_state;
}
/*@}*/

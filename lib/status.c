#include "status.h"
#include "motor.h"

global_state_t global_state;

void status_update() {
	motor_status( &global_state );
	drive_status( &global_state );
}

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

void status_read(global_state_t * status) {
	*status = global_state;
}

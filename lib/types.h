#ifndef _TYPES_H_
#define _TYPES_H_
#include <inttypes.h>
#include "options.h"

/* Global storage for the different Types,
 * like fifo's, order's, and stati.
 */

// TODO wth is TACHO ?
// TODO Flagged for DELETION
typedef struct TACHO {
	int16_t left;
	int16_t right;
	int16_t difference;
} tacho_t;

typedef struct PID_DATA {
	int16_t lastError;
	int16_t sumError;
	int16_t sumError_Max;
	int16_t P_Factor;
	int16_t I_Factor;
	int16_t D_Factor;
} pid_data_t;

typedef struct GLOBAL_STATE {
	uint8_t state_left;			// Unused
	uint8_t state_right;		// Unused
	int8_t speed_left;
	int8_t speed_right;
	int16_t position_left;
	int16_t position_right;
	int16_t difference;			//Unused
	uint8_t flags;				//Unused
} global_state_t;

typedef struct ORDER {
	uint8_t data[ORDER_TYPE_MAX_LENGTH];
	uint8_t status;
} order_t;

#endif

#ifndef _TYPES_H_
#define _TYPES_H_
#include <inttypes.h>
#include "options.h"

/**
 * @defgroup TYPES_Module Types
 * Global storage for the different Types,
 * like fifo's, order's, and state.
 * @{
 */

/**
 * Storage for the PID parameters and state.
 */
typedef struct PID_DATA {
	int16_t lastError;    /**< Do not know */
	int16_t sumError;     /**< The sum of all errors*/
	int16_t sumError_Max; /**< The maximal sum of all errors*/
	int16_t P_Factor;     /**< The proportional factor*/
	int16_t I_Factor;     /**< The integral factor*/
	int16_t D_Factor;     /**< The differential factor*/
} pid_data_t;

/**
 * Storage for global state information.
 */
typedef struct GLOBAL_STATE {
	uint8_t state_left;     /**< Unused*/
	uint8_t state_right;    /**< Unused*/
	int8_t speed_left;      /**< Speed of the left wheel*/
	int8_t speed_right;     /**< Speed of the right wheel*/
	int16_t position_left;  /**< Position of the left wheel*/
	int16_t position_right; /**< Position of the right wheel*/
	int16_t difference;     /**< Unused*/
	uint8_t flags;          /**< Unused*/
} global_state_t;

/**
 * The elemental order. Length may be adjusted through #ORDER_TYPE_MAX_LENGTH.
 */
typedef struct ORDER {
	uint8_t data[ORDER_TYPE_MAX_LENGTH]; /**< Hold the actual order data*/
	uint8_t status;                      /**< Stores different states of an order*/
} order_t;

/*@}*/
#endif

#ifndef _TYPES_H_
#define _TYPES_H_

#include <inttypes.h>
#include "options.h"

/**
 * @defgroup TYPES_Module Types
 * Global storage for the different Types.
 * 
 * @{
 */

/**
 * Storage for the PID parameters and state.
 */
typedef struct PID_DATA {
	int16_t last_error;    /**< Do not know */
	int16_t sum_error;     /**< The sum of all errors*/
	int16_t sum_error_max; /**< The maximal sum of all errors*/
	int16_t pfactor;     /**< The proportional factor*/
	int16_t ifactor;     /**< The integral factor*/
	int16_t dfactor;     /**< The differential factor*/
} pid_data_t;

/**
 * The elemental order. Length may be adjusted through #ORDER_TYPE_MAX_LENGTH.
 */
typedef struct ORDER {
	uint8_t data[ORDER_TYPE_MAX_LENGTH]; /**< Hold the actual order data*/
	uint8_t status;                      /**< Stores different states of an order*/
} order_t;

/*@}*/
#endif

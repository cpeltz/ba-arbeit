#ifndef _TYPES_H_
#define _TYPES_H_
#include <inttypes.h>

/* Global storage for the different Types,
 * like fifo's, order's, and stati.
 */

typedef struct FIFO_BUFFER {
	uint8_t volatile count;
	uint8_t size;
	uint8_t *p_read;
	uint8_t *p_write;
	uint8_t read2end, write2end;
} fifo_t;

typedef struct TACHO {
	int16_t left;
	int16_t right;
	int16_t difference;
} tacho_t;

typedef struct DRIVE_ORDER {
	uint8_t command;
	int16_t parameter[4];
} drive_order_t;

typedef struct PARSE_STATUS {
	uint8_t command;
	uint8_t parameter[7];
} parse_status_t;

typedef struct PID_DATA {
	int16_t lastError;
	int16_t sumError;
	int16_t sumError_Max;
	int16_t P_Factor;
	int16_t I_Factor;
	int16_t D_Factor;
} pid_data_t;

typedef struct GLOBAL_STATE {
	uint8_t state_left;
	uint8_t state_right;
	int8_t speed_left;
	int8_t speed_right;
	int16_t position_left;
	int16_t position_right;
	int16_t difference;
	drive_order_t drive_order;
	uint8_t flags;
} global_state_t;

#define ORDER_TYPE_MAX_LENGTH 32

typedef struct ORDER {
	uint8_t data[ORDER_TYPE_MAX_LENGTH];
	uint8_t status;
} order_t;

#endif

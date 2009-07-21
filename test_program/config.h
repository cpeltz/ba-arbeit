#ifndef __CONFIG_H__
#define __CONFIG_H__
															    
#define PROTO_MAX_RECURSION_DEPTH	10		// max. recursion depth for conditions
#define PROTO_RECV_BUFFER_SIZE		2000	// Size of the reciving buffer for protocol commands
#define PROTO_SEND_BUFFER_SIZE		32		// Size of the send buffer for protocol answers
#define I2C_MAX_ATTEMPTS			3		// max. tries for a connect
#define DRIVER_ADDRESS				42		// I²C bus address for the drive module
#define DRIVER_BUFFER_SIZE			40		// The message buffer size for driver messages (I²C)
#define DRIVER_TICKS_PER_CM			11		// How many ticks are one cm?
#define DRIVER_CM_PER_OVERFLOW		2730	// How many cm were driven when the tick counter has had an overflow?
#define DRIVER_OVERFLOW_DELTA		2000	// Delta for overflow calculation

#endif
#ifndef IO_H
#define IO_H

/**
 * Defines the size of the incoming Buffer.
 *
 * The size is measured in bytes and defines how big
 * the buffer for incoming data is.
 */
#define IO_INBUFFER_SIZE 256
/**
 * Defines the size of the out_buffer and obj_buffer Arrays.
 *
 * The size is measured in bytes and defines how big
 * the buffer for outgoing data is. The same number
 * of bytes is also used as size for the obj_buffer.
 */
#define IO_OUTBUFFER_SIZE 256

/**
 * The speed with which position-correction during active braking will occur.
 *
 * Valid values are 1 to 127. NEVER use anything outside
 * this range, as it will break the active braking feature.
 * When in active brake mode the system will try to hold it's
 * last known position. If it is deviated it will use it's wheels
 * with a speed equal to this constant to correct it.
 */
#define DRIVE_ACTIVE_BRAKE_AMOUNT 10

/**
 * Number of distinct orders the parser can buffer.
 *
 * Beware that the byte count for this is (ORDER_TYPE_MAX_LENGTH + 1) * PARSER_ORDER_BUFFER_SIZE
 * and can become quite large.
 */
#define PARSER_ORDER_BUFFER_SIZE 5

/**
 * Number of bytes for order-data.
 *
 * The real byte count for the order type is this constant + 1
 * as there is one additional byte for status keeping.
 * This number solely specifies how many bytes a order may have
 * at most.
 * Beware that this should _never_ be set to 0 or less.
 * Best practice is to set it to the max value bytes_needed() in parser.c
 * will return.
 */
#define ORDER_TYPE_MAX_LENGTH 15

/**
 * Defines the size of the order queue.
 *
 * The byte count here is QUEUE_SIZE * (ORDER_TYPE_MAX_LENGTH + 1)
 * and may become rather large very fast.
 */
#define QUEUE_SIZE  10

/**
 * The I²C Bus Slave Address.
 */
#define TWI_ADDRESS 84

/**
 * The Baud rate for the UART Interface.
 */
#define UART_BAUD_RATE 115200L

#endif

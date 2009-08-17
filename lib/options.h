#ifndef OPTIONS_H
#define OPTIONS_H

/**
 * @defgroup OPTIONS_Module Options
 * This Module contains define-options which influence
 * many other modules.
 * @{
 */

/**
 * Specifies the default value for ACTIVE_BRAKE_AMOUNT.
 *
 * See the Description there for more information. The default
 * value will be used after a reset or simple when the device
 * was started. Although it will be used, after a wrong value
 * was set through the option_instruction function.
 */
#define ACTIVE_BRAKE_AMOUNT_DEFAULT 40

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
 *
 * Beware, that this is not the real Address. Because of how the Hardware
 * handles the address register for the I2C-Bus, the real Address (42)
 * has to be bitshifted to the left by 1. Thats the 84.
 * THE REAL ADDRESS IS 42!
 */
#define TWI_ADDRESS 84

/**
 * The Baud rate for the UART Interface.
 */
#define UART_BAUD_RATE 57600L

#ifndef DEBUG
	#define DEBUG_ENABLE 0
#endif

/*@}*/
#endif

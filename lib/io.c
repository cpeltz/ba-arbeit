#include "io.h"
#include "twi.h"
#include "definitions.h"
#include "uart.h"
#include "options.h"
#include "led.h"
#include "pin.h"
#include <avr/pgmspace.h>

/**
 * \defgroup IO_Module IO Module
 * This Module is an abstraction layer between
 * the outgoing/incomming interfaces and the rest
 * of the System.
 * @{
 */

/**
 * Stores every byte recieved, until it gets fetched.
 */
static uint8_t in_buffer[IO_INBUFFER_SIZE];
/**
 * Stores every byte ment for transmission.
 */
static uint8_t out_buffer[IO_OUTBUFFER_SIZE];
/**
 * Holds the Object boundaries.
 *
 * An Object is always a unit transmitted at once. Object may be as long as 1 byte or IO_OUTBUFFER_SIZE.
 */
static uint8_t obj_memory[IO_OUTBUFFER_SIZE];
/**
 * Used for maintaining the in_buffer ring buffer.
 */
uint8_t inpos_begin = 0, inpos_end = 1;
/**
 * Used for maintaining the out_buffer ring buffer.
 */
uint8_t outpos_begin = 0, outpos_end = 1;
/**
 * Used for maintaining the obj_buffer ring buffer.
 */
uint8_t objpos_begin = 0, objpos_end = 1;

/**
 * Offset from beginning of the object, which is currently transmitting.
 *
 * This is used to know which byte should be transmitted next and whether
 * or not a object was transmitted completely.
 */
static uint8_t transmission_offset = 0;

/**
 * Setup-Function for IO related subsystems
 */
void io_init(void) {
	led_init();
	twi_init();
	uart_init();
	led_switch(LED_RED1, ON);
}

/**
 * Returns how much bytes are still available in the incoming buffer.
 *
 * @return <em>uint8_t</em> Returns the number of available bytes in the in_buffer.
 */
uint8_t io_get_free_buffer_size(void) {
	if (inpos_begin == inpos_end)
		return 0;
	else if (inpos_begin < inpos_end)
		return IO_INBUFFER_SIZE - (inpos_end - 1 - inpos_begin);
	else // begin > end
		return inpos_begin - inpos_end - 1;
}

/**
 * Fetches the next recieved byte from the buffer.
 *
 * @param [out] value Pointer to the variable in which the new byte shoulw be written.
 * @return <em>uint8_t</em> Returns either 1, on success, or 0 otherwise.
 */
uint8_t io_get(uint8_t* value) {
	pin_set_C(6);
	uint8_t temp = (inpos_begin + 1);
	if (temp == inpos_end)
		return 0;
	pin_clear_C(6);
	pin_set_C(7);
	*value = in_buffer[inpos_begin];
	pin_clear_C(7);
	pin_set_C(1);
	inpos_begin = temp;
	pin_clear_C(1);
	return 1;
}

/**
 * Puts one byte into the transmission buffer.
 *
 * @param [in] value The byte one want's to transmit.
 * @return <em>uint8_t</em> Returns 1 on success, 0 otherwise.
 */
uint8_t io_put(uint8_t value) {
	if (outpos_end == outpos_begin)
		return 0;
	out_buffer[outpos_end - 1] = value;
	outpos_end = (outpos_end + 1) % IO_OUTBUFFER_SIZE;
	return 1;
}

/**
 * Function used by the I²C ISR to put a byte in the in_buffer.
 *
 * @param value The byte pushed into the in_buffer.
 */
void _io_push(uint8_t value) {
	if (inpos_end == inpos_begin)
		return;
	in_buffer[inpos_end - 1] = value;
	inpos_end = (inpos_end + 1) % IO_INBUFFER_SIZE;
}

/**
 * Query function to get the next byte of the object, which should be transmitted.
 *
 * @return <em>uint8_t</em> Returns the next byte which should be transmitted.
 * 		If there isn't such a byte the function will return 0xff. (which is also
 *		a valid byte).
 */
uint8_t io_get_next_transmission_byte(void) {
	if ((outpos_begin + 1) % IO_OUTBUFFER_SIZE == outpos_end)
		return 0xff;
	if (transmission_offset >= io_obj_get_current_size())
		return 0xff;
	return out_buffer[outpos_begin + transmission_offset++];
}

/**
 * Returns the number of bytes the current object has.
 *
 * @return <em>uint8_t</em> Returns the number of bytes of the current object. 0 if there isn't an object.
 */
uint8_t io_obj_get_current_size() {
	uint8_t obj_end = obj_memory[objpos_begin];
	if((objpos_begin + 1) % IO_OUTBUFFER_SIZE == objpos_end)
		return 0;
	if(obj_end > outpos_begin)
		return obj_end - outpos_begin;
	return IO_OUTBUFFER_SIZE - (outpos_begin - obj_end);
}

/**
 * Querys the number of bytes of the object still remaining to be transmitted.
 *
 * @return <em>uint8_t</em> Number of Bytes of the object still to be transmitted.
 */
uint8_t io_obj_get_remaining_size() {
	return io_obj_get_current_size() - transmission_offset;
}

/**
 * Removes the current object from the buffers.
 */
void io_obj_remove_current(void) {
	if ((objpos_begin + 1) % IO_OUTBUFFER_SIZE == objpos_end)
		return;
	outpos_begin = obj_memory[objpos_begin];
	objpos_begin = (objpos_begin + 1) % IO_OUTBUFFER_SIZE;
	transmission_offset = 0;
}

/**
 * Marks the beginning of an Object.
 *
 * When putting something in the out_buffer with io_put(), one should always call io_obj_start() first
 * and after finishing using io_put(), io_obj_end() has to be called.
 * @todo Function has to be rewritten.
 */
void io_obj_start(void) {
//	if (obj_memory[objpos_end - 1] != outpos_end - 1)
//		io_obj_end();
}

/**
 * Ends the current Object.
 *
 * When putting something in the out_buffer with io_put(), one should always call io_obj_start() first
 * and after finishing using io_put(), io_obj_end() has to be called.
 */
void io_obj_end(void) {
	extern uint8_t INTERFACE_TWI;
	obj_memory[objpos_end - 1] = outpos_end - 1;
	objpos_end = (objpos_end + 1) % IO_OUTBUFFER_SIZE;
	if (!INTERFACE_TWI)
		uart_start_transmission();
}

/**
 * Simple function to reset the transmission_offset and with that the transmission status.
 */
void io_reset_transmission_status(void) {
	transmission_offset = 0;
}

/**
 * Used to find out the number of Objects still in the buffer.
 *
 * @return <em>uint8_t</em> The count of the Objects still in the Buffer.
 */
uint8_t io_obj_remaining(void) {
	if (objpos_begin > objpos_end)
		return IO_OUTBUFFER_SIZE - (objpos_begin - objpos_end) - 1;
	else
		return (objpos_end - 1) - objpos_begin;
}

/*@}*/

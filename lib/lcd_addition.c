#include "lcd_addition.h"
#include "lcd.h"
#include "order.h"
#include "queue.h"
#include <inttypes.h>
#include <stdlib.h>

const char *version = "Ver. 2.9.20090810";

/**
 * Helper function to convert an integer to a string in hex notation.
 *
 * @param[in] value The integer, which should be converted.
 * @param[out] buffer The string in which the characters will be written, must be at least 3.
 * @param[in] size The size of the buffer, has to be at least 3 big, and more then
 * 3 won't be used.
 */
void itoa_hex(uint8_t value, char *buffer, uint8_t size) {
	uint8_t lower = value & 0x0f;
	uint8_t upper = (value & 0xf0) >> 4;
	if( size <= 2)
		return;
	buffer[0] = (upper < 10) ? ('0' + upper) : ('a' + (upper-10));
	buffer[1] = (lower < 10) ? ('0' + lower) : ('a' + (lower-10));
	buffer[2] = '\0';
}

#include "io.h"

/**
 * Prints current status informations on the LCD.
 *
 * Will only be updated if the current order changes.
 */
void lcd_print_status(const order_t * const order) {
	extern uint8_t ACTIVE_BRAKE_ENABLE;
	extern uint8_t ACTIVE_BRAKE_WHEN_IDLE;
	extern uint8_t ACTIVE_BRAKE_WHEN_TRIGGER_REACHED;
	extern uint8_t INTERFACE_TWI;
	extern uint8_t DEBUG_ENABLE;
	char buffer[3];
	lcd_clrscr();
	lcd_puts(version);
	lcd_gotoxy(0,1);
	INTERFACE_TWI ? lcd_puts("TWI ") : lcd_puts("twi ");
	DEBUG_ENABLE ? lcd_puts("DEBUG ") : lcd_puts("debug ");
	ACTIVE_BRAKE_ENABLE ? lcd_puts("AB:E") : lcd_puts("AB:e");
	ACTIVE_BRAKE_WHEN_IDLE ? lcd_puts("I") : lcd_puts("i");
	ACTIVE_BRAKE_WHEN_TRIGGER_REACHED ? lcd_puts("T") : lcd_puts("t");
	if(order != NULL) {
		lcd_gotoxy(0,2);
		int length = order_size(order);
		for(int i=0; i < length; i++) {
			itoa_hex(order->data[i], buffer, 3);
			lcd_puts(buffer);
			if(i == 6)
				lcd_gotoxy(0,3);
			else
				lcd_puts(" ");
		}
	}
/*	lcd_gotoxy(0,2);
	itoa_hex(io_obj_remaining(), buffer, 3);
	lcd_puts(buffer);
	lcd_puts(" Objects");
*/
}

void lcd_update_screen(void) {
	static order_t *order = NULL;
	order_t *current = queue_get_current_order();
	if(order != current) {
		lcd_print_status(current);
		order = current;
	}

}

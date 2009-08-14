#include "lcd_addition.h"
#include "lcd.h"
#include "order.h"
#include "queue.h"
#include <inttypes.h>
#include <stdlib.h>

const char *version = "Ver. 2.9.20090813";

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

#include <string.h>

char info[4][21];
uint8_t info_col = 0, info_row = 0;
void lcd_update_info(const order_t * const order) {
	extern uint8_t ACTIVE_BRAKE_ENABLE;
	extern uint8_t ACTIVE_BRAKE_WHEN_IDLE;
	extern uint8_t ACTIVE_BRAKE_WHEN_TRIGGER_REACHED;
	extern uint8_t INTERFACE_TWI;
	extern uint8_t DEBUG_ENABLE;
	char buffer[3];
	lcd_clrscr();
	info[0][0] = '\0';
	info[0] = strcat(info[0], version);
	INTERFACE_TWI ? info[1] = strcat(info[1], "TWI ") : info[1] = strcat(info[1], "twi ");
	DEBUG_ENABLE ? info[1] = strcat(info[1], "DEBUG ") : info[1] = strcat(info[1], "debug ");
	ACTIVE_BRAKE_ENABLE ?  info[1] = strcat(info[1], "AB:E") : info[1] = strcat(info[1], "AB:e");
	ACTIVE_BRAKE_WHEN_IDLE ?  info[1] = strcat(info[1], "I") : info[1] = strcat(info[1], "i");
	ACTIVE_BRAKE_WHEN_TRIGGER_REACHED ?  info[1] = strcat(info[1], "T") : info[1] = strcat(info[1], "t");
	if(order != NULL) {
		uint8_t length = order_size(order);
		uint8_t row = 2;
		length = (length > 13) ? 13 : length;
		for(uint8_t i=0; i < length; i++) {
			itoa_hex(order->data[i], buffer, 3);
			info[row] = strcat(info[2], buffer);
			lcd_puts(buffer);
			if(i == 6 || i == 13)
				row = 3;
			else
				info[row] = strcat(info[2], " ");
		}
	}
	info_col = info_row = 0;
}

void lcd_update_screen(void) {
	static order_t *order = NULL;
	order_t *current = queue_get_current_order();
	if(order != current) {
		lcd_update_info(current);
		order = current;
	}
	if(info_col != 20 && info_row != 3) {
		if(lcd_read(0) & (1 << LCD_BUSY)) {
			return;
		} else {
			if(info[info_row][info_col])
				lcd_write(info[info_row][info_col], 1);
			else
				info_col = 20;
			if(info_col < 20)
				info_col++;
			else if (info_row < 3) {
				info_row++;
				info_col = 0;
			}
		}
	}
}

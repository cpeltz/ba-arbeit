#include "lcd_addition.h"
#include "lcd.h"
#include "order.h"
#include "queue.h"
#include <inttypes.h>
#include <stdlib.h>
#include "pin.h"

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

char info[4][21]={"Ver. 3.0.0.0000","","",""};
uint8_t info_col = 0, info_row = 0;
uint8_t lcd_update_underway = 0;

void lcd_update_info(const order_t * const order) {
	extern uint8_t ACTIVE_BRAKE_ENABLE;
	extern uint8_t ACTIVE_BRAKE_WHEN_IDLE;
	extern uint8_t ACTIVE_BRAKE_WHEN_TRIGGER_REACHED;
	extern uint8_t INTERFACE_TWI;
	extern uint8_t DEBUG_ENABLE;
	lcd_update_underway = 1;
	lcd_clrscr();
	if(INTERFACE_TWI) {
		info[1][0] = 'T';
		info[1][1] = 'W';
		info[1][2] = 'I';
	} else {
		info[1][0] = 't';
		info[1][1] = 'w';
		info[1][2] = 'i';
	}
	info[1][3] = ' ';
	if(DEBUG_ENABLE) {
		info[1][4] = 'D';
		info[1][5] = 'E';
		info[1][6] = 'B';
		info[1][7] = 'U';
		info[1][8] = 'G';
	} else {
		info[1][4] = 'd';
		info[1][5] = 'e';
		info[1][6] = 'b';
		info[1][7] = 'u';
		info[1][8] = 'g';
	}
	info[1][9] = ' ';
	info[1][10] = 'A';
	info[1][11] = 'B';
	info[1][12] = ':';
	info[1][13] = ACTIVE_BRAKE_ENABLE ? 'E' : 'e';
	info[1][14] = ACTIVE_BRAKE_WHEN_IDLE ? 'I' : 'i';
	info[1][15] = ACTIVE_BRAKE_WHEN_TRIGGER_REACHED ? 'T' : 't';
	info[1][16] = '\0';
	info[2][0] = '\0';
	info[3][0] = '\0';
	if(order != NULL) {
		uint8_t row = 2, col = 0;
		uint8_t length = order_size(order);
		length = (length > 13) ? 13 : length;
		for(uint8_t i=0; i < length; i++) {
			uint8_t lower = order->data[i] & 0x0f;
			uint8_t upper = (order->data[i] & 0xf0) >> 4;
			info[row][col++] = (upper < 10) ? ('0' + upper) : ('a' + (upper-10));
			info[row][col++] = (lower < 10) ? ('0' + lower) : ('a' + (lower-10));
			if(i == 6 || i == 13) {
				info[row][col] = '\0';
				row = 3;
				col = 0;
			} else
				info[row][col++] = ' ';
		}
		info[row][col] = '\0';
	}
	info_col = info_row = 0;
}

void lcd_update_screen(void) {
	pin_set_A(3);
	static order_t *order = NULL;
	order_t *current = queue_get_current_order();
	pin_clear_A(3);
	pin_set_A(4);
	if(order != current) {
		lcd_update_info(current);
		order = current;
	}
	pin_clear_A(4);
	pin_set_A(5);
	if(lcd_update_underway) {
		if(lcd_read(0) & (1 << LCD_BUSY)) {
			pin_clear_A(5);
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
				lcd_gotoxy(0, info_row);
			} else
				lcd_update_underway = 0;
		}
	}
	pin_clear_A(5);
}

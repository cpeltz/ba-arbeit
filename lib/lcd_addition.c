#include "lcd_addition.h"
#include "lcd.h"
#include "order.h"
#include "queue.h"
#include <inttypes.h>
#include <stdlib.h>
#include "pin.h"
#include "definitions.h"
#include "debug.h"

/**
 * @defgroup LCD_ADDITION_MODULE LCD Addition Module
 * Provides routines to update the LCD with vital runtime information.
 * @{
 */
char info[4][21]={VERSION,"","",""};
int8_t info_col = 0, info_row = 0;
uint8_t lcd_update_underway = 0;

/**
 * Update the in-memory information thats displayed on the LCD.
 *
 * @param[in] order The order which should be printed. NULL will print nothing.
 */
void lcd_update_info(const order_t * const order) {
	extern uint8_t ACTIVE_BRAKE_ENABLE;
	extern uint8_t ACTIVE_BRAKE_WHEN_IDLE;
	extern uint8_t ACTIVE_BRAKE_WHEN_TRIGGER_REACHED;
	extern uint8_t INTERFACE_TWI;
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
		// Make sure we never write more then we have space for (14 2 digit hex numbers)
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
	info_col = 0;
	info_row = -1;
	lcd_update_underway = 1;
}

/**
 * Main-Loop function to handle the correct updateing of the LCD.
 */
void lcd_update_screen(void) {
	static order_t *order = NULL;
	order_t *current = queue_get_current_order();
	// If order changed then update the lcd information
	if(order != current) {
		lcd_update_info(current);
		order = current;
	}
	// We are in the process of putting the updated information
	// on the LC-Display
	if(lcd_update_underway) {
		// Check if Busy-Flag is still set
		if(lcd_read(0) & (1 << LCD_BUSY)) {
			return;
		} else {
			// If we are at row -1 we have to clear the LCD first
			if(info_row < 0) {
				lcd_write(1 << LCD_CLR, 0);
				info_row = 0;
				return;
			}
			// Put the current character on the LCD if the char is not '\0'
			if(info[info_row][info_col])
				lcd_write(info[info_row][info_col], 1);
			else
				info_col = 20;
			// If we are not at the end of the line we just move the column forward
			if(info_col < 20)
				info_col++;
			else if (info_row < 3) {
				// We are at the end of the line and have to move to the next one
				info_row++;
				info_col = 0;
				lcd_gotoxy(0, info_row);
			} else {
				// And now we are done wiht updateing the LCD (Pheew)
				lcd_update_underway = 0;
			}
		}
	}
}
/*@}*/

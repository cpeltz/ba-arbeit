#ifndef LCD_ADDITION_H
#define LCD_ADDITION_H

#include "types.h"

const static char *version = "Ver. 2.9.20090728";

void lcd_update_screen(void);
void lcd_print_status(const order_t * const order); 

#endif

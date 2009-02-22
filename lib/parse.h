#ifndef PARSE_H
#define PARSE_H

#include <inttypes.h>
#include "types.h"

void parser_init(void);
void parser_add_byte(uint8_t byte); // TODO FIXME Check with twi.h whether this ist the right type
uint8_t parser_has_new_order();
void parser_get_new_order(order_t* order);

#endif

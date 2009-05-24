#ifndef PARSE_H
#define PARSE_H

#include <inttypes.h>
#include "types.h"

void parser_init(void);
void parser_update(void);
uint8_t parser_has_new_order();
void parser_get_new_order(order_t* order);

#endif

#ifndef PARSE_H
#define PARSE_H

#include <inttypes.h>
#include "types.h"

void parser_init(void);
void parser_update(void);
uint8_t parser_has_new_order(void);
void parser_get_new_order(order_t* order);
uint8_t bytes_needed(uint8_t);

#endif

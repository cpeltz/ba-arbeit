#ifndef IO_H
#define IO_H

#include <inttypes.h>
#include "types.h"

void io_init(void);
uint8_t io_get_available(void);
uint8_t io_get(uint8_t* value);
uint8_t io_put(uint8_t value);
void io_flush(void);

void _io_push(uint8_t value);
#endif

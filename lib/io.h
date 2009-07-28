#ifndef IO_H
#define IO_H

#include <inttypes.h>

void io_init(void);
uint8_t io_get_available(void);
uint8_t io_get(uint8_t* value);
uint8_t io_put(uint8_t value);
void io_flush(void);

void _io_push(uint8_t value);
uint8_t io_obj_get_remaining_size(void);
void io_obj_remove_current(void);
uint8_t io_get_next_transmission_byte(void);
uint8_t io_obj_get_current_size(void);
void io_obj_start(void);
void io_obj_end(void);
void io_reset_transmission_status(void);
uint8_t io_obj_remaining(void);
#endif

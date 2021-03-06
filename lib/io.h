#ifndef IO_H
#define IO_H

#include <inttypes.h>

/**
 * Do _NOT_ change this define, it _will_ break the IO Framework.
 */
#define IO_BUFFER_SIZE 256

void io_init(void);
uint8_t io_get_free_buffer_size(void);
uint8_t io_get(uint8_t* value);
void io_pop(void);
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

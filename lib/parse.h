#ifndef PARSE_H
#define PARSE_H

#include <inttypes.h>
#include <avr/pgmspace.h>

typedef struct DRIVE_ORDER
{
  uint8_t command[3];
  int16_t parameter[4];
  uint8_t command_length;
  uint8_t parameter_count;
} drive_order_t;

typedef struct PARSE_SET
{
  uint8_t status;
  uint8_t number_length;
  char number[6];
  uint8_t flags;
} parse_set_t;

int8_t parse_character(const uint8_t character, drive_order_t * parser, parse_set_t * pset);
int8_t parse_string(const char *progmem_input, drive_order_t * output);
int8_t parse_string_twi(const char *input, drive_order_t * output);

#endif

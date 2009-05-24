#ifndef FLAGS_H
#define FLAGS_H

#include <inttypes.h>

// Funktionen für globale Flags
void flag_set(uint8_t flag);
void flag_clear(uint8_t flag);
uint8_t flag_read(uint8_t flag);
uint8_t flag_read_and_clear(uint8_t flag);

// und für lokale Flags
void flag_local_set(uint8_t * storage, const uint8_t flag);
void flag_local_clear(uint8_t * storage, const uint8_t flag);
uint8_t flag_local_read(const uint8_t * storage, const uint8_t flag);
uint8_t flag_local_read_and_clear(uint8_t * storage, const uint8_t flag);

#endif

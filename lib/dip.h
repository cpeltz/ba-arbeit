#ifndef DIP_H
#define DIP_H

#include <inttypes.h>

// Used to initialize the DIP Switches
void dip_init(void);
// Used to get a boolean value (can only be 0 or 1)
uint8_t dip_read(const uint8_t bit);

#endif

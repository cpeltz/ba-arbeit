#ifndef TWI_H
#define TWI_H

#include <inttypes.h>

void twi_init(void);
int16_t twi_fiforead(void);
void twi_fifoinit(void);
int16_t twi_statefifoRead(void);

#endif

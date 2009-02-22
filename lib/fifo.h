#ifndef FIFO_H
#define FIFO_H

#include "types.h"

void fifo_Init(fifo_t * fifo, uint8_t * fifo_Buffer, const uint8_t fifo_Size);
uint8_t fifo_Write(fifo_t * fifo, const uint8_t fifo_Data);
uint8_t fifo_Read(fifo_t * fifo);

#endif

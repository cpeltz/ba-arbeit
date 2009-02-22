#ifndef FIFO_H
#define FIFO_H

typedef struct FIFO_BUFFER
{
  uint8_t volatile count;
  uint8_t size;
  uint8_t *p_read;
  uint8_t *p_write;
  uint8_t read2end, write2end;
} fifo_t;

void fifo_Init(fifo_t * fifo, uint8_t * fifo_Buffer, const uint8_t fifo_Size);
uint8_t fifo_Write(fifo_t * fifo, const uint8_t fifo_Data);
int16_t fifo_Read(fifo_t * fifo);

#endif

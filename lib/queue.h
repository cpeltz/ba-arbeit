#ifndef QUEUE_H
#define QUEUE_H

#include <inttypes.h>
#include "parse.h"

void queue_Init(void);
void queue_Flush(void);
int8_t queue_Write(const drive_order_t * order);
int8_t queue_Read(drive_order_t * output);
int8_t queue_Delete(void);
uint8_t queue_Entries(void);
uint8_t queue_ReadPosition(void);
uint8_t queue_WritePosition(void);
void queue_ReadEntry(uint8_t entry, drive_order_t * output);
int8_t queue_Do(const char *progmem_input);
int8_t queue_Do_twi(const char *input);

#endif

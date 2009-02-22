#ifndef STATUS_H
#define STATUS_H

#include "parse.h"

#define STAT_HALT   0
#define STAT_RUN    1

typedef struct GLOBAL_STATE
{
  uint8_t state_left;
  uint8_t state_right;
  int8_t speed_left;
  int8_t speed_right;
  int16_t position_left;
  int16_t position_right;
  int16_t difference;
  drive_order_t drive_order;
  uint8_t flags;

} global_state_t;

void status_update(global_state_t * status);
void status_init(void);
void status_read(global_state_t * status);

#endif

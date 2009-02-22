#include <stdint.h>
#include "status.h"

global_state_t global_state;

void
status_update(global_state_t * status)
{
  global_state = *status;
}

void
status_init(void)
{
  global_state.state_left = 0;
  global_state.state_right = 0;
  global_state.speed_left = 0;
  global_state.speed_right = 0;
  global_state.position_left = 0;
  global_state.position_right = 0;
  global_state.difference = 0;
  global_state.flags = 0;
}

void
status_read(global_state_t * status)
{
  *status = global_state;
}

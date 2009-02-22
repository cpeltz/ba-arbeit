#ifndef STATUS_H
#define STATUS_H

#include "types.h"

#define STAT_HALT   0
#define STAT_RUN    1

void status_update();
void status_init(void);
void status_read(global_state_t * status);

#endif

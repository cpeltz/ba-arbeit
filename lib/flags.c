#include <inttypes.h>
#include "flags.h"

static uint32_t flags_Storage = 0;

void
flag_Set(uint8_t flag)
// globale Flagge setzen
{
  flags_Storage |= ((uint32_t) 1 << flag);
}

void
flag_Clear(uint8_t flag)
// globale Flagge löschen
{
  flags_Storage &= ~((uint32_t) 1 << flag);
}

uint8_t
flag_Read(uint8_t flag)
// globale Flagge lesen
{
  return ((flags_Storage & ((uint32_t) 1 << flag)) >> flag);
}

uint8_t
flag_ReadAndClear(uint8_t flag)
// globale Flagge lesen und löschen
{
  if ((flags_Storage & ((uint32_t) 1 << flag)))
    {
      flag_Clear(flag);
      return 1;
    }
  else
    {
      return 0;
    }
}

void
flagLocal_Set(uint8_t * storage, const uint8_t flag)
// lokale Flagge setzen
{
  *storage |= (1 << flag);
}

void
flagLocal_Clear(uint8_t * storage, const uint8_t flag)
// lokale Flagge löschen
{
  *storage &= ~(1 << flag);
}

uint8_t
flagLocal_Read(const uint8_t * storage, const uint8_t flag)
// lokale Flagge lesen
{
  return ((*storage & (1 << flag)) >> flag);
}

uint8_t
flagLocal_ReadAndClear(uint8_t * storage, const uint8_t flag)
// lokale Flagge lesen und löschen
{
  if ((*storage & (1 << flag)))
    {
      flagLocal_Clear(storage, flag);
      return 1;
    }
  else
    {
      return 0;
    }
}

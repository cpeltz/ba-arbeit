#include <inttypes.h>
#include "queue.h"
#include "parse.h"

// Größe der Queue
#define QUEUE_SIZE  100

// Befehlsarray [x]{y], wobei [x] = QueuePosition und [y] = Seite
static drive_order_t command_queue[QUEUE_SIZE];

// Dummy Befehl
const drive_order_t dummy_order = { {0, 0, 0}, {0, 0, 0, 0}, 0, 0 };

// Queue-Positionen
static uint8_t queue_readposition = 0;
static uint8_t queue_writeposition = 0;
static uint8_t queue_entries = 0;

// Löscht das gesamte Array und initialisert die Zeiger
void
queue_Init(void)
{
  for (uint8_t counter = 0; counter < (QUEUE_SIZE - 1); counter++)
    {
      command_queue[counter] = dummy_order;
    }
  queue_readposition = 0;
  queue_writeposition = 0;
  queue_entries = 0;
}

void
queue_Flush(void)
{
  queue_readposition = 0;
  queue_writeposition = 0;
  queue_entries = 0;
}

// Schreibt einen gesamten Queueeintrag
int8_t
queue_Write(const drive_order_t * order)
{
  if (queue_entries == QUEUE_SIZE)
    {
      return -1;
    }
  else
    {
      command_queue[queue_writeposition] = *order;

      queue_entries++;

      queue_writeposition++;
      queue_writeposition %= QUEUE_SIZE;
    }
  return 1;
}

// Liest den aktuellen Queueeintrag
int8_t
queue_Read(drive_order_t * output)
{
  if (queue_entries == 0)
    {
      return -1;
    }
  else
    {
      *output = command_queue[queue_readposition];
    }
  return 1;
}

// Löscht den aktuellen Queueeintrag und springt zum nächsten
int8_t
queue_Delete(void)
{
  if (queue_entries == 0)
    {
      return -1;
    }
  else
    {
      command_queue[queue_readposition] = dummy_order;

      queue_entries--;

      queue_readposition++;
      queue_readposition %= QUEUE_SIZE;
    }
  return 1;
}

// Gibt die Anzahl der Einträge zurück
uint8_t
queue_Entries(void)
{
  return queue_entries;
}

// Gibt die Position des Lesezeigers zurück
uint8_t
queue_ReadPosition(void)
{
  return queue_readposition;
}

// Gibt die Position des Schreibzeigers zurück
uint8_t
queue_WritePosition(void)
{
  return queue_writeposition;
}

// Liest einen bestimmten Eintrag aus der Queue
void
queue_ReadEntry(uint8_t entry, drive_order_t * output)
{
  *output = command_queue[entry % QUEUE_SIZE];
}

// Direct-Drive Befehl
int8_t
queue_Do(const char *progmem_input)
{
  drive_order_t parse;
  int8_t exitcode = parse_string(progmem_input, &parse);
  if (exitcode == 1)
    {
      queue_Write(&parse);
    }
  return exitcode;
}

// Befehlszeichenkette von TWI in Queue eintragen
int8_t
queue_Do_twi(const char *input)
{
  drive_order_t parse;
  int8_t exitcode = parse_string_twi(input, &parse);
  if (exitcode == 1)
    {
      queue_Write(&parse);
    }
  return exitcode;
}

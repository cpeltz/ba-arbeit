#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <inttypes.h>
#include <ctype.h>
#include <stdlib.h>
#include "twi.h"
#include "debug.h"
#include "queue.h"
#include "flags.h"
#include "irq.h"
#include "parse.h"
#include "fifo.h"
#include "definitions.h"
#include "status.h"
#include "motor.h"
#include "led.h"

uint8_t twi_rx_state = 0;
uint8_t twi_tx_state = 0;
char twi_receive[32];
uint8_t twi_receive_length = 0;
drive_order_t twi_transfer;

uint8_t output = 0;
uint8_t output_offset = 0;
static uint8_t twi_comfifoBuffer[10];
static uint8_t twi_stateBuffer[10];
static fifo_t twi_comfifo;
static fifo_t twi_state;
static global_state_t temp_state;

static uint8_t twi_parameter_load(const uint8_t value);

static uint8_t twi_drive_order_output(const drive_order_t * order, const uint8_t state);

static uint8_t twi_queue_output(uint8_t reset);

ISR(TWI_vect)
{
  led_switch(LED_BLUE, SINGLE);
  
  uint8_t twi_status = TWSR & 0xf8;
  fifo_Write(&twi_state, twi_status);
  char twi_data = 0;

  switch (twi_status)
    {
    case 0x60:
      // Own SLA+W has been received;
      // ACK has been returned

      twi_rx_state = 0;

      TWCR &= ~(1 << TWSTO);
      TWCR |= (1 << TWEA);
      break;

    case 0x80:
      // Previously addressed with own
      // SLA+W; data has been received;
      // ACK has been returned

      twi_data = TWDR;
      //debug_WriteInteger(PSTR("D:"), twi_data);

      TWCR &= ~(1 << TWSTO);
      TWCR |= (1 << TWEA);

      switch (twi_rx_state)
        {
        case 0:
          // Erstes Zeichen der Übertragung empfangen
          switch (twi_data)
            {
            case 0xf0:
              // Registerabfrage
              twi_rx_state = 1;
              break;

            case 0xf1:
              // Ausgabe des aktuellen Kommandos
              twi_tx_state = 2;
              break;

            case 0xf2:
              // Ausgabe der Queue
              twi_tx_state = 1;
              break;

            case 0xf3:
              // Übertragung eines Kommandos
              twi_rx_state = 2;
              twi_receive[0] = '>';
              twi_receive[1] = '\0';
              twi_receive_length = 1;
              break;

            case 0xfe:
              // TWI Reset
              twi_tx_state = 0;
              twi_rx_state = 0;
              flag_Set(RESET_TWI);
              break;

            case 0xff:
              // Controller Reset
              cli();
              motor_Break(2);
              wdt_reset();
              wdt_enable(4);
              while (1);
              break;

            default:
              // Ein-Byte Kommando direkt ausführen
              if (isprint(twi_data))
                {
                  if (!(fifo_Write(&twi_comfifo, twi_data)))
                    {
                      TWCR &= ~(1 << TWEA);
                    }
                }
              break;
            }
          break;

        case 1:
          // Registerabfrage
          output_offset = twi_data;
          twi_rx_state = 0;
          twi_tx_state = 0;
          break;

        case 2:
          // Übertragung eines Kommandos
          if (twi_data == '\0')
            {
              // Kommando abgeschlossen
              //debug_PutString(twi_receive);
              //debug_NewLine();
              twi_receive[twi_receive_length] = twi_data;
              //debug_WriteInteger(PSTR("Q:"), queue_Do_twi(twi_receive));
              queue_Do_twi(twi_receive);
              twi_rx_state = 0;
            }
          else
            {
              // Zeichen anhängen
              if (isprint(twi_data))
                {
                  // Nur 'druckbare' Zeichen
                  twi_receive[twi_receive_length++] = twi_data;
                }
            }
          break;
        }
      break;

    case 0xa0:
      // A STOP condition or repeated
      // START condition has been
      // received while still addressed as
      // Slave

      TWCR &= ~((1 << TWSTA) | (1 << TWSTO));
      TWCR |= (1 << TWEA);
      break;

    case 0xa8:
      // Own SLA+R has been received;
      // ACK has been returned

      switch (twi_tx_state)
        {
        case 0:
          // Registerabfrage
          output = twi_parameter_load(output_offset++);
          TWDR = output;
          TWCR |= (1 << TWEA);
          break;

        case 1:
          // Ausgabe der Queue
          twi_queue_output(1);
          output = twi_queue_output(0);
          TWDR = output;
          if (output == 0x00)
            {
              // Keine Einträge in der Queue
              output = 0xee;
              TWDR = output;
              twi_tx_state = 0;
              TWCR &= ~(1 << TWEA);
            }
          else
            {
              TWCR |= (1 << TWEA);
            }
          break;

        case 2:
          // Ausgabe des aktuellen Kommandos
          status_read(&temp_state);
          twi_drive_order_output(&temp_state.drive_order, 0);
          output = twi_drive_order_output(&temp_state.drive_order, 1);
          TWDR = output;
          if (output == 0x00)
            {
              // Kein Eintrag in der Queue
              output = 0xee;
              TWDR = output;
              twi_tx_state = 0;
              TWCR &= ~(1 << TWEA);
            }
          else
            {
              TWCR |= (1 << TWEA);
            }
          break;
        }

      TWCR &= ~((1 << TWSTA) | (1 << TWSTO));
      break;

    case 0xb8:
      // Data byte in TWDR has been
      // transmitted; ACK has been
      // received

      switch (twi_tx_state)
        {
        case 0:
          // Registerabfrage
          output = twi_parameter_load(output_offset++);
          TWDR = output;
          TWCR |= (1 << TWEA);
          break;

        case 1:
          // Ausgabe der Queue
          output = twi_queue_output(0);
          TWDR = output;
          if (output == 0x00)
            {
              // Alle Einträge gesendet
              twi_tx_state = 0;
              TWCR &= ~(1 << TWEA);
            }
          break;

        case 2:
          // Ausgabe des aktuellen Kommandos
          output = twi_drive_order_output(&temp_state.drive_order, 1);
          TWDR = output;
          if (output == 0x00)
            {
              // Eintrag komplett gesendet
              twi_tx_state = 0;
              TWCR &= ~(1 << TWEA);
            }
          else
            {
              TWCR |= (1 << TWEA);
            }
          break;
        }

      TWCR &= ~((1 << TWSTA) | (1 << TWSTO));
      break;

    case 0xc0:
      // Data byte in TWDR has been
      // transmitted; NOT ACK has been
      // received

    case 0xc8:
      // Last data byte in TWDR has been
      // transmitted (TWEA = "0"); ACK
      // has been received
      twi_tx_state = 0;

      TWCR &= ~((1 << TWSTA) | (1 << TWSTO));
      TWCR |= (1 << TWEA);
      break;

    case 0x00:
      // Bus error due to an illegal
      // START or STOP condition

      twi_rx_state = 0;
      twi_tx_state = 0;
      flag_Set(RESET_TWI);

      break;
    }
  TWCR |= (1 << TWINT);
}

void
twi_init(void)
{
  if (flag_Read(INTERFACE_TWI))
    {
      // Slave Address = 84
      TWAR = 84;
      // TWI aktivieren
      TWCR = (1 << TWEA) | (1 << TWEN) | (1 << TWIE);
      fifo_Init(&twi_comfifo, twi_comfifoBuffer, 10);
      fifo_Init(&twi_state, twi_stateBuffer, 10); 
    }
}

static uint8_t
twi_parameter_load(const uint8_t value)
{
  uint8_t ret = 0;
  global_state_t status;
  tacho_t tacho;

  switch (value)
    {
    case 0:
      ret = 0;
    case 1:
      // High-Byte, Radposition, links
      // ret = (wheel_ReadPosition(0) >> 8);
      status_read(&status);
      ret = (status.position_left >> 8);
      break;
    case 2:
      // Low-Byte, Radposition, links
      // ret = (wheel_ReadPosition(0) & 0xff);
      status_read(&status);
      ret = (status.position_left & 0xff);
      break;
    case 3:
      // High-Byte, Radposition, rechts
      status_read(&status);
      ret = (status.position_right >> 8);
      break;
    case 4:
      // Low-Byte, Radposition, rechts
      status_read(&status);
      ret = (status.position_right & 0xff);
      break;
    case 5:
      // Tempo, links
      status_read(&status);
      ret = status.speed_left;
      break;
    case 6:
      // Tempo, rechts
      status_read(&status);
      ret = status.speed_right;
      break;
    case 7:
      // High-Byte, Differenz
      status_read(&status);
      ret = (status.difference >> 8);
      break;
    case 8:
      // Low-Byte, Differenz
      status_read(&status);
      ret = (status.difference & 0xff);
      break;
    case 9:
      // main_state[0]
      status_read(&status);
      ret = status.state_left;
      break;
    case 10:
      // main_state[1]
      status_read(&status);
      ret = status.state_right;
      break;
    case 11:
      // Einträge in der Queue
      ret = queue_Entries();
      break;
    case 12:
      // High-Byte, Tacho links
      wheel_GetTacho(&tacho);
      ret = (tacho.left >> 8);
      break;
    case 13:
      // Low_Byte, Tacho links
      wheel_GetTacho(&tacho);
      ret = (tacho.left & 0xf0);
      break;
    case 14:
      // High-Byte, Tacho rechts
      wheel_GetTacho(&tacho);
      ret = (tacho.right >> 8);
      break;
    case 15:
      // Low_Byte, Tacho rechts
      wheel_GetTacho(&tacho);
      ret = (tacho.right & 0xf0);
      break;
    case 16:
      // High-Byte, Tacho Differenz
      wheel_GetTacho(&tacho);
      ret = (tacho.difference >> 8);
      break;
    case 17:
      // Low_Byte, Tacho Difference
      wheel_GetTacho(&tacho);
      ret = (tacho.difference & 0xf0);
      break;

    default:
      ret = 0xff;
      break;
    }

  return ret;
}

static uint8_t
twi_drive_order_output(const drive_order_t * order, const uint8_t state)
{
  static uint8_t doo_state;
  static char ptemp[10];
  static uint8_t pcount = 0;
  static uint8_t poff = 0;
  char ret = 0;

  switch (state)
    {
    case 0:
      // Reset
      doo_state = 0;
      ptemp[0] = '\0';
      pcount = 0;
      poff = 0;
      break;
    case 1:
      // Continue
      switch (doo_state)
        {
        case 0:
          // Ausgabe beginnen
          if (order->command_length == 0)
            {
              // Kommando ungültig
              ret = 0x00;
            }
          else
            {
              do
                {
                  ptemp[pcount] = order->command[pcount];
                  pcount++;
                }
              while (pcount < order->command_length);
              pcount = 0;
              doo_state = 1;
              ret = '>';
            }
          break;

        case 1:
          // Kommando ausgeben
          // if (ptemp[pcount + 1] == '\0')
          if ((pcount + 1) == order->command_length)
            {
              ret = ptemp[pcount];
              if (order->parameter_count == 0)
                {
                  doo_state = 255;
                }
              else
                {
                  doo_state = 2;
                  pcount = 0;
                }
            }
          else
            {
              ret = ptemp[pcount++];
            }
          break;

        case 2:
          // Parameter vorbereiten
          ret = ',';
          itoa(order->parameter[poff++], ptemp, 10);
          doo_state = 3;
          break;

        case 3:
          // Parameter ausgeben
          if (ptemp[pcount + 1] == '\0')
            {
              ret = ptemp[pcount];
              if (poff >= order->parameter_count)
                {
                  doo_state = 255;
                }
              else
                {
                  doo_state = 2;
                  pcount = 0;
                }
            }
          else
            {
              ret = ptemp[pcount++];
            }
          break;

        case 255:
          // Kein Zeichen mehr da
          ret = 0x00;
          break;
        }
    }
  return ret;
}

static uint8_t
twi_queue_output(uint8_t reset)
{
  uint8_t ret = 0;

  static uint8_t output_state = 0;
  static uint8_t output_readposition = 0;
  static uint8_t output_entries = 0;
  static drive_order_t output_order;

  switch (reset)
    {
    case 1:
      // Queue-Ausgabe zurücksetzen
      output_state = 0;
      output_readposition = queue_ReadPosition();
      output_entries = queue_Entries();
      break;

    case 0:
      // Queue ab aktueller Leseposition ausgeben
      switch (output_state)
        {
        case 0:
          // Ausgabe initialisieren
          if (output_entries > 0)
            {
              output_entries--;
              twi_drive_order_output(&output_order, 0);
              queue_ReadEntry(output_readposition++, &output_order);
              ret = twi_drive_order_output(&output_order, 1);
              output_state = 1;
            }
          else
            {
              ret = 0x00;
            }
          break;

        case 1:
          // Ausgabe fortführen
          ret = twi_drive_order_output(&output_order, 1);
          if (ret == 0)
            {
              // Eintrag zu Ende
              if (output_entries)
                ret = '\r';
              output_state = 0;
            }
          break;
        }
      break;
    }
  return ret;
}

int16_t
twi_fiforead(void)
{
  return fifo_Read(&twi_comfifo);
}

void
twi_fifoinit(void)
{
  fifo_Init(&twi_comfifo, twi_comfifoBuffer, 10);
}

int16_t
twi_statefifoRead(void)
{
  return fifo_Read(&twi_state);
}

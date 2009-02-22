/*******************************************
* Studienarbeit:                          *
* Motorsteuerung                          *
*******************************************/

#include <inttypes.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include "./lib/definitions.h"
#include "./lib/motor.h"
#include "./lib/flags.h"
#include "./lib/uart.h"
#include "./lib/led.h"
#include "./lib/timer.h"
#include "./lib/irq.h"
#include "./lib/lcd.h"
#include "./lib/fifo.h"
#include "./lib/pid.h"
#include "./lib/drive.h"
#include "./lib/debug.h"
#include "./lib/parse.h"
#include "./lib/queue.h"
#include "./lib/twi.h"
#include "./lib/dip.h"
#include "./lib/status.h"

// Variable für Main-States (links und rechts)
uint8_t main_state[2] = { 1, 1 };

// V. für Sicherungskopien der Zeitflaggen
uint8_t main_time_flags = 0;
// V. für Tachowerte
global_state_t status;

#define MT1MS   0
#define MT10MS  1
#define MT100MS 2
#define MT262MS 3

// Status 3, aktiv Bremsen
int16_t s3_position[2];
uint8_t s3_state[2];
uint8_t s3_timeout[2];

// Status 4, PID-Fahrt
int8_t s4_speed[2];

// Status 5, PID-Fahrt mit Differenzausgleich
int8_t s5_speed;

// Backups im Halt-Modus
uint16_t trigger_time[2];

// Parser Variable
drive_order_t parser;
parse_set_t pset;

// Temp Variable für QueueDrive
// drive_order_t queue_drive[4];
int8_t exitcode = 0;
tacho_t tacho_temp;
char buffer[10];

// Queue->Job Bearbeitung
drive_order_t instruction;
uint8_t job_flags = 3;

// JOB_DONE_L/R werden nach ausgelöstem Trigger gesetzt
#define JOB_DONE_L      0
#define JOB_DONE_R      1
// Jobausführung läuft/ist angehalten
#define JOB_RUN         2

// Drive-Order Interpreter
drive_order_t drive_order;

// Triggervariablen
uint8_t trigger_flags = 0;

// Kennzeichnung des aktiven Triggers
#define T_TRIGGER_ACTIVE_L    0
#define T_TRIGGER_ACTIVE_R    1
#define P_TRIGGER_ACTIVE_L    2
#define P_TRIGGER_ACTIVE_R    3

int
main(void)
{
  // Reset Quelle speichern
  exitcode = MCUSR;
  MCUSR = 0;
  // Watchdog ausschalten
  wdt_reset();
  //MCUSR &= ~(1<<WDRF);
  wdt_disable();

  // Eingang von DIP initialisieren
  dip_init();

  if (dip_read(0))
    {
      // DIP1 = ON: Steuerung über TWI
      flag_Set(INTERFACE_TWI);
    }
  else
    {
      // DIP1 = OFF: Steuerung über UART
      flag_Set(INTERFACE_UART);
    }

  if (dip_read(2))
    {
      // DIP3 = ON: LCD vorhanden
      flag_Set(LCD_PRESENT);
    }
  else
    {
      // DIP3 = OFF: LCD nicht vorhanden
    }

  // Alles mögliche initialisieren
  motor_Init();
  uart_Init();
  led_init();
  timer_Init();
  irq_Init();
  queue_Init();
  twi_init();
  status_init();

  // Main-States zurücksetzen
  main_state[0] = 0;
  main_state[1] = 0;

  // Interrupts aktivieren
  sei();

  flag_Set(DEBUG_ENABLE);
  debug_ResetTerminal();
  debug_WriteString_P(PSTR("Motorsteuerung V2.008.02.18\r\n"));
  debug_WriteString_P(PSTR("---------------------------\r\n\n"));
  debug_WriteString_P(PSTR("DIP-Schalter Einstellungen:\r\n"));
  if (dip_read(0))
    debug_WriteString_P(PSTR("DIP1: ON   Interface = TWI\r\n"));
  else
    debug_WriteString_P(PSTR("DIP1: OFF  Interface = UART\r\n"));
  if (dip_read(1))
    debug_WriteString_P(PSTR("DIP2: ON   Debug-Ausgaben aktiv\r\n"));
  else
    debug_WriteString_P(PSTR("DIP2: OFF  Debug-Ausgaben inaktiv\r\n"));
  if (dip_read(2))
    debug_WriteString_P(PSTR("DIP3: ON   LCD aktiv\r\n"));
  else
    debug_WriteString_P(PSTR("DIP3: OFF  LCD inaktiv\r\n"));
  if (dip_read(3))
    debug_WriteString_P(PSTR("DIP4: ON   nicht verwendet\r\n"));
  else
    debug_WriteString_P(PSTR("DIP4: OFF  nicht verwendet\r\n"));

  debug_WriteString_P(PSTR("\nReset Quelle:\r\n"));
  if (exitcode & (1<<PORF))
    {
      debug_WriteString_P(PSTR("Power-On "));
      exitcode |= 0x80; // Starte LED Test
    }
  if (exitcode & (1<<EXTRF))
    debug_WriteString_P(PSTR("External "));
  if (exitcode & (1<<BORF))
    debug_WriteString_P(PSTR("Brown-Out "));
  if (exitcode & (1<<WDRF))
    debug_WriteString_P(PSTR("Watchdog "));
  if (exitcode & (1<<JTRF))
    debug_WriteString_P(PSTR("JTAG"));
    debug_NewLine();

    led_test();
    led_switchoff();

    if (flag_Read(LCD_PRESENT))
      {
        led_switch(LED_RED1, FLASHFAST);
        debug_WriteString_P(PSTR("\r\nInitialisiere LCD..."));
        lcd_init(LCD_DISP_ON);
        lcd_clrscr();
        lcd_puts_p(PSTR("Initialize..."));
        debug_WriteString_P(PSTR(" Okay.\r\n"));
        led_switch(LED_RED1, OFF);
      }

  led_switch(LED_GREEN, ON);

  if (dip_read(1))
    {
      // DIP2 = ON: Debug Ausgaben
      debug_WriteString_P(PSTR("\r\nBeginne mit Debug-Ausgaben...\r\n-----------------------------\r\n"));
      flag_Set(DEBUG_ENABLE);
      led_switch(LED_ORANGE, ON);
    }
  else
    {
      // DIP2 = OFF: keine Debug Ausgaben
      debug_WriteString_P(PSTR("\r\nBeende Debug-Ausgaben.\r\n"));
      flag_Clear(DEBUG_ENABLE);
    }

  // Standard PID-Parameter setzen
  drive_SetPIDParameter(2, 80, 20, 10, 500);
  
  while (1)
    {
      // Erstelle lokale Kopie der ZeitFlaggen

      if (flag_ReadAndClear(TIMER_1MS))
        flagLocal_Set(&main_time_flags, MT1MS);
      if (flag_ReadAndClear(TIMER_10MS))
        flagLocal_Set(&main_time_flags, MT10MS);
      if (flag_ReadAndClear(TIMER_100MS))
        flagLocal_Set(&main_time_flags, MT100MS);
      if (flag_ReadAndClear(TIMER_262MS))
        flagLocal_Set(&main_time_flags, MT262MS);

/*-----------------------------------------------------------------*/
      uint8_t side = 0;

      do
        {
          if (flag_Read(HALT_DRIVE))
            {
              motor_Break(2);
            }
          else
            {
              switch (main_state[side])
                {
                case 0:
                  // Macht überhaupt nichts
                  break;
                case 1:
                  // "Free Running" - Modus
                  // - nur einmal ausführen
                  motor_SetSpeed(side, 0);
                  main_state[side] = 0;
                  break;
                case 2:
                  // Bremsen
                  // - nur einmal ausführen
                  motor_Break(side);
                  main_state[side] = 0;
                  break;
                case 3:
                  // Aktiv Bremsen
                  // - jede Millisekunde ausführen
                  if (flagLocal_Read(&main_time_flags, MT1MS))
                    {
                      int16_t position;

                      switch (s3_state[side])
                        {
                        case 0:
                          // Motor abschalten und Rad ausrollen lassen
                          motor_SetSpeed(side, 0);
                          s3_timeout[side] = 2;
                          s3_state[side] = 1;
                          break;
                        case 1:
                          // Warte auf Timeout oder Stillstand
                          if (wheel_ReadSpeed(side) == 0)
                            {
                              s3_state[side] = 3;
                              s3_position[side] = wheel_ReadPosition(side);
                            }
                          else if (s3_timeout[side] == 0)
                            {
                              s3_timeout[side] = 2;
                              s3_state[side] = 2;
                              motor_Break(side);
                            }
                          break;
                        case 2:
                          // nochmal
                          if ((wheel_ReadSpeed(side) == 0) || (s3_timeout[side] == 0))
                            {
                              s3_state[side] = 3;
                              s3_timeout[side] = 5;
                              s3_position[side] = wheel_ReadPosition(side);
                            }
                          break;
                        case 3:
                          // Position halten
                          position = wheel_ReadPosition(side);

                          if (s3_position[side] == position)
                            motor_Break(side);
                          else
                            {
                              if (s3_position[side] < position)
                                motor_SetSpeed(side, -60);
                              else
                                motor_SetSpeed(side, 60);

                              if (s3_timeout[side] == 0)
                                {
                                  s3_timeout[side] = 5;
                                  s3_position[side] = position;
                                }
                            }
                          break;
                        }
                    }
                  if (flagLocal_Read(&main_time_flags, MT100MS))
                    {
                      if (s3_timeout[side])
                        s3_timeout[side]--;
                    }
                  break;
                case 4:
                  // PID-Fahrt
                  // alle 100ms ausführen
                  if (s4_speed[side] == 0)
                    {
                      main_state[side] = 3;
                      s3_state[side] = 0;
                    }
                  else if (flagLocal_Read(&main_time_flags, MT100MS))
                    {
                      drive_UsePID(side, s4_speed[side]);
                    }
                  break;
                case 5:
                  // PID-Fahrt mit Differenzausgleich
                  // alle 100ms
                  if (s5_speed == 0)
                    {
                      main_state[0] = 3;
                      main_state[1] = 3;
                      s3_state[0] = 0;
                      s3_state[1] = 0;
                    }
                  else if (flagLocal_Read(&main_time_flags, MT100MS))
                    {
                      drive_UsePID(2, s5_speed);
                      main_state[1] = 0;
                    }
                  break;
                }
            }

/*-----------------------------------------------------------------*/
          // In diesem Teil werden aktive Trigger überprüft
/*-----------------------------------------------------------------*/
          if (flagLocal_Read(&trigger_flags, (T_TRIGGER_ACTIVE_L + side)))
            {
              // Zeittrigger aktiv
              if (flag_ReadAndClear(T_TRIGGER_L + side))
                {
                  // aktiv und ausgelöst
                  debug_WriteInteger(PSTR("Zeittrigger ausgelöst: "), side);
                  flagLocal_Set(&job_flags, (JOB_DONE_L + side));
                  flagLocal_Clear(&trigger_flags, (T_TRIGGER_ACTIVE_L + side));
                  main_state[side] = 3;
                  s3_state[side] = 0;
                }
            }
          else if (flagLocal_Read(&trigger_flags, (P_TRIGGER_ACTIVE_L + side)))
            {
              // Positionstrigger aktiv
              if (flag_ReadAndClear(P_TRIGGER_L + side))
                {
                  // aktiv und ausgelöst
                  debug_WriteInteger(PSTR("Positionstrigger ausgelöst: "), side);
                  flagLocal_Set(&job_flags, (JOB_DONE_L + side));
                  flagLocal_Clear(&trigger_flags, (P_TRIGGER_ACTIVE_L + side));
                  main_state[side] = 3;
                  s3_state[side] = 0;
                }
            }
/*-----------------------------------------------------------------*/
        }
      while (++side < 2);

      if (flagLocal_ReadAndClear(&main_time_flags, MT1MS))
        {
          if (flag_ReadAndClear(RESET_TWI))
            {
              twi_init();
            }
        }
      if (flagLocal_ReadAndClear(&main_time_flags, MT10MS))
        {
          int16_t twistatetemp = twi_statefifoRead();
          if ((twistatetemp != -1) && (flag_Read(DEBUG_ENABLE)))
            {
              debug_WriteString_P(PSTR("TWI Statusbyte: 0x"));
              itoa(twistatetemp, buffer, 16);
              debug_PutString(buffer);
              debug_NewLine();
            }
        }
      if (flagLocal_ReadAndClear(&main_time_flags, MT100MS))
        {
        }
      if (flagLocal_ReadAndClear(&main_time_flags, MT262MS))
        {
          if (flag_Read(LCD_PRESENT))
            {
              lcd_clrscr();
              lcd_gotoxy(0,0);
              itoa(status.speed_left, buffer, 10);
              lcd_puts(buffer);
              lcd_gotoxy(5,0);
              itoa(status.speed_left, buffer, 10);
              lcd_puts(buffer);
              lcd_gotoxy(10,0);
              itoa(status.state_left, buffer, 10);
              lcd_puts(buffer);
              lcd_gotoxy(12,0);
              itoa(status.state_right, buffer, 10);
              lcd_puts(buffer);
              lcd_gotoxy(14,0);
              itoa(queue_Entries(), buffer, 10);
              lcd_puts(buffer);

              lcd_gotoxy(0,1);
              itoa(status.position_left, buffer, 10);
              lcd_puts(buffer);
              lcd_gotoxy(7,1);
              itoa(status.position_right, buffer, 10);
              lcd_puts(buffer);
              lcd_gotoxy(14,1);
              itoa(status.difference, buffer, 10);
              lcd_puts(buffer);

              wheel_GetTacho(&tacho_temp);
              lcd_gotoxy(0,2);
              itoa(tacho_temp.left, buffer, 10);
              lcd_puts(buffer);
              lcd_gotoxy(7,2);
              itoa(tacho_temp.right, buffer, 10);
              lcd_puts(buffer);
              lcd_gotoxy(14,2);
              itoa(tacho_temp.difference, buffer, 10);
              lcd_puts(buffer);
              

              lcd_gotoxy(0,3);
              if (flagLocal_Read(&job_flags, JOB_RUN))
                lcd_puts_p(PSTR("RUN "));
              else
                lcd_puts_p(PSTR("STOP"));
              if (flagLocal_Read(&job_flags, JOB_DONE_L))
                lcd_puts_p(PSTR(" LD"));
              else
                lcd_puts_p(PSTR(" LA"));
              if (flagLocal_Read(&job_flags, JOB_DONE_R))
                lcd_puts_p(PSTR(" RD"));
              else
                lcd_puts_p(PSTR(" RA"));
              if (flag_Read(HALT_DRIVE))
                lcd_puts_p(PSTR(" PAUSE"));
               
              //status.speed_left
            }
        }
/*-----------------------------------------------------------------*/

      int16_t character = 0;
      int8_t parse_exit_code = 0;

      if (flag_Read(INTERFACE_UART))
        {
          // Lese Zeichen von UART Schnittstelle
          character = uart_GetChar();
          if (!(character == -1))
            {
              parse_exit_code = parse_character(character, &parser, &pset);
            }
        }
      else if (flag_Read(INTERFACE_TWI))
        {
          // Lese Zeichen von TWI
          character = twi_fiforead();
          parse_exit_code = -1;
        }

      if (!(character == -1))
        {
          if (parse_exit_code == -1)
            // kein verwertbares Zeichen für den Parser
            {
              // Direktes Kommando auswerten
              if (flag_Read(DEBUG_ENABLE))
                {
                  //debug_WriteInteger(PSTR("Ein-Byte Zeichen: "), character);
                  if (flag_Read(HALT_DRIVE))
                    debug_WriteString_P(PSTR("Pause aktiviert!\r\n"));
                }

              switch (character)
                {
                case ' ':
                  // anhalten
                  main_state[0] = 5;
                  s5_speed = 0;
                  flagLocal_Clear(&job_flags, JOB_RUN);
                  job_flags |= (1 << JOB_DONE_L) | (1 << JOB_DONE_R);
                  queue_Flush();
                  trigger_flags = 0;
                  if (flag_Read(DEBUG_ENABLE))
                    debug_WriteString_P(PSTR("Fahrzeug anhalten.\r\n"));
                  break;
                case 'w':
                  // vorwärts
                  queue_Flush();
                  queue_Do(PSTR("P,80,20,10,500"));
                  queue_Do(PSTR("D,40"));
                  flagLocal_Set(&job_flags, JOB_RUN);
                  job_flags |= (1 << JOB_DONE_L) | (1 << JOB_DONE_R);
                  trigger_flags = 0;
                  if (flag_Read(DEBUG_ENABLE))
                    debug_WriteString_P(PSTR("Fahre vorwärts.\r\n"));
                  break;
                case 's':
                  // rückwärts
                  queue_Flush();
                  queue_Do(PSTR("P,80,20,10,500"));
                  queue_Do(PSTR("D,-40"));
                  flagLocal_Set(&job_flags, JOB_RUN);
                  job_flags |= (1 << JOB_DONE_L) | (1 << JOB_DONE_R);
                  trigger_flags = 0;
                  if (flag_Read(DEBUG_ENABLE))
                    debug_WriteString_P(PSTR("Fahre rückwärts.\r\n"));
                  break;
                case 'a':
                  // links
                  queue_Flush();
                  queue_Do(PSTR("P,80,20,10,500"));
                  queue_Do(PSTR("D,-40,40"));
                  flagLocal_Set(&job_flags, JOB_RUN);
                  job_flags |= (1 << JOB_DONE_L) | (1 << JOB_DONE_R);
                  trigger_flags = 0;
                  if (flag_Read(DEBUG_ENABLE))
                    debug_WriteString_P(PSTR("Linksdrehung.\r\n"));
                  break;
                case 'd':
                  // rechts
                  queue_Flush();
                  queue_Do(PSTR("P,80,20,10,500"));
                  queue_Do(PSTR("D,40,-40"));
                  flagLocal_Set(&job_flags, JOB_RUN);
                  job_flags |= (1 << JOB_DONE_L) | (1 << JOB_DONE_R);
                  trigger_flags = 0;
                  if (flag_Read(DEBUG_ENABLE))
                    debug_WriteString_P(PSTR("Rechtsdrehung.\r\n"));
                  break;

                case 'p':
                  // Fahrzeug bis zur Freigabe anhalten
                  if (flag_Read(DEBUG_ENABLE))
                    debug_WriteString_P(PSTR("Pause bis zur Freigabe.\r\n"));
                  if (!(flag_Read(HALT_DRIVE)))
                    {
                      flag_Set(HALT_DRIVE);
                      if (flagLocal_Read(&trigger_flags, T_TRIGGER_ACTIVE_L))
                        {
                          // linker Zeittrigger aktiv
                          trigger_time[0] = trigger_Get_T(0);
                          trigger_Clear_T(0);
                        }
                      if (flagLocal_Read(&trigger_flags, T_TRIGGER_ACTIVE_L))
                        {
                          // rechter Zeittrigger aktiv
                          trigger_time[1] = trigger_Get_T(1);
                          trigger_Clear_T(1);
                        }
                    }
                  break;
                case 'P':
                  // Freigabe
                  if (flag_Read(DEBUG_ENABLE))
                    debug_WriteString_P(PSTR("Pause freigegeben.\r\n"));
                  if (flag_Read(HALT_DRIVE))
                    {
                      flag_Clear(HALT_DRIVE);
                      if (flagLocal_Read(&trigger_flags, T_TRIGGER_ACTIVE_L))
                        {
                          // linken Zeittrigger wiederherstellen
                          trigger_Set_T(0, trigger_time[0]);
                        }
                      if (flagLocal_Read(&trigger_flags, T_TRIGGER_ACTIVE_L))
                        {
                          // rechten Zeittrigger wiederherstellen
                          trigger_Set_T(1, trigger_time[1]);
                        }
                    }
                  break;

                case '\r':
                  // aktuellen Befehl als erledigt markieren
                  if (flag_Read(DEBUG_ENABLE))
                    debug_WriteString_P(PSTR("Aktuellen Job beenden.\r\n"));
                  job_flags |= (1 << JOB_DONE_L) | (1 << JOB_DONE_R);
                  trigger_flags = 0;
                  break;

                case 'j':
                  // Job Ausführung anhalten (keine neuen Jobs beginnen)
                  if (flag_Read(DEBUG_ENABLE))
                    debug_WriteString_P(PSTR("Job-Ausführung anhalten.\r\n"));
                  flagLocal_Clear(&job_flags, JOB_RUN);
                  break;
                case 'J':
                  // Job Ausführung fortsetzen
                  if (flag_Read(DEBUG_ENABLE))
                    debug_WriteString_P(PSTR("Job-Ausführung fortsetzen.\r\n"));
                  flagLocal_Set(&job_flags, JOB_RUN);
                  break;

                case 'k':
                  // Queue löschen
                  if (flag_Read(DEBUG_ENABLE))
                    debug_WriteString_P(PSTR("Queue löschen.\r\n"));
                  queue_Flush();
                  trigger_flags = 0;
                  break;

                case 't':
                  // Tacho zurücksetzen
                  if (flag_Read(DEBUG_ENABLE))
                    debug_WriteString_P(PSTR("Tacho zurücksetzen.\r\n"));
                  wheel_ClearTacho();
                  break;

                  // // QueueControl
                  // case 'y':
                  // if (queue_Read(&parser) == 1)
                  // debug_WriteDriveOrder(&parser);
                  // else
                  // debug_WriteString_P(PSTR("Keine Einträge.\r\n"));
                  // break;
                  // case 'c':
                  // debug_WriteInteger(PSTR("Einträge: "), queue_Entries());
                  // break;
                  // case 'x':
                  // exitcode = 0;
                  // if (queue_Entries() > 0)
                  // {
                  // do
                  // {
                  // queue_ReadEntry(queue_ReadPosition() + exitcode++, &parser);
                  // debug_WriteDriveOrder(&parser);
                  // }
                  // while (exitcode < queue_Entries());
                  // }
                  // else
                  // debug_WriteString_P(PSTR("Keine Einträge.\r\n"));
                  // break;
                }
            }
          else if (parse_exit_code == 1)
            // Fertig ausgewertete Zeichenkette in Queue eintragen
            {
              debug_WriteDriveOrder(&parser);
              queue_Write(&parser);
              // switch (parser.command[0])
              // {
              // case 'D':
              // // Drive Befehl
              // exitcode = queue_Write(&parser);
              // break;
              // case 'S':
              // // Straight Drive Befehl
              // exitcode = queue_Write(&parser);
              // break;
              // case 'P':
              // // PID Befehl
              // exitcode = queue_Write(&parser);
              // break;
              // }
            }
        }

      if ((flagLocal_Read(&job_flags, JOB_DONE_L)) && (flagLocal_Read(&job_flags, JOB_DONE_R)))
        {
          status.drive_order.command[0] = '\0';
          status.drive_order.command_length = 0;
          status.drive_order.parameter_count = 0;
        }

      // Job Verarbeitung
      if (flagLocal_Read(&job_flags, JOB_RUN) && (!(flag_Read(HALT_DRIVE))))
        {
          // nur wenn Jobausführung aktiv
          if ((flagLocal_Read(&job_flags, JOB_DONE_L)) && (flagLocal_Read(&job_flags, JOB_DONE_R)))
            {
              // nur wenn keine Jobs laufen
              // können neue bearbeitet werden
              if (queue_Read(&instruction) == 1)
                {
                  status.drive_order = instruction;
                  if (flag_Read(DEBUG_ENABLE))
                    {
                      debug_WriteString_P(PSTR("Beginne Ausführung von "));
                      debug_WriteDriveOrder(&instruction);
                    }
                  // Queueeintrag löschen und Befehl ausführen
                  queue_Delete();

                  job_flags &= ~((1 << JOB_DONE_L) | (1 << JOB_DONE_R));
                  trigger_flags = 0;

                  switch (instruction.command[0])
                    {
                    case 'D':
                      // DAS Fahrkommando überhaupt

                      switch (instruction.command_length)
                        {
                        case 1:
                          // D ohne Zusatz
                          // entspricht Fahren ohne Trigger
                          switch (instruction.parameter_count)
                            {
                            case 1:
                              // Normale Fahrt, Tempo in parameter[0]
                              job_flags |= (1 << JOB_DONE_L) | (1 << JOB_DONE_R);
                              main_state[0] = 4;
                              main_state[1] = 4;
                              s4_speed[0] = instruction.parameter[0];
                              s4_speed[1] = instruction.parameter[0];
                              break;
                            case 2:
                              // Normale Fahrt
                              // Tempo links in parameter[0]
                              // Tempo rechts in parameter[1]
                              job_flags |= (1 << JOB_DONE_L) | (1 << JOB_DONE_R);
                              main_state[0] = 4;
                              main_state[1] = 4;
                              s4_speed[0] = instruction.parameter[0];
                              s4_speed[1] = instruction.parameter[1];
                              break;
                            }
                          break;
                        case 2:
                          // Gibt kein D?-Kommando
                          break;
                        case 3:
                          // Fahrt mit Trigger
                          main_state[0] = 4;
                          main_state[1] = 4;
                          s4_speed[0] = instruction.parameter[0];
                          s4_speed[1] = instruction.parameter[1];

                          switch (instruction.command[1])
                            {
                            case 'N':
                              // Kein Trigger für linkes Rad
                              flagLocal_Set(&job_flags, JOB_DONE_L);
                              break;
                            case 'T':
                              // Zeittrigger für linkes Rad
                              flagLocal_Set(&trigger_flags, T_TRIGGER_ACTIVE_L);
                              trigger_Set_T(0, instruction.parameter[2]);
                              break;
                            case 'P':
                              // Positionstrigger für linkes Rad
                              wheel_ClearPosition(0);
                              flagLocal_Set(&trigger_flags, P_TRIGGER_ACTIVE_L);
                              trigger_Set_P(0, instruction.parameter[2]);

                              break;
                            }
                          switch (instruction.command[2])
                            {
                            case 'N':
                              // Kein Trigger für rechtes Rad
                              flagLocal_Set(&job_flags, JOB_DONE_R);
                              break;
                            case 'T':
                              // Zeittrigger für rechtes Rad
                              flagLocal_Set(&trigger_flags, T_TRIGGER_ACTIVE_R);

                              if (instruction.parameter_count == 3)
                                trigger_Set_T(1, instruction.parameter[2]);
                              else
                                trigger_Set_T(1, instruction.parameter[3]);
                              break;
                            case 'P':
                              // Positionstrigger für rechtes Rad
                              wheel_ClearPosition(1);
                              flagLocal_Set(&trigger_flags, P_TRIGGER_ACTIVE_R);

                              if (instruction.parameter_count == 3)
                                trigger_Set_P(1, instruction.parameter[2]);
                              else
                                trigger_Set_P(1, instruction.parameter[3]);
                              break;
                            }
                          break;
                        }
                      break;
                    case 'S':
                      // Kommando für Geradeausfahrt
                      if (instruction.command_length == 1)
                        {
                          // Geradeausfahrt ohne Trigger
                          job_flags |= (1 << JOB_DONE_L) | (1 << JOB_DONE_R);

                          main_state[0] = 5;
                          s5_speed = instruction.parameter[0];
                        }
                      else
                        {
                          flagLocal_Set(&job_flags, JOB_DONE_R);
                          switch (instruction.command[1])
                            {
                            case 'N':
                              // auch kein Trigger
                              flagLocal_Set(&job_flags, JOB_DONE_L);
                              main_state[0] = 5;
                              s5_speed = instruction.parameter[0];
                              break;
                            case 'T':
                              // Zeittrigger
                              flagLocal_Set(&trigger_flags, T_TRIGGER_ACTIVE_L);
                              trigger_Set_T(0, instruction.parameter[1]);
                              main_state[0] = 5;
                              s5_speed = instruction.parameter[0];
                              break;
                            case 'P':
                              // Positionstrigger
                              wheel_ClearPosition(0);
                              flagLocal_Set(&trigger_flags, P_TRIGGER_ACTIVE_L);
                              trigger_Set_P(0, instruction.parameter[1]);
                              main_state[0] = 5;
                              s5_speed = instruction.parameter[0];
                              break;
                            case 'D':
                              // Einstellen der Tick-Differenz
                              flagLocal_Set(&job_flags, JOB_DONE_L);

                              if ((instruction.parameter_count == 0)
                                  || (instruction.parameter[0] == 0))
                                {
                                  // Keine Differenzangabe dient zum
                                  // Rücksetzen
                                  wheel_ClearDifference();
                                }
                              else
                                {
                                  // sonst parameter[0] zur derzeitigen
                                  // Differenz addieren
                                  wheel_WriteDifference(wheel_ReadDifference() +
                                                        instruction.parameter[0]);
                                }
                              break;
                            }
                        }
                      break;
                    case 'P':
                      // PID-Parameter Änderungen
                      job_flags |= (1 << JOB_DONE_L) | (1 << JOB_DONE_R);

                      if (instruction.command_length == 1)
                        {
                          // Kommando P stellt Parameter beider Räder ein
                          if (instruction.parameter_count == 4)
                            {
                              drive_SetPIDParameter(2, instruction.parameter[0],
                                                    instruction.parameter[1],
                                                    instruction.parameter[2],
                                                    instruction.parameter[3]);
                            }
                        }
                      else
                        {
                          switch (instruction.command[1])
                            {
                            case 'L':
                              // Neue Parameter für linkes Rad
                              if (instruction.command[2] == 'S')
                                {
                                  // Einstellen der Fehlersumme
                                  drive_SetPIDSumError(0, instruction.parameter[0]);
                                }
                              else
                                {
                                  // PID Parameter, links
                                  drive_SetPIDParameter(0, instruction.parameter[0],
                                                        instruction.parameter[1],
                                                        instruction.parameter[2],
                                                        instruction.parameter[3]);
                                }
                              break;
                            case 'R':
                              // Neue Parameter für rechtes Rad
                              if (instruction.command[2] == 'S')
                                {
                                  // Einstellen der Fehlersumme
                                  drive_SetPIDSumError(1, instruction.parameter[0]);
                                }
                              else
                                {
                                  // PID Parameter, rechts
                                  drive_SetPIDParameter(1, instruction.parameter[0],
                                                        instruction.parameter[1],
                                                        instruction.parameter[2],
                                                        instruction.parameter[3]);
                                }
                              break;
                            case 'S':
                              // Neue Fehlersumme für beide Räder
                              drive_SetPIDSumError(2, instruction.parameter[0]);
                              break;
                            }
                        }
                      break;
                    }
                }
              else
                {
                }
            }
        }
      // Globale Status Variable aktualisieren
      status.state_left = main_state[0];
      status.state_right = main_state[1];
      status.speed_left = wheel_ReadSpeed(0);
      status.speed_right = wheel_ReadSpeed(1);
      status.position_left = wheel_ReadPosition(0);
      status.position_right = wheel_ReadPosition(1);
      status.difference = wheel_ReadDifference();

      status_update(&status);
    }
}

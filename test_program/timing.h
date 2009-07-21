
/*******************************************************
 **  Mikroprozessorlabor                              **
 **                                                   **
 **  Institut f�r Betriebssysteme und Rechnerverbund  **
 **  Technische Universit�t Braunschweig              **
 *******************************************************/

// File :  timing.h 
// Modul:  N�here Bezeichnung des Projektes oder Teilprojektes
// Vers.:  x.yy
// Autor:  Name, Vorname 

// Letzte �nderung: 2004.11.05.
//*****************************


#ifndef __timing_h__
#define __timing_h__

#include "byte.h"

// ----- Definitionen -----
// ========================


// ----- Deklarationen -----
// =========================

typedef enum
	{ TIMER_NOP = 0,
	, TIMER_STARTED
	, TIMER_REACHED
	} TimerStatus;

/** Wartet eine angebene Zeit an Millisekunden.
  *
  * Das Warten ist ein Busy-Wait.
  * 
  * @param ms Die zu wartende Zeit
  */
void wait_ms(unsigned int ms);

/** Wartet us * 100 Mikrosekunden.
  *
  * @param us Die anzahl an zu wartenden 100 Mikrosekunden.
  */
void wait_100us(byte us);

void init_timer0();

/** Startet den Timer.
  *
  * Der Timer flankt den Port 4.
  *
  * @param ms Die Periodendauer in Millisekunden.
  */
void wait_with_timer(unsigned int ms);


#endif // __timing_h__

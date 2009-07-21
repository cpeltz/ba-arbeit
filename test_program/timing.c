
/*******************************************************
 **  Mikroprozessorlabor                              **
 **                                                   **
 **  Institut f�r Betriebssysteme und Rechnerverbund  **
 **  Technische Universit�t Braunschweig              **
 *******************************************************/

// File :  timing.c 
// Modul:  N�here Bezeichnung des Projektes oder Teilprojektes
// Vers.:  x.yy
// Autor:  Name, Vorname 

// Letzte �nderung: 2004.11.05.
//*****************************



// ----- Includes -----
//=====================

#include "timing.h"
#include "lcd.h"
#include <REG552.H>    

// ----- Definitionen -----
// ========================

extern TimerStatus xdata condTimerStatus;

// ----- Deklarationen, die nur innerhalb dieser Datei gebraucht werden -----
// ==========================================================================

unsigned int wait_ms_max = 1;
unsigned int wait_ms_current = 0;

/** Wartet 100 Mikrosekunden. */
void usleep100();

// ----- Funktionsdefinitionen -----
// =================================

void init_timer0() {
	TR0 = 0;       // switch off timer
	EA = 1;        // enable interrupts
	ET0 = 1;       // enable timer interrupt
	TMOD = (TMOD & 0xf0) | 0x01; // set timer mode
	TH0 = 0xfc;    // let timer count 922 clicks
	TL0 = 0x65;    // ^^
	TR0 = 1;       // switch on timer
}

void int_timer0() interrupt 1 {
	if ((++wait_ms_current) >= wait_ms_max) {
		TR0 = 0;
		ET0 = 0;
		wait_ms_current = 0;
		condTimerStatus = TIMER_REACHED;
		//lcd_print_string("TIMER FINITO");
	}
	else {
		TR0 = 0;       // switch off timer
		TH0 = 0xfc;    // let timer count 922 clicks
		TL0 = 0x65;    // ^^
		TR0 = 1;       // switch on timer
	}
}

void wait_with_timer(unsigned int ms) {
	init_timer0();
	condTimerStatus = TIMER_STARTED;
	wait_ms_max = ms;
	wait_ms_current = 0;
}

void usleep100() {
	// Verbr�t in etwa 100 Mikrosekunden

	int i = 0;
	while(i < 5)
		i++;
	i++;
	i++;
	i++;
	i++;
}

void wait_100us(byte us) {
	// Verbr�t in etwa us * 100 Mikrosekunden
	
	while (us--) {
		int i = 0;
		while(i < 4)
			i++;
		i++;
		i++;
		i++;
		i++;
	}
}

void wait_ms(unsigned int ms) {
	// Verbr�t in etwa ms Millisekunden

	for(; ms > 0; ms--) {
		int i;
		for (i = 0; i < 7; i++)
			usleep100();
		for (i = 0; i < 4; i++);
		i++;
	}
}
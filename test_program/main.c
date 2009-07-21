
/*******************************************************
 **  Mikroprozessorlabor                              **
 **                                                   **
 **  Institut für Betriebssysteme und Rechnerverbund  **
 **  Technische Universität Braunschweig              **
 *******************************************************/

// File :  main.c
// Modul:  Nähere Bezeichnung des Projektes oder Teilprojektes
// Vers.:  x.yy
// Autor:  Name, Vorname 

// Letzte Änderung: 2004.11.05.
//*****************************



// ----- Includes -----
// ====================

#include <REG552.H>               // special function register declarations
#include <stdio.h>                // prototype declarations for I/O functions
#include <string.h>
#include "byte.h"
#include "timing.h"
#include "helper.h"
#include "lcd.h"
#include "i2c_master.h"
#include "motor_platine/order.h"



// ----- Definitionen -----
// ========================

#ifdef MONITOR51                         // Debugging with Monitor-51 needs space   
char code reserve [3] _at_ 0x23;         // for serial interrupt if Stop Exection    
#endif                                   // with Serial Intr. is enabled                        


// ----- Deklarationen, die nur innerhalb dieser Datei gebraucht werden -----
// ==========================================================================



// ----- Funktionsdefinitionen -----
// =================================

#ifndef MONITOR51
void initSerial()
{
   S0CON = 0x50;		// S0CON: mode 1, 8bit Uart 
   TMOD  = 0x20;		// TMOD : timer 1, mode 2 => 8bit Auto-Reload 
   TH1 	 = 0xfd;		// TH1  : reload value for 9600baud @ 11,0592 MHz 
   TR1 	 = 1;			// TR1  : timer 1 run (TR1 = TCON.6) 
   TI 	 = 1;			// TI   : set TI to send first char of UART (TI = SCON.1) 
}
#endif

byte xdata buf[5];

void main() {
	// Main-Funktion
	byte send = 0;
	order_t order;
	#ifndef MONITOR51		// Falls nicht für den Monitor übersetzt wird und die
		initSerial();		// serielle Schnittstelle dort schon initialisiert
	#endif					// wurde, initialisiere sie jetzt.

	i2c_init();
	lcd_init();
	lcd_print_string("Waiting ... (10s)");
	wait_ms(10000);
	lcd_print_string("Ready");

	while(1) {
		//order_init(&order);
		/*lcd_print_string("Test 1: 0x03 0x40 0x40");
		order_set_type(&order, ORDER_DRIVE_N_N);
		order_add_params(&order, "11", 0x40, 0x40);*/
		//order->dat[0] = 0x11;
		//send = order_send(&order);
		buf[0]=42;
		buf[1]=0x11;
		send = i2c_send(buf,2);
		int2str(buf, send);
		lcd_print_string(buf);
		//wait_ms(20000);
		//lcd_print_string("STOP");
		//order_set_type(&order, ORDER_CONTROL_STOP_DRIVE);
		//order_send(&order);
		wait_ms(20000);
	}
}
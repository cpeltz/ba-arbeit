#include "lib/definitions.h"
#include "lib/debug.h"
#include "lib/dip.h"
#include "lib/drive.h"
#include "lib/flags.h"
#include "lib/io.h"
#include "lib/irq.h"
#include "lib/lcd.h"
#include "lib/motor.h"
#include "lib/order.h"
#include "lib/parse.h"
#include "lib/queue.h"
#include "lib/status.h"
#include "lib/timer.h"
#include <inttypes.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>

/** 
 * @mainpage Drive Software Documenation
 * @section intro_sec Introduction
 * The aim of the Project is to provide a modern, well written,
 * good documented and extensible control software for a drive hardware
 * module, which was created in an earlier work by Timo ***. The challenges
 * lie in the constrained resources of the hardware, a high tolerance
 * against errors and throughput.
 *
 * @section design_sec Design
 * The design of the functions, modules and structures is geared towards
 * an object-oriented approch, where applicable and where it doesn't impedes
 * speed. The time spent in interrupt service routines (ISR) has been
 * minimized to the point, that they are still readable but only do the
 * needed steps (which can be seen in the USART ISR's). There are many
 * options which influence the whole software system and the user may
 * adjust these (located in options.h) to fit the software more to his/her
 * needs or just decrease memory footprint. Extensibility is mostly
 * implemented in the protocol part of the system, but most functions are
 * as general as they can be and as much side-effect free as possible.
 * With placeholder functions to support more then 15 different normal
 * orders and simple steps to add a normal function the protocol is as
 * extensible as it can get.
 *
 * @section howto_sec Howtos
 * Collection of different "HowTos".
 * @subsection howto_add_norder_sec How to add a normal order to the system.
 * Description will come soon ... it's to hot at the moment.
 *
 * @section adddoc_sec Additional Documentation
 * To be able to use the software documented here, one should read the
 * protocol reference document provided with the software in the doc/
 * directory. To completely understand what the software does on a low-level
 * one has to read the chip documentation from atmel. Also provided in the
 * doc/ directory.
 */

/**
 * @defgroup MAIN_Module System
 * Holds the main function and the logical program system.
 * @{
 */

/**
 *	Show from which source the system got resettet.
 */
int8_t reset_source = 0;
/**
 *	Stores the local time flags.
 *
 *	On the timer Interrupts which occure every 1ms, 10ms, 100ms and 262ms, the respective flag
 *	in this bitfield will be set.
 */
uint8_t local_time_flags = 0;

/**
 *	The Main setup function; calls every needed *_init() function
 */
void initialize(void) {
	// activate interrupts
	sei();

	// init all subsystems
	dip_init();
	io_init();
	motor_init();
	timer_init();
	order_array_init();
	irq_init();
	queue_init();
	status_init();
	lcd_init(LCD_DISP_ON);

	// set standard PID parameter
	drive_SetPIDParameter(2, 80, 20, 10, 500);
}

/**
 *	Used to read the settings given by the position of the dip switches.
 *
 *	With the dip switches, the user chooses which Interface he wants to use, if a LCD is plugged in or
 *	to enable the debugging output.
 */
void update_dip_flags(void) {
	// clear all dip flags
	flag_clear(DEBUG_ENABLE);
	flag_clear(INTERFACE_TWI);
	flag_clear(LCD_PRESENT);
	if (dip_read(0))
		// DIP1 = ON: Use TWI as communication methode
		flag_set(INTERFACE_TWI);
	if (dip_read(1))
		// DIP2 = ON: Activate Debug Output
		flag_set(DEBUG_ENABLE);
	if (dip_read(2))
		// DIP3 = ON: LCD is pluged in
		flag_set(LCD_PRESENT);
}

/**
 * This function prints, on every system start, information about the System.
 * @todo Disable dbug at the end
 */
void print_startup(void) {
	// Startup Debug Infos
	if (flag_read(LCD_PRESENT)) {
		lcd_clrscr();
		lcd_puts("Ver. 2.9.20090722");
		lcd_gotoxy(0,1);
		if (dip_read(0)) {
			lcd_puts("TWI ");
		} else {
			lcd_puts("twi ");
		}
		if (dip_read(1)) {
			lcd_puts("DEBUG");
		} else {
			lcd_puts("debug");
		}
	}
	flag_set(DEBUG_ENABLE);
	debug_ResetTerminal();
	debug_WriteString_P(PSTR("Motorsteuerung V2.9.20090722\n"));
	debug_WriteString_P(PSTR("----------------------------\n\n"));
	debug_WriteString_P(PSTR("DIP-Schalter Einstellungen:\n"));
	if (dip_read(0))
		debug_WriteString_P(PSTR("DIP1: ON   Interface = TWI\n"));
	else
		debug_WriteString_P(PSTR("DIP1: OFF  Interface = UART\n"));
	if (dip_read(1))
		debug_WriteString_P(PSTR("DIP2: ON   Debug-Ausgaben aktiv\n"));
	else
		debug_WriteString_P(PSTR("DIP2: OFF  Debug-Ausgaben inaktiv\n"));
	if (dip_read(2))
		debug_WriteString_P(PSTR("DIP3: ON   LCD aktiv\n"));
	else
		debug_WriteString_P(PSTR("DIP3: OFF  LCD inaktiv\n"));
	if (dip_read(3))
		debug_WriteString_P(PSTR("DIP4: ON   nicht verwendet\n"));
	else
		debug_WriteString_P(PSTR("DIP4: OFF  nicht verwendet\n"));

	debug_WriteString_P(PSTR("\nReset Quelle:\n"));
	if (reset_source & (1<<PORF)) 
		debug_WriteString_P(PSTR("Power-On "));
	if (reset_source & (1<<EXTRF))
		debug_WriteString_P(PSTR("External "));
	if (reset_source & (1<<BORF))
		debug_WriteString_P(PSTR("Brown-Out "));
	if (reset_source & (1<<WDRF))
		debug_WriteString_P(PSTR("Watchdog "));
	if (reset_source & (1<<JTRF))
		debug_WriteString_P(PSTR("JTAG"));
	debug_NewLine();
	update_dip_flags();
}

/**
 *	Fills the local variable with time tick flags.
 *
 *	To show how much time has passed since last execution and to
 *	be able to do order maintainace time based, this is used.
 */
void copy_timer_flags(void) {
	// Copy global timer flags to a local copy, which will be used throughout the program.
	// This is done to not miss a timer tick.
	local_time_flags = 0;
//	if (flag_read_and_clear(TIMER_1MS))
//		flag_local_set( &local_time_flags, TIMER_1MS );
//	if (flag_read_and_clear(TIMER_10MS))
//		flag_local_set( &local_time_flags, TIMER_10MS );
	if (flag_read_and_clear(TIMER_100MS))
		flag_local_set( &local_time_flags, TIMER_100MS );
	if (flag_read_and_clear(TIMER_262MS))
		flag_local_set( &local_time_flags, TIMER_262MS );
}

/**
 *	Main process for orders.
 *
 *	This function simply maintains an order, gets a new one from the queue
 *	or just brake active.
 */
void process_orders(void) {
	// This function gets an order and lets it execute
	order_t *current_order = NULL;

//	debug_WriteInteger(PSTR("main.c : process_orders() :  Avaialable Orders = "), queue_order_available());
//	debug_NewLine();
	current_order = queue_get_current_order();
	if (current_order != NULL) {
//		debug_WriteString_P(PSTR("main.c : process_orders() :  Ack new Order, starting processing\n"));
		order_process(current_order);
//		debug_WriteString_P(PSTR("main.c : process_orders() :  Processing done\n"));
		if (current_order->status & ORDER_STATUS_DONE) {
//			debug_WriteString_P(PSTR("main.c : process_orders() :  Order has status = DONE\n"));
			drive_brake_active_set();
			queue_pop();
			current_order = NULL;
		}
	} else {
		drive_brake_active();
	}
}

void itoa_hex(uint8_t value, char *buffer, uint8_t size) {
	uint8_t lower = value & 0x0f;
	uint8_t upper = (value & 0xf0) >> 4;
	if( size <= 2)
		return;
	buffer[0] = (upper < 10) ? ('0' + upper) : ('a' + (upper-10));
	buffer[1] = (lower < 10) ? ('0' + lower) : ('a' + (lower-10));
	buffer[2] = '\0';
}

void lcd_print_status(void) {
	static order_t *order = NULL;
	order_t *current = queue_get_current_order();
	char buffer[3];
	if(order != current) {
		order = current;
		lcd_clrscr();
		lcd_puts("Ver. 2.9.20090722");
		lcd_gotoxy(0,1);
		if (dip_read(0)) {
			lcd_puts("TWI ");
		} else {
			lcd_puts("twi ");
		}
		if (dip_read(1)) {
			lcd_puts("DEBUG");
		} else {
			lcd_puts("debug");
		}
		if(order != NULL) {
			lcd_gotoxy(0,2);
			int length = bytes_needed(order->data[0]);
			for(int i=0; i < length; i++) {
				itoa_hex(order->data[i], buffer, 3);
				lcd_puts(buffer);
				if(i == 6)
					lcd_gotoxy(0,3);
				else
					lcd_puts(" ");
			}
		}
	}
}

int main(void) {
	// Save the reset source of the last reset
	reset_source = MCUSR;
	MCUSR = 0;
	
	// Disable the watchdog timer
	wdt_reset();
	wdt_disable();

	// Initialize all subsystems (DIP, Drive, etc pp)
	initialize();
	
	// Update Status for the first time
	status_update();

	// Set Global flags which are depending on the dip switches
	update_dip_flags();

	// Print driver version etc pp to debug
	print_startup();


	while(1) {
//		debug_WriteString_P(PSTR("main.c : main() :  copy_timer_flags()\r\n"));
		// Copy the global timer flags to a local variable (but global for this file)
		copy_timer_flags();

//		debug_WriteString_P(PSTR("main.c : main() :  process_orders()\r\n"));
		// Processes the next or current order
		process_orders();		

		// If a LCD is pluged in we get nice status messages on it
		if (flag_read(LCD_PRESENT))
			lcd_print_status();

//		debug_WriteString_P(PSTR("main.c : main() :  parser_update()\n"));
		// Update the order parser
		parser_update();

//		debug_WriteString_P(PSTR("main.c : main() :  queue_update()\n"));
		// Housekeeping for the order queue
		queue_update();
		
//		debug_WriteString_P(PSTR("main.c : main() :  status_update()\n"));
		// Update the global status of the program
		status_update();
	}

	// Should be never reached
	return 0;
}
/*@}*/

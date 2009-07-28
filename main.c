#include "lib/definitions.h"
#include "lib/debug.h"
#include "lib/dip.h"
#include "lib/drive.h"
#include "lib/io.h"
#include "lib/irq.h"
#include "lib/lcd.h"
#include "lib/lcd_addition.h"
#include "lib/motor.h"
#include "lib/order.h"
#include "lib/parse.h"
#include "lib/queue.h"
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
 * To add a new order, following things have to be done:
 * In the File order_functions.[ch] the new handler function has to be inserted.
 * Please use void $OrderName_instruction(order_t *order) as Prototype (with the
 * $OrderName replaced by the real name for the order). When the Order is done
 * you have to set order->status |= ORDER_STATUS_DONE so the order will be
 * discarded. You can use ORDER_STATUS_STARTED to use the same handler function
 * for setting up things and for maintaining an order (look for an example of
 * this at the drive_instruction handler function.
 * After this, you have to register your handler function, you do that in the
 * file order.c in order_init(). There is an array, that gets initialized with
 * the functions, put you function at the correct array position as the position
 * corresponds with the command code (thats the lower 4 bit of the first byte of
 * the order.
 * Then you need to visit the parser.c file and edit the bytes_needed(uint8_t)
 * function. That function gives back the number of bytes a order will be long
 * debending on it's command byte (the first byte of the order). In the big
 * switch statement add a case for your new order with the case beeing 0x0y with
 * y beeing the hexadecimal value of the lower 4 bits of the command byte.
 * Please make sure, that every possibility of the order lengths is smaller then
 * the ORDER_TYPE_MAX_LENGTH variable (in options.h) and at least 1, as the command
 * byte will be counted too.
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
extern uint8_t LCD_PRESENT;
extern uint8_t DEBUG_ENABLE;
extern uint8_t INTERFACE_TWI;
extern uint8_t timer_global_flags;

const char *version = "Ver. 2.9.20090727";

/**
 *	Used to read the settings given by the position of the dip switches.
 *
 *	With the dip switches, the user chooses which Interface he wants to use, if a LCD is plugged in or
 *	to enable the debugging output.
 */
void update_dip_flags(void) 
	// DIP1 = ON: Use TWI as communication methode
	INTERFACE_TWI = dip_read(0);
	// DIP2 = ON: Activate Debug Output
	DEBUG_ENABLE = dip_read(1);
	// DIP3 = ON: LCD is pluged in
	LCD_PRESENT = dip_read(2);
}

/**
 *	The Main setup function; calls every needed *_init() function
 */
void initialize(void) {
	// activate interrupts
	sei();

	// init all subsystems
	dip_init();
	update_dip_flags();
	lcd_init(LCD_DISP_ON);
	io_init();
	motor_init();
	timer_init();
	order_array_init();
	irq_init();
	queue_init();

	// set standard PID parameter
	drive_SetPIDParameter(2, 80, 20, 10, 500);
}

/**
 * This function prints, on every system start, information about the System.
 * @todo Only to dip_update once on startup and use a temp variable here
 */
void print_startup(void) {
	uint8_t prev_DEBUG_ENABLE_value = DEBUG_ENABLE;
	// Startup Debug Infos
	if (LCD_PRESENT) {
		lcd_print_status(NULL);
	}
	DEBUG_ENABLE = 1;
	debug_WriteString_P(PSTR("Motorsteuerung "));
	debug_PutString(version);
	debug_WriteString_P(PSTR("\n----------------------------\n\n"));
	debug_WriteString_P(PSTR("DIP-Schalter Einstellungen:\n"));
	if (INTERFACE_TWI)
		debug_WriteString_P(PSTR("DIP1: ON   Interface = TWI\n"));
	else
		debug_WriteString_P(PSTR("DIP1: OFF  Interface = UART\n"));
	if (prev_DEBUG_ENABLE_value)
		debug_WriteString_P(PSTR("DIP2: ON   Debug-Ausgaben aktiv\n"));
	else
		debug_WriteString_P(PSTR("DIP2: OFF  Debug-Ausgaben inaktiv\n"));
	if (LCD_PRESENT)
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
	debug_WriteString_P(PSTR("\n"));
	DEBUG_ENABLE = prev_DEBUG_ENABLE_value;
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
	local_time_flags = timer_global_flags;
	timer_global_flags = 0;
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
	extern uint8_t ACTIVE_BRAKE_WHEN_IDLE;

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
	} else if (ACTIVE_BRAKE_WHEN_IDLE) {
		drive_brake_active();
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
		if (LCD_PRESENT)
			lcd_update_screen();

//		debug_WriteString_P(PSTR("main.c : main() :  parser_update()\n"));
		// Update the order parser
		parser_update();

//		debug_WriteString_P(PSTR("main.c : main() :  queue_update()\n"));
		// Housekeeping for the order queue
		queue_update();
	}

	// Should be never reached
	return 0;
}
/*@}*/

#include "lib/definitions.h"
#include "lib/debug.h"
#include "lib/dip.h"
#include "lib/flags.h"
#include "lib/io.h"
#include "lib/motor.h"
#include "lib/order.h"
#include "lib/queue.h"
#include "lib/status.h"
#include <inttypes.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>

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
	// init all subsystems
	dip_init();
	motor_init();
	timer_init();
	irq_init();
	queue_init();
	status_init();
	io_init();

	// activate interrupts
	sei();

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
 *	This function prints, on every system start, information about the System.
 */
//TODO correct the "PSTR" uses
void print_startup(void) {
	// Startup Debug Infos
	flag_set(DEBUG_ENABLE);
	debug_ResetTerminal();
	debug_WriteString_P(PSTR("Motorsteuerung V2.9.20090501\r\n"));
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
	if (flag_read_and_clear(TIMER_1MS))
		flag_local_set( &local_time_flags, TIMER_1MS );
	if (flag_read_and_clear(TIMER_10MS))
		flag_local_set( &local_time_flags, TIMER_10MS );
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
	static order_t *current_order = NULL;

	if (queue_order_available())
		current_order = queue_get_current_order();
	else
		drive_brake_active(); // TODO Needed? At the moment in here to make sure we mimic the old drivers behaviour
	if (current_order != NULL) {
		order_process(current_order);
		//order_check_triggers(current_order); // FIXME: Do not need this here, should be in the order functions
		if (current_order->status & ORDER_STATUS_DONE) {
			drive_brake_active_set();
			queue_pop();
			current_order = NULL;
		}
	}
}

void lcd_print_status(void) {
	// TODO Implement
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
		// Copy the global timer flags to a local variable (but global for this file)
		copy_timer_flags();

		// Processes the next or current order
		process_orders();		

		// If a LCD is pluged in we get nice status messages on it
		if (flag_read(LCD_PRESENT))
			lcd_print_status();

		// Update the order parser
		parser_update();

		// Housekeeping for the order queue
		queue_update();
		
		// Update the global status of the program
		status_update();

		// Write Output Buffer to medium
		io_flush();
	}

	// Should be never reached
	return 0;
}

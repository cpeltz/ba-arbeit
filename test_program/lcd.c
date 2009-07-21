#include "lcd.h"
#include "timing.h"

/**
 * Pointer für den Memory-Mapped-IO Zugriff auf das LCD:
 * WR_COM_REG: write command
 * WR_DAT_REG: write data
 * RD_COM_REG: read command
 * RD_DAT_REG: read data
 */
byte xdata *WR_COM_REG = WR_COM_REG_;
byte xdata *WR_DAT_REG = WR_DAT_REG_;
byte xdata *RD_COM_REG = RD_COM_REG_;
byte xdata *RD_DAT_REG = RD_DAT_REG_;

/**
 * Gibt zurueck, ob das Display bereit ist, einen Command
 * entgegenzunehmen.
 */
byte lcd_busy() {
	return 0x80 & *(RD_COM_REG);
}

/**
 * Blockiert, bis das Display ein Kommando annehmen kann.
 */
void lcd_comm_wait() {
	while (lcd_busy());
}

/**
 * Sendet ein Kommando ans LCD und blockiert dabei solange, bis
 * das LCD diesen annehmen kann.
 * Parameter:
 *  - command: Das Kommando
 */
void lcd_command(byte command) {
	lcd_comm_wait();
	*(WR_COM_REG) = command;
}

/**
 * Sendet ein Zeichen ans LCD und blockiert dazu solange wie nötig,
 * siehe lcd_command(byte).
 * Parameter
 *  - c: Das Zeichen (ASCII)
 */
void lcd_data(unsigned char c) {
	lcd_comm_wait();
	*(WR_DAT_REG) = c;
}

/**
 * Initialisiert das Display wie im Datenblatt beschrieben (8-Bit).
 */
void lcd_init() {
	wait_ms(16);
	*(WR_COM_REG) = 0x30;
	wait_ms(5);
	*(WR_COM_REG) = 0x30;
	wait_100us(2);
	*(WR_COM_REG) = 0x30;
	wait_ms(5);
	
	lcd_command(0x38);
	lcd_command(0x0c);
	lcd_command(0x01);
	lcd_command(0x06);
}

/**
 * Löscht den Inhalt des LCD und setzt den Cursor auf Position 0.
 */
void lcd_clear() {
	lcd_command(0x01);
}

/**
 * lcd_set_cursor_pos(l, c) setzt den Cursor des Displays in Zeile l
 * und Spalte c.
 * Parameter:
 *  - line: Die Zeile (0-3)
 *  - col: Die Spalte (0-19)
 */
/*void lcd_set_cursor_pos(unsigned char line,unsigned char col) {
	unsigned char addr;
	if(col >= 20) col = 19;
	switch(line) {
	default:
	case 0:
		addr = 0x00;
		break;
	case 1:
		addr = 0x40;
		break;
	case 2:
		addr = 0x14;
		break;
	case 3:
		addr = 0x54;
		break;
	}
	addr += col;
	lcd_command(addr | 0x80);
}*/

/**
 * Schreibt den gegebenen Text an die aktuelle Cursor-Position.
 * Parameter:
 *  - buf: Der Text
 */
/*void lcd_write_text(char* buf) {
	while(*buf) {
		lcd_data(*buf);
		buf++;
	}
}*/

/**
 * Löscht das Display und schreibt den gegebenen Text ab Position 0.
 * Parameter:
 *  - buffer: Der Text
 */
void lcd_print_string(char *buffer) {
	char *cur = buffer;
	lcd_clear();
	while (*cur) {
		lcd_data(*cur);
		cur++;
		switch (cur - buffer) {
		case 20:
			lcd_command(0x40 | 0x80); // Jump to 0x40
			break;
		case 40:
			lcd_command(0x14 | 0x80); // Jump to 0x10
			break;
		case 60:
			lcd_command(0x54 | 0x80); // Jump to 0x50
			break;
		}
	}
}

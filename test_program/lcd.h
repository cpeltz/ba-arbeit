
/*******************************************************
 **  Mikroprozessorlabor                              **
 **                                                   **
 **  Institut f�r Betriebssysteme und Rechnerverbund  **
 **  Technische Universit�t Braunschweig              **
 *******************************************************/

// File :  lcd.h 
// Modul:  N�here Bezeichnung des Projektes oder Teilprojektes
// Vers.:  x.yy
// Autor:  Name, Vorname 

// Letzte �nderung: 2007.11.08.
//*****************************


#ifndef __lcd_h__
#define __lcd_h__

#ifndef __byte__
#define __byte__
typedef unsigned char byte;
#endif //__byte__



// ----- Definitionen -----
// ========================

// Als Vorlage f�r die Aufgabe 4 werden einige Konstanten
// zur Ansteuerung des LC-Displays definiert.


// Registeradressen der LCD-Anzeige        R/W  RS
//                                         =A1  =A0
//                                   ---------------									
#define WR_COM_REG_      0xff00   //        0    0 
#define WR_DAT_REG_      0xff01   //        0    1
#define RD_COM_REG_      0xff02   //        1    0 
#define RD_DAT_REG_      0xff03   //        1    1

// ----- Deklarationen -----
// =========================

// Hier werden einige m�gliche Kommandos f�r die Displayansteuerung 
// als Funktionen deklariert. Es ist f�r die Modularit�t sinnvoll, 
// solchen modulspezifischen Funktionen (und auch globalen Variablen) 
// ein Pr�fix voranzustellen.
// Die Implementationen dieser Funktionen fehlen noch und geh�ren
// in die Datei "lcd.c".


void lcd_command(unsigned char wert);
void lcd_data(unsigned char wert);
void lcd_print_string(unsigned char *buffer);
void lcd_init(void);
void lcd_clear();
//void lcd_set_cursor_pos(unsigned char line,unsigned char col);
//void lcd_write_text(char* buf);

#endif// _lcd_h_





/*******************************************************
 **  Mikroprozessorlabor                              **
 **                                                   **
 **  Institut f�r Betriebssysteme und Rechnerverbund  **
 **  Technische Universit�t Braunschweig              **
 *******************************************************/

// File :  i2c_master.h 
// Modul:  i2c (Master-Modus)
// Vers.:  1.01
// Autor:  Rilk, Markus 

// Letzte �nderung: 2002.09.30.
//*****************************


#ifndef __i2c_master_h__
#define __i2c_master_h__

#ifndef __byte__
#define __byte__
typedef unsigned char byte;
#endif //__byte__


// ----- Definitionen -----
// ========================


// ----- Deklarationen -----
// =========================


// !!!!! WICHTIG !!!!!
// Die Arrays, die den Funtionen "i2c_send(...)" und "i2c_receive(...)" �bergeben werden
// m�ssen im externen Speicher liegen, d.h. sie m�ssen mit "xdata" deklariert werden.


// Initialisiere das I2C-Interface
void i2c_init();
	

// Sende Daten zum Slave
// <buffer>:	buffer[0] - Adresse des Empf�ngers	
//				buffer[n] (n != 0) - Daten, die gesendet werden sollen
// <size>:		Gr��e von <buffer> 
// <return>:	Anzahl der gesendeten Daten
byte i2c_send(byte *buffer, byte size);


// Empfange Daten vom Slave
// <buffer>:	buffer[0] - Adresse des Empf�ngers	
//				buffer[n] (n != 0) - Speicher f�r die empfangenen Daten
// <size>:		Gr��e von <buffer> 
// <return>:	Anzahl der empfangenen Daten
byte i2c_receive(byte *buffer, byte size);


#endif //__i2c_master_h__











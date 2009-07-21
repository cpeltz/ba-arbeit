
/*******************************************************
 **  Mikroprozessorlabor                              **
 **                                                   **
 **  Institut für Betriebssysteme und Rechnerverbund  **
 **  Technische Universität Braunschweig              **
 *******************************************************/

// File :  i2c_master.c 
// Modul:  i2c (Master-Modus)
// Vers.:  1.01
// Autor:  Rilk, Markus 
// Letzte Änderung: 2002.09.30.
//*****************************



// ----- Includes -----
//=====  


//================

#include <REG552.H>               // special function register declarations
#include "i2c_master.h"
#include "config.h"

// ----- Deklarationen, die nur innerhalb dieser Datei gebraucht werden -----
// ==========================================================================

void checkStatus(byte i2cStatus);


// ----- Funktionsdefinitionen -----
// =================================


// --- Globale Variablen ---

byte xdata *i2cBuf;		 // Zeiger auf Sende- bzw. Empfangspuffer
byte xdata i2cBufSize;	 // Größe des aktuellen Puffers
byte xdata i2cBufIndex;  // Aktuelle Position im Puffer
byte xdata i2cAttempts;	 // Anzahl der Verbindungsversuche
byte xdata i2cStartByte; // Slave-Adresse + Lesen oder Schreiben
bit i2cBusy; 			 // Busy-Flag



// Interrupthandler für I2C-Bus Schnittstelle (S1-Interrupt)
void i2c_interrupt() interrupt 5 {	
	byte xdata i2cStatus = S1STA;	// Kopiert Status in temporäre Variable
	checkStatus(i2cStatus);
}


// Prüfe den Status
void checkStatus(byte i2cStatus) {
	switch(i2cStatus) 
	{
	case 0x08:
		// State: 0x08 - START Zustand wurde gesendet
		// Mode: Master-Transeiver
		// Action: SLR+RW werden gesendet, ACK Bit empfangen
	case 0x10:
		// State: 0x10 - START wurde wiederholt gesendet
		// Mode: Master-Transeiver
		// Action: SLR+RW werden gesendet, ACK Bit empfangen
		S1DAT = i2cStartByte;				
		S1CON = 0xC5;		
		break;
			
	case 0x18:
		// State: 0x18 - SLR+W wurde gesendet, ACK Bit empfangen
		// Mode: Master-Transmitter
		// Action: Datenbyte senden oder Bus freigeben
	case 0x28:
		// State: 0x28 - Datenbyte wurde gesendet, ACK Bit empfangen
		// Mode: Master-Transmitter
		// Action: Datenbyte senden oder Bus freigeben
		i2cBufIndex++;
		if(i2cBufIndex == i2cBufSize)
		{
			// Es wurde der gesamte Puffer übertragen -> Ende
			S1CON = 0xD5;
			i2cBusy = 0;
		}
		else
		{
			// Übertrage die nächsten Daten
			S1DAT = i2cBuf[i2cBufIndex];
                            			S1CON = 0xC5;
		}
		break;

	case 0x48:
		// State: 0x48 - SLA+R wurde gesendet, NOT ACK Bit empfangen
		// Mode: Master-Receiver
		// Action: Wiederholte Startbedingung oder Abbruch, falls zuviele Versuche
	case 0x20:
		// State: 0x20 - SLA+W wurde gesendet, NOT ACK Bit empfangen
		// Mode: Master-Transmitter
		// Action: Wiederholte Startbedingung oder Abbruch, falls zuviele Versuche
		if(i2cAttempts < I2C_MAX_ATTEMPTS)
		{
			S1CON = 0xE5; // Sende erneut Startbedingung
			i2cAttempts++;
		}
		else
		{
			S1CON = 0xD5; // Sende Stopbedingung
			i2cBusy = 0;
		}
		break;

	case 0x50:
		// State 0x50: Datenbyte wurde empfangen, ACK Bit gesendet
		// Mode: Master-Receiver
		// Action: Bus freigeben oder Empfang weiterer Daten
		i2cBuf[i2cBufIndex] = S1DAT;
	case 0x40:
		// State 0x40: SLA+R gesendet, Ack Bit empfangen
		// Mode: Master-Receiver
		// Action: Empfange Daten, Ack Bit senden
		i2cBufIndex++;
		
		if(i2cBufIndex == i2cBufSize - 1) 
		{
			S1CON = 0xC1;	// NOT ACK senden
		} 
		else 
		{	
			S1CON = 0xC5;	// ACK senden und nächstes Byte empfangen
		}
		break;

	case 0x58:
		// State 0x58: Datenbyte wurde empfangen, NOT ACK Bit wurde gesendet
		// Mode: Master-Receiver
		// Action: Datenbyte lesen, Datentransfer stoppen, Bus freigeben 
		i2cBuf[i2cBufIndex] = S1DAT;
		i2cBufIndex++;
	default:
		// Empfang abbrechen und Bus freigeben
		S1CON = 0xD5;
		i2cBusy = 0;
		break;
	}
}


// Sende Daten zum Slave
// <buffer>:	buffer[0] - Adresse des Empfängers	
//				buffer[n] (n != 0) - Daten, die gesendet werden sollen
// <size>:		Größe von <buffer> 
// <return>:	Anzahl der gesendeten Daten
byte i2c_send(byte* buffer, byte size) 
{
	i2cBuf = buffer;
	i2cBufSize = size;
	i2cAttempts = 0;
		
	i2cStartByte = *i2cBuf << 1;	// LSB == 0 für Schreibvorgang
	
	i2cBusy = 1;
	STA = 1;	// Sende Startbit
	i2cBufIndex = 0;	
	
	while(i2cBusy == 1);	// Warten bis Sendeende oder Abbruch
	return i2cBufIndex;
}


// Empfange Daten vom Slave
// <buffer>:	buffer[0] - Adresse des Empfängers	
//				buffer[n] (n != 0) - Speicher für die empfangenen Daten
// <size>:		Größe von <buffer> 
// <return>:	Anzahl der empfangenen Daten
byte i2c_receive(byte *buffer, byte size) 
{
	if(size < 2) return 0;

	i2cBuf = buffer;
	i2cBufSize = size;
	i2cAttempts = 0;

	i2cStartByte = (*i2cBuf << 1) | 0x01; 	// LSB == 1 für Lesevorgang
	i2cBusy = 1;
	STA = 1;	// Sende Startbit
	i2cBufIndex = 0;	
	
	while(i2cBusy == 1);	// Warten bis Empfangsende oder Abbruch
	return i2cBufIndex;
}


// Initialisiere I2C-Interface
void i2c_init() 
{
	i2cBusy = 0;		// Busy Flag rücksetzen 
	S1CON = 0xC5;		// 100khz i2c 
	
	S1ADR = 0x01;		// Eigene Slave Adresse 
	
	SDA = 1;			// Portpins setzen, um Port-Read zu ermöglichen 
	SCL = 1;
	
	ES1 = 1;
	ENS1 = 1;			// I2C Interrupt freigeben 
	EA = 1;				// Interrupte generell freigeben
}


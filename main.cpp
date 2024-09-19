#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>

#include "VoltageStatistics.h"
#include "ATMega32_utility_bib.h"
#include "can.h"


void Timer_1_Compare_ISR_init();

volatile uint32_t Zeitstempel=0;
volatile uint32_t Zeitstempel_100ms=0;
volatile bool flag_100ms=false;


int main ()
{
	DDRC = 0xFF;			// LED-Port: output
	PORTC = 0x00;			// LEDs ein
    _delay_ms(100);
    PORTC = 0xFF;            // LEDs aus
    VoltageStatistics V;
	USART UART(8,0,1,57600);	// USART init 8 Zeichenbits , keien Paritätsbits , 1 Stoppbit, 9600 Zeichen pro Sekunde
	can_init(BITRATE_500_KBPS);      // CAN init 500 kbit/s
    char buffer[100];		// Buffer zur Zwschischenspeicherung von Zeichenketten
	
   	can_t sendmsg_activ;    // Message-Objekt auf dem Stack anlegen
	      sendmsg_activ.id= 0x33;
		  sendmsg_activ.flags.rtr=0;
		  sendmsg_activ.length=2;
	can_t sendmsg_nactiv;    // Message-Objekt auf dem Stack anlegen
	      sendmsg_nactiv.id= 0x32;
		  sendmsg_nactiv.flags.rtr=0;
		  sendmsg_nactiv.length=0;

          
	sprintf(buffer,"Probepruefung Teil 2\n\r\n\r");	// Zeichenkette erzeugen und in dn Zwischenspeicher schreiben
	UART.UsartPuts(buffer);		   // Ausgabe
	
	_delay_ms(500);
	
	Timer_1_Compare_ISR_init(); // Timer 1 init und start
    
	uint16_t lastValueMilliVolt=0;
	uint8_t cnt=0;
	while (1)
	{
	    
		if(flag_100ms==true){
		cli();
        Zeitstempel_100ms=(Zeitstempel*100);
        sei();
		cnt++;
		flag_100ms=false;
        lastValueMilliVolt= (V.convertMilliVoltage(V.readValue(6)));
		V.addValue(lastValueMilliVolt);
		if((lastValueMilliVolt<500)&&(cnt%2==0))
		{
          V.sendCanMessages(false,&sendmsg_nactiv,lastValueMilliVolt,Zeitstempel_100ms);  
        }   
         if((lastValueMilliVolt>=500))
		{
          V.sendCanMessages(true,&sendmsg_activ,lastValueMilliVolt,Zeitstempel_100ms);
		}
		if(cnt%10==0)
		{
		  TGL_BIT(LED_PORT,3);
		  V.showStatistics();
		  cnt=0;

		}
		}
	
	}
	return 0;
}


// Init Timer1 Compare ISR
// Vorteiler = 1024 , OCR1 = xx => 100ms pro Überlauf
// 16MHZ / 1024 = 15,625 kHz  1/15,625 khz = 64 us = 0,064ms
// bei 65536 Schritten 4,19 Sekunden
// 100ms / 0,064 ms = 1562,5 Schritte => Zähler zählt bis 1562 => 99,97ms 
// 2. Vorteiler 256
// 16MHZ/ 256 = 62,5 kHz 1/ 62,5kHz= 16uS = 0,016 ms
// 100ms / 0,016ms = 6250 Schritte = 100ms

void Timer_1_Compare_ISR_init()
{
    // Timer1 initialisieren
  	// Zählerstandsregister zurücksetzen
  	TCNT1 = 0;// Startwert
  	// Vergleichsregister setzen
  	OCR1A  = 6250-1; // => Zähler zählt bis 6250 => 100ms 
  	
  	// Konfigurationsregister:
  	// WGM1 = Toggle OC1A on Compare Match;
  	// COM1 =Toggle OC1A on Compare Match;
  	// CS1 = Vorteiler 256
	TCCR1A = (0<<COM1A1) | (1<<COM1A0) | (0<<COM1B1) | (0<<COM1B0) | (0<<FOC1A) | (0<<FOC1B) | (0<<WGM11) | (0<<WGM12);
    TCCR1B = (0<<ICNC1) | (0<<ICES1) | (0<<WGM13) | (1<<WGM12) | (1<<CS12) | (0<<CS11) | (0<<CS10);
	
	//Interruptmaskenregister setzen
	//COIE1A = INT auslösen bei Überlauf Timer1 aktiv
	TIMSK=(1<<OCIE1A);
	
	//Interrupts global freigeben
	sei();
	
	// Interrupts nicht mehr freigeben
	// cli()
	
	return;
}


// Compare " Vergleichsregister" Interrupt
//------------------------------------------------------------------------------
//  Interrupt Service Routinen
//------------------------------------------------------------------------------
// Interrupt-Service-Routine für den Interrupt bei Überlauf des Timer1
// ISR: Schlüsselwort für Compiler, dass dies eine ISR ist
// TIMER0_COMP_vect: Information an den Compiler, mit welchem Interrupt
// diese ISR verknüpft werden soll. Der Bezeichner "TIMER1_COMPA_vect"
//ist wie alle anderen ISR-Bezeichner in "avr/interrupt.h" definiert.
ISR(TIMER1_COMPA_vect)
{
// tue etwas beim Überlauf von OCR1
Zeitstempel=Zeitstempel+1; // Alle 100ms Zeitstemple inkrementieren
flag_100ms=true;

TGL_BIT(LED_PORT,0);

 
}

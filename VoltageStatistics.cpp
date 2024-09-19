#include "VoltageStatistics.h"


VoltageStatistics::VoltageStatistics()
{
	memset(values,0,sizeof(values));
}

uint16_t VoltageStatistics::readValue(uint8_t kanal)
{
  ADC_read pin(kanal);
  return (pin.adcwert()); // ADC in Pin kanal

}

uint16_t VoltageStatistics::convertMilliVoltage(uint16_t value)
{
  return((((1000UL*refVoltage*value)/adc_steps)));

}

bool VoltageStatistics::sendCanMessages(bool activ, can_t* send, uint16_t lastValue,uint32_t Zeitstempel_100ms)
{  

   USART UART(8,0,1,57600);	// USART init 8 Zeichenbits , keien Paritätsbits , 1 Stoppbit, 9600 Zeichen pro Sekunde
   char buffer[100];		// Buffer zur Zwschischenspeicherung von Zeichenketten
   
   // Ausgabe
  if(activ==true)
  {
     
     send->data[0]=(lastValue & 0xFF);
	 send->data[1]=((lastValue>>8)&0xFF);

     
   	 if(can_send_message(send))		// CAN-Nachricht versenden
		{
		 TGL_BIT(LED_PORT,1);
		 sprintf(buffer,"%ld ms: 0x%x - - %d \n\r",(Zeitstempel_100ms),send->id,lastValue);	// Zeichenkette erzeugen und in dn Zwischenspeicher schreiben
         UART.UsartPuts(buffer);		   // Ausgabe
		 return true;
		}else{
		 return false;
		}
		
	}
  
   if(activ==false)
  {
      
      
      if(can_send_message(send))		// CAN-Nachricht versenden
		{
		 TGL_BIT(LED_PORT,2);
	     sprintf(buffer,"%ld ms: 0x%x \n\r",Zeitstempel_100ms,send->id);	// Zeichenkette erzeugen und in dn Zwischenspeicher schreiben
	     UART.UsartPuts(buffer);		   // Ausgabe		
		 return true;
		}else{
		 return false;
		}
	
	} 
	 return false;
	}
     
 void VoltageStatistics::addValue(uint16_t voltageInMillivolt){

       
    // Alles stellen nach links verschieben und den neuen Wert an Stelle 10 speichern
    for(uint8_t i=0;i<valueSize ;i++)
	{
        values[i] = values[i+1];

		}
    values[valueSize-1] = voltageInMillivolt;



 }    

void VoltageStatistics::showStatistics(){
    USART UART(8,0,1,57600);	// USART init 8 Zeichenbits , keien Paritätsbits , 1 Stoppbit, 9600 Zeichen pro Sekunde
     char buffer[250];
	 uint16_t min = 65535;
	 uint16_t max = 0;
	 uint32_t summe = 0;
	 uint16_t mittelwert;


	for(uint8_t i=0;i<valueSize;i++)
	{
	  	summe=summe+values[i];
		if(values[i] < min) {
			min = values[i];
			}
		if(values[i] > max) {
			max = values[i];
			}	
        

	}
	mittelwert= (uint16_t)((summe/((uint32_t)valueSize)));
	sprintf(buffer,"mittlere Spannung: %d mV; kleinste Spannung: %d mV; groesste Spannung: %d mV \n\r",mittelwert, min, max);	// Zeichenkette erzeugen und in dn Zwischenspeicher schreiben
	UART.UsartPuts(buffer);		   // Ausgabe
}
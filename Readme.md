Dette Repository indeholder et projekt, hvor der modtages karakterer på én af de 2 UART's et AtMega328PB board indeholder. Hvis der kobles et XBEE Shield på AtMega328PB boardet med en XBEE Radio på, 
kan der sendes karakterer fra en anden ZigBee Device til boardet. Typisk vil den anden ZigBee device være en anden XBEE. Styringen af hvilken UART (0 eller 1), der anvendes på AtMega328PB boardet, styres ved brug af en 
enkelt define AVR8_UART_NUMBER i filen UART.h. Sættes denne define til 0 anvendes UART 0, og sættes denne define til 1 anvendes UART 1. 
Hvis UART 0 anvendes, kan man bestemme, om det skal være en en PC, der sender karakterer til AtMga328PB boardet ved at sætte switchen på XBEE Shield'et til position USB. Hvis man sætter switchen på XBEE Shield'et 
til position XBBE vil karakterer på AtMega328PB boardet blove modtaget fra XBEE radionen, der modtager disse "Over the air" fra en anden ZigBee Device. Dette vil typisk være en anden XBEE radio.  
Uanset hvilken position swictchen på XBEE Shield'et står i, vil karakterer der sendes fra AtMega328PB boardet blive sendt både til en anden ZigBee Device og til PC. 
Hvis man sætter AVR8_UART_NUMBER i filen UART.h til 1, skal man koble en CP2102 USB-Seriel Konverter til sit AtMega328PB board for at kunne sende - og modtage karaketerer fra en tilkoblet PC. 

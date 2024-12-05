/*
 * AdaFruit_Display_C.c
 *
 * Created: 23-06-2023 00:18:42
 * Author : ltpe
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "Lib/ProjectDefines.h"
#include "Lib/Display_Lib/ssd1306.h"
#include "Lib/UART_Lib/UART.h"

static volatile bool CharacterReceivedFromUART = false;
static volatile uint8_t DisplayLineCounter = 0;
static volatile uint8_t I2C_Address = SSD1306_ADDR;
static volatile uint8_t I2CDisplayStatus = SSD1306_SUCCESS;

static volatile RingBufferStruct RingBufferReceiver;

void ReceiceCharacterFromUart(char ReceivedCharacter)
{
	if ((RingBufferReceiver.BufferHead + 1) % DisplayBufferSize == RingBufferReceiver.BufferTail) 
	{
		// Buffer is full, discard the oldest character
		RingBufferReceiver.SkippedCharacter = RingBufferReceiver.DisplayBuffer[RingBufferReceiver.BufferTail];
		RingBufferReceiver.SkippedCharacterIndicator = true;
		RingBufferReceiver.BufferOverrunCounter++;
		RingBufferReceiver.BufferTail = (RingBufferReceiver.BufferTail + 1) % DisplayBufferSize;
	}
	RingBufferReceiver.DisplayBuffer[RingBufferReceiver.BufferHead] = ReceivedCharacter;
	RingBufferReceiver.BufferHead = (RingBufferReceiver.BufferHead + 1) % DisplayBufferSize;
	CharacterReceivedFromUART = true;
}

void WriteReceivedCharacterFromUARTInDisplay(volatile RingBufferStruct RingBufferStructPointerVariable)
{
	char ReceivedCharacterFromUARTString[2];
	
	while (RingBufferStructPointerVariable.BufferTail != RingBufferStructPointerVariable.BufferHead) 
	{
		SSD1306_SetPosition(0, DisplayLineCounter++);
		SSD1306_DrawString("character : ", NORMAL);
		
		sprintf(ReceivedCharacterFromUARTString, "%c", RingBufferStructPointerVariable.DisplayBuffer[RingBufferStructPointerVariable.BufferTail]);
		SSD1306_DrawString(ReceivedCharacterFromUARTString, BOLD);
		  // Increment the cursor position for the next line

		// Move the tail pointer to the next character
		RingBufferStructPointerVariable.BufferTail = (RingBufferStructPointerVariable.BufferTail + 1) % DisplayBufferSize;
		DisplayLineCounter = DisplayLineCounter % MAX_NUMBER_OF_LINES_IN_DISPLAY;
	}
}

void WriteReceivedCharacterFromUARTOnUART(volatile RingBufferStruct *RingBufferStructPointer)
{
	char ReceivedCharacterFromUARTString[2];
	
	while (RingBufferStructPointer->BufferTail != RingBufferStructPointer->BufferHead)
	{
		printf("\ncharacter : ");
		
		sprintf(ReceivedCharacterFromUARTString, "%c", RingBufferStructPointer->DisplayBuffer[RingBufferStructPointer->BufferTail]);
		printf(ReceivedCharacterFromUARTString);
		printf("\n");
		
		// Move the tail pointer to the next character
		RingBufferStructPointer->BufferTail = (RingBufferStructPointer->BufferTail + 1) % DisplayBufferSize;
	}
}

void I2C_0_init()
{
#if defined (_AVR_ATMEGA328PB_H_INCLUDED)
	/* Enable TWI0 */
	PRR0 &= ~(1 << PRTWI0);
	
	TWCR0 = (1 << TWEN)   /* TWI0: enabled */
	| (1 << TWIE) /* TWI0 Interrupt: enabled */
	| (0 << TWEA) /* TWI0 Acknowledge: disabled */;
	
	/* SCL bitrate = F_CPU / (16 + 2 * TWBR0 * TWPS value) */
	/* Configured bit rate is 100.000kHz, based on CPU frequency 8.000MHz */
	TWBR0 = 0x20;          /* SCL bit rate: 100.000kHZ before prescaling */
	TWSR0 = 0x00 << TWPS0; /* SCL precaler: 1, effective bitrate = 100.000kHz */
#else
	/* Enable TWI */
	PRR &= ~(1 << PRTWI);
	
	TWCR = (1 << TWEN)   /* TWI: enabled */
	| (1 << TWIE) /* TWI Interrupt: enabled */
	| (0 << TWEA) /* TWI Acknowledge: disabled */;
	
	/* SCL bitrate = F_CPU / (16 + 2 * TWBR * TWPS value) */
	/* Configured bit rate is 100.000kHz, based on CPU frequency 8.000MHz */
	TWBR = 0x20;          /* SCL bit rate: 100.000kHZ before prescaling */
	TWSR = 0x00 << TWPS0; /* SCL precaler: 1, effective bitrate = 100.000kHz */
#endif
}

int main(void)
{
	RingBufferReceiver.BufferHead = 0;
	RingBufferReceiver.BufferTail = 0;
	RingBufferReceiver.BufferOverrunCounter = 0;
	RingBufferReceiver.SkippedCharacter = false;
	
	SetupFunctionCallbackPointer(ReceiceCharacterFromUart);
	// Kodelinjen herover kan også skrives som vist i den udkommenterede kodelinje 
	// herunder. Dette skyldes, at når man i C skriver navnet på en funktion,
	// mener man implicit adressen på funktionen !!!
	//SetupFunctionCallbackPointer(&ReceiceCharacterFromUart);
	
	I2C_0_init();
	RS232Init();
	Enable_UART_Receive_Interrupt();
	
	// Enable global interrupt
	sei();

	// init ssd1306
	I2CDisplayStatus = SSD1306_Init(I2C_Address);
	if (SSD1306_SUCCESS != I2CDisplayStatus)
	{
		printf("Display NOT properly initialized !!!\n");
	}
	else
	{
		printf("Display properly initialized !!!\n");
	}

	// clear screen
	SSD1306_ClearScreen();
	
	while (1)
	{
		if (true == CharacterReceivedFromUART)
		{
			CharacterReceivedFromUART = false;
			if (SSD1306_SUCCESS == I2CDisplayStatus)
			{
				WriteReceivedCharacterFromUARTInDisplay(RingBufferReceiver);
			}
			else
			{
				WriteReceivedCharacterFromUARTOnUART(&RingBufferReceiver);
			}
		}
		
		if (true == RingBufferReceiver.SkippedCharacterIndicator)
		{
			RingBufferReceiver.SkippedCharacterIndicator = false;
			printf("\nSkipped character from Uart %c\n", RingBufferReceiver.SkippedCharacter);
			printf("Number of Skipped Characters : %d\n", RingBufferReceiver.BufferOverrunCounter);
		}
	}
}


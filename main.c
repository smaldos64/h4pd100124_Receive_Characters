/*
 * AdaFruit_Display_C.c
 *
 * Created: 23-06-2023 00:18:42
 * Author : ltpe
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>

#include "Lib/ProjectDefines.h"
#include "Lib/Display_Lib/ssd1306.h"
#include "Lib/UART_Lib/UART.h"

//static volatile char ReceivedCharacterFromUART;
static volatile bool CharacterReceivedFromUART = false;
static volatile uint8_t DisplayLineCounter = 0;
static volatile uint8_t I2C_Address = SSD1306_ADDR;

static volatile char DisplayBuffer[DisplayBufferSize];
static uint8_t BufferHead = 0;
static uint8_t BufferTail = 0;
static uint16_t BufferOverrunCounter = 0;
static volatile bool SkippedCharacterIndicator = false;
static volatile char SkippedCharacter;

void ReceiceCharacterFromUart(char ReceivedCharacter)
{
	//ReceivedCharacterFromUART = ReceivedCharacter;
	
	if ((BufferHead + 1) % DisplayBufferSize == BufferTail) 
	{
		// Buffer is full, discard the oldest character
		SkippedCharacter = DisplayBuffer[BufferTail];
		SkippedCharacterIndicator = true;
		BufferOverrunCounter++;
		BufferTail = (BufferTail + 1) % DisplayBufferSize;
	}
	DisplayBuffer[BufferHead] = ReceivedCharacter;
	BufferHead = (BufferHead + 1) % DisplayBufferSize;
	CharacterReceivedFromUART = true;
}

void WriteReceivedCharacterFromUARTInDisplay()
{
	char ReceivedCharacterFromUARTString[2];
	
	while (BufferTail != BufferHead) {
		SSD1306_SetPosition(0, DisplayLineCounter++);
		SSD1306_DrawString("character : ", NORMAL);
		
		sprintf(ReceivedCharacterFromUARTString, "%c", DisplayBuffer[BufferTail]);
		SSD1306_DrawString(ReceivedCharacterFromUARTString, BOLD);
		  // Increment the cursor position for the next line

		// Move the tail pointer to the next character
		BufferTail = (BufferTail + 1) % DisplayBufferSize;
		DisplayLineCounter = DisplayLineCounter % MAX_NUMBER_OF_LINES_IN_DISPLAY;
	}
}

//void WriteReceivedCharacterFromUARTInDisplay(char ReceivedCharacterFromUART)
//{
	//char ReceivedCharacterFromUARTString[2];
	//
	//SSD1306_SetPosition(0, DisplayLineCounter++);
	//SSD1306_DrawString("character : ", NORMAL);
	//
	//sprintf(ReceivedCharacterFromUARTString, "%c", ReceivedCharacterFromUART);
	//SSD1306_DrawString(ReceivedCharacterFromUARTString, BOLD);
		//
	////SSD1306_UpdateScreen(I2C_Address);
	//DisplayLineCounter = DisplayLineCounter % MAX_NUMBER_OF_LINES_IN_DISPLAY;
//}

//void WriteReceivedCharacterFromUARTInDisplay(char ReceivedCharacterFromUART)
//{
	//char ReceivedCharacterFromUARTString[2];
	//
	//SSD1306_SetPosition(0, DisplayLineCounter++);
	//SSD1306_DrawString("character : ", NORMAL);
	//
	//sprintf(ReceivedCharacterFromUARTString, "%c", ReceivedCharacterFromUART);
	//SSD1306_DrawString(ReceivedCharacterFromUARTString, BOLD);
	//
	////SSD1306_UpdateScreen(I2C_Address);
	//DisplayLineCounter = DisplayLineCounter % MAX_NUMBER_OF_LINES_IN_DISPLAY;
//}

void SetOrResetPortBit(uint8_t *PointerToPort, uint8_t PortPosititon, bool PortBitValue)
{
	if (true == PortBitValue)
	{
		*PointerToPort |= (1 << PortPosititon);
	}
	else
	{
		*PointerToPort &= ~(1 << PortPosititon);
	}
}

int main(void)
{
	uint8_t *PortPointer;
	uint8_t PortBitPosititon;
	bool IsPortOn;
	
	PortPointer = (uint8_t *)0x0025;
	PortBitPosititon = 1;
	IsPortOn = true;
	
	SetOrResetPortBit(PortPointer, PortBitPosititon, IsPortOn);
	PortPointer--;
	SetOrResetPortBit(PortPointer, PortBitPosititon, IsPortOn);
	
	
	//if (true == IsPortOn)
	//{
		//*PortPointer |= (1 << PortBitPosititon);
	//}
	//else
	//{
		//*PortPointer &= ~(1 << PortBitPosititon);
	//}
	//
	//PortPointer--;
	//if (true == IsPortOn)
	//{
		//*PortPointer |= (1 << PortBitPosititon);
	//}
	//else
	//{
		//*PortPointer &= ~(1 << PortBitPosititon);
	//}
	
	
	SetupFunctionCallbackPointer(ReceiceCharacterFromUart);
	// Kodelinjen herover kan også skrives som vist i den udkommenterede kodelinje 
	// herunder. Dette skyldes, at når man i C skriver navnet på en funktion,
	// mener man implicit adressen på funktionen !!!
	//SetupFunctionCallbackPointer(&ReceiceCharacterFromUart);
	RS232Init();
	Enable_UART_Receive_Interrupt();
	
	// Enable global interrupt
	sei();

	// init ssd1306
	SSD1306_Init(I2C_Address);

	// clear screen
	SSD1306_ClearScreen();
	//SSD1306_UpdateScreen(I2C_Address);
	
	while (1)
	{
		if (true == CharacterReceivedFromUART)
		{
			CharacterReceivedFromUART = false;
			//WriteReceivedCharacterFromUARTInDisplay(ReceivedCharacterFromUART);
			WriteReceivedCharacterFromUARTInDisplay();
		}
		
		if (true == SkippedCharacterIndicator)
		{
			SkippedCharacterIndicator = false;
			printf("\nSkipped character from Uart %c\n", SkippedCharacter);
			printf("Number of Skipped Characters : %d\n", BufferOverrunCounter);
		}
	}
}


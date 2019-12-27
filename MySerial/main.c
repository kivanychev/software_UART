/*
 * MySerial.c
 *
 * Author : kivanychev
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

#include "uart.h"

#define TRUE    1
#define FALSE   0

// USART settings
#define FOSC            16000000
#define USART_BR        9600
#define MYUBRR          FOSC/16/USART_BR - 1

//==========================================================================================
// LOCAL VARIABLES
//==========================================================================================

volatile unsigned short canSendByte = FALSE;
volatile unsigned char byteToSend;

//==========================================================================================
// IMPLEMENTATION
//==========================================================================================

void UART_InitUSART0()
{
    UBRR0 = MYUBRR;
    UCSR0A = 0;
    UCSR0B = (1 << TXEN0) | (1 << RXEN0) | (1 << RXCIE0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}


//-----------------------------------------------------
// Sample call-back function for collecting
// received bytes
//-----------------------------------------------------
void ReadCallback(unsigned char c)
{
    // Resend via USART to PC
    UDR0 = c;
}

//-----------------------------------------------------
// Program entry point
// Test UART
//-----------------------------------------------------
int main(void)
{
    
    char *heloString = "Hello\r\n";
    int ind = 0;

    UART_InitUSART0();
    UART_Init(ReadCallback);
        
    while(heloString[ind] != '\0')
    {
        while(UART_SendByte(heloString[ind]) != UART_OK)
            ;   // Wait for UART to be ready for sending
            
        ind++;
    }

    /* Replace with your application code */
    while (1)
    {
        if(canSendByte)
        {
            UDR0 = byteToSend;
        	UART_SendByte(byteToSend);      // Send to BT
            canSendByte = FALSE;
        }            
    }
}

//-----------------------------------------------------
// Resend byte to BT
//-----------------------------------------------------
ISR(USART_RX_vect)
{
	byteToSend = UDR0;
	canSendByte = TRUE;

}

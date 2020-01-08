/*
 * SW_UART_USB_Receiver.c
 *
 * Created: 07.01.2020 15:24:37
 * Author : ZorG
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
// IMPLEMENTATION
//==========================================================================================

void UART_InitUSART0()
{
    UBRR0 = MYUBRR;
    UCSR0A = 0;
    UCSR0B = (1 << TXEN0) | (1 << RXEN0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}



void SW_UART_ReadCallback(unsigned char byte)
{
    // Send byte from SW UART to USART
    UDR0 = byte;
}



int main(void)
{
    UART_InitUSART0();
    UART_Init(SW_UART_ReadCallback);
    
    while (1) 
    {
    }
}


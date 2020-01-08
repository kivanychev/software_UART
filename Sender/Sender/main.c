/*
 * SW_UART_USB_Receiver.c
 *
 * Created: 07.01.2020 15:24:37
 * Author : ZorG
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

#include "uart.h"


//==========================================================================================
// IMPLEMENTATION
//==========================================================================================


void SW_UART_ReadCallback(unsigned char byte)
{
}


int main(void)
{
    UART_Init(SW_UART_ReadCallback);
    
    char byteToSend;
    char *str = "Hello from sender\r\n";
    unsigned short ind = 0;

    while (1) 
    {
        byteToSend = str[ind];
        ind++;
        if( str[ind] == '\0')
        {
            ind = 0;
        }
        
        UART_SendByte(byteToSend);
    }
}


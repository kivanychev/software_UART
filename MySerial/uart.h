#ifndef __UART_H__
#define __UART_H__


#include <avr/io.h>
#include <avr/interrupt.h>

//==========================================================================================
// CONSTANTS
//==========================================================================================

#define BAUD_9600       207

// Modes of UART
#define MODE_RECEIVE    2
#define MODE_SEND       1
#define MODE_READY      0

// Data frame stages
#define STAGE_WAIT      0
#define STAGE_START_BIT 1
#define STAGE_DATA_BITS 2
#define STAGE_STOP_BIT  3
#define STAGE_FINISH    4

// Pins
#define TXPIN           PORTD3
#define RXPIN           PORTD2

// UART states
#define UART_OK             0
#define UART_ERROR_STOPBIT  1
#define UART_BUSY           2

//==========================================================================================
// MACROS
//==========================================================================================

#define StartTimer() TCCR0B = (1 << CS01); TCNT0 = BAUD_9600 - 1
#define StopTimer() TCCR0B = 0

//==========================================================================================
// LOCAL TYPES
//==========================================================================================

typedef void (* TReadCallback_fp)(unsigned char);

//==========================================================================================
// FUNCTION PROTOTYPES
//==========================================================================================

extern void			  UART_Init(TReadCallback_fp fp);
extern unsigned char  UART_SendByte(unsigned char byte);
extern void           UART_SetReadCallback(TReadCallback_fp callback_fp);

#endif
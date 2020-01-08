/*
 * uart.c
 *
 * Author : Kirill Ivanychev
 */ 

#include "uart.h"

//==========================================================================================
// LOCAL VARIABLES
//==========================================================================================

volatile unsigned char uart_mode = MODE_READY;
volatile unsigned char uart_stage = STAGE_WAIT;

volatile unsigned char uart_received_byte;
volatile unsigned char uart_byte_to_send;

volatile unsigned char uart_current_bit_position = 1;

volatile unsigned char receive_state = UART_OK;

// Callback pointer for reading bytes
TReadCallback_fp callback_pointer;


//==========================================================================================
// IMPLEMENTATION
//==========================================================================================

//-----------------------------------------------------
//  Initialize UART
//  PD2 - RXD
//  PD3 - TXD
//-----------------------------------------------------
void UART_Init(TReadCallback_fp cb_fp)
{
    // Set CTC mode
    TCCR0A = (1 << WGM01);
    OCR0A = BAUD_9600;                      // 104.2 us
    TIMSK0 = (1 << OCIE0A);

    PORTD |= (1 << TXPIN);                  // Set silence at TX
    PORTD |= (1 << RXPIN);                  // Set pull-up

    DDRD |= (1 << TXPIN);                   // Set as output
    DDRD &= (0xFF - (1 << RXPIN));          // Set as input

    uart_current_bit_position = 1;
    uart_mode = MODE_READY;
    uart_stage = STAGE_WAIT;
    uart_received_byte = 0;
    uart_byte_to_send = 0;

    // COnfigure INT0 for triggering start bit
    EICRA = (1 << ISC01);
    EIMSK = (1 << INT0);

    callback_pointer = cb_fp;

    sei();
}


//-----------------------------------------------------
// Sends byte over UART
//-----------------------------------------------------
unsigned char UART_SendByte(unsigned char byte)
{
    uart_byte_to_send = byte;

    if(uart_mode != MODE_READY)
    {
        return UART_BUSY;
    }

    uart_mode = MODE_SEND;
    uart_stage = STAGE_WAIT;
    
    StartTimer();
    
    while(uart_mode != MODE_READY)
        ;
        
    return UART_OK;
}

//-----------------------------------------------------
// SW UART timer handler
//-----------------------------------------------------
ISR(TIMER0_COMPA_vect)
{
    unsigned char portPins;
    
    switch(uart_mode)
    {
        case MODE_RECEIVE:
            switch(uart_stage)
            {
                case STAGE_WAIT:
                    uart_current_bit_position = 1;
                    // Fall through as we already receiver start bit signal from INT0
            
                    case STAGE_START_BIT:
                    uart_stage = STAGE_DATA_BITS;
                    break;

                case STAGE_DATA_BITS:
                    portPins = PIND;
            
                    if( (portPins & (1 << RXPIN)) != 0)
                    {
                        // Got '1'
                        uart_received_byte |= uart_current_bit_position;
                    }
            
                    uart_current_bit_position = (uart_current_bit_position << 1);

                    if(uart_current_bit_position == 0)
                    {
                        // Finished receiving
                        uart_stage = STAGE_STOP_BIT;
                        // Unlock INT0 for next reception
                        EIMSK = (1 << INT0);
                    }


                    break;

                case STAGE_STOP_BIT:
                    portPins = PIND;

                    if( (portPins & (1 << RXPIN)) == 0)
                    {
                        // Got '0'
                        receive_state = UART_ERROR_STOPBIT;
                    }
                    else
                    {
                        receive_state = UART_OK;
                    }
            
                    // Fall through as we reached stop bit time

                case STAGE_FINISH:
                    uart_stage = STAGE_WAIT;
                    uart_mode = MODE_READY;
            
                    callback_pointer(uart_received_byte);
                    StopTimer();
                    break;

                default:
                    break;

            }

            break;

        case MODE_SEND:
            switch(uart_stage)
            {
                case STAGE_WAIT:
                    PORTD &= (0xFF - (1 << TXPIN));
                    uart_stage = STAGE_START_BIT;
                    uart_current_bit_position = 1;
                    break;

                case STAGE_START_BIT:
                    uart_stage = STAGE_DATA_BITS;

                    // Fall Through!
            
                case STAGE_DATA_BITS:
                    if( (uart_byte_to_send & uart_current_bit_position) != 0)
                    {
                        PORTD |= (1 << TXPIN);  // Set '1'
                    }
                    else
                    {
                        PORTD &= (0xFF - (1 << TXPIN)); // Set '0'
                    }

                    uart_current_bit_position = (uart_current_bit_position << 1);

                    if(uart_current_bit_position == 0)
                    {
                        uart_stage = STAGE_STOP_BIT;
                    }

                    break;

                case STAGE_STOP_BIT:
                    PORTD |= (1 << TXPIN);  // Set '1' as Stop bit
                    uart_stage = STAGE_FINISH;
                    break;

                case STAGE_FINISH:
                    uart_stage = STAGE_WAIT;
                    uart_mode = MODE_READY;

                    StopTimer();
                    break;

                default:
                    break;

            }

            break;
        
        default:
            break;

    }
}

//-----------------------------------------------------
// INT0 handle
// Triggers for falling edge as the start bit
// Starts data reception
//-----------------------------------------------------
ISR(INT0_vect)
{
    if(uart_mode == MODE_READY)
    {
        
        // Lock INT0 interrupt.
        // It will be unlocked on STAGE_FINISH for MODE_RECEIVE
        EIMSK &= (0xff - (1 << INT0));
        
        uart_mode = MODE_RECEIVE;
        uart_stage = STAGE_WAIT;
        
        uart_received_byte = 0;
        
        StartTimer();
        
    }
}


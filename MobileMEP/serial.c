/*
 * serial.c
 *
 * Created:		10/05/2018 19:51:00
 * Author:		Graham
 * Description:	Module responsible for receiving data from the serial port
 */ 

#include "serial.h"
#include "utilities.h"
#include "schedular.h"

#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#define MAXIMUM_TX_BUFFER_SIZE						256
#define MAXIMUM_RX_BUFFER_SIZE						256

#define FRAMING_ERROR								0x10
#define DATA_OVERRUN_ERROR							0x08
#define PARITY_ERROR								0x04
#define ANY_SERIAL_ERRORS							(FRAMING_ERROR | DATA_OVERRUN_ERROR | PARITY_ERROR)
#define NO_SERIAL_ERRORS							0x00

#define UART_RX_INTERRUPT_ENABLE					0x80
#define UART_DATA_REGISTER_EMPTY_INTERRUPT_ENABLE	0x20
#define RECEIVER_ENABLE								0x10
#define TRANSMITTER_ENABLE							0x08
#define DOUBLE_UART_TRANSMISSION_SPEED				0x02

#define EIGHT_DATA_BITS								0x06

#define BAUD_RATE_115200							8

// receive variables 
static unsigned char srl_receive_data_buffer[MAXIMUM_RX_BUFFER_SIZE];
static unsigned short srl_receive_input_index;
static unsigned short srl_receive_output_index;
static unsigned short srl_receive_bytes_in_buffer;

// transmit variables
static unsigned char srl_transmit_data_buffer[MAXIMUM_TX_BUFFER_SIZE];
static unsigned short srl_transmit_input_index;
static unsigned short srl_transmit_output_index;
static unsigned short srl_transmit_bytes_in_buffer;

static unsigned char srl_index_of_task_to_signal_on_rx = NO_TASK;

// name:	SRL_Init
// Desc:	Module initialisation function sets up serial port.
void SRL_Init(void)
{
	// clear the buffers
	memset((void*)&srl_receive_data_buffer, 0, MAXIMUM_RX_BUFFER_SIZE);
	memset((void*)&srl_transmit_data_buffer, 0, MAXIMUM_TX_BUFFER_SIZE);
	//
	srl_receive_input_index = 0;
	srl_receive_output_index = 0;
	srl_receive_bytes_in_buffer = 0;
	//
	srl_transmit_input_index = 0;
	srl_transmit_output_index = 0;
	srl_transmit_bytes_in_buffer = 0;
	//
	// set up the serial port for 115200-8-n-1
	UCSR0A = DOUBLE_UART_TRANSMISSION_SPEED;
	//
	UCSR0B = UART_RX_INTERRUPT_ENABLE | 
				RECEIVER_ENABLE | TRANSMITTER_ENABLE;
	//
	UCSR0C = EIGHT_DATA_BITS;
	//
	UBRR0H = GET_16_BIT_MSB(BAUD_RATE_115200);
	UBRR0L = GET_16_BIT_LSB(BAUD_RATE_115200);
}

// name:	SRL_Add_data_to_transmit_buffer
// Desc:	adds data bytes to the transmit buffer.
void SRL_Add_data_to_transmit_buffer(const unsigned char *data_to_add_ptr, unsigned short data_length)
{
	int i;
	
	// disable interrupts while we add data to the tx buffer
	cli();
	
	// add the data to the transmit buffer
	for(i = 0; i < data_length; i++)
	{
		srl_transmit_data_buffer[srl_transmit_input_index++] = *(data_to_add_ptr + i);
		//
		if(MAXIMUM_TX_BUFFER_SIZE == srl_transmit_input_index)
		{
			srl_transmit_input_index = 0;	
		}
	}
	//
	// if this is the first data then start sending it
	if(0 == srl_transmit_bytes_in_buffer)
	{
		// add data to the tx buffer
		UDR0 = srl_transmit_data_buffer[srl_transmit_output_index];
		//
		// and enable the data register empty interrupt
		UCSR0B |= UART_DATA_REGISTER_EMPTY_INTERRUPT_ENABLE;
	}
	//
	// add these bytes to the bytes in transmit buffer
	srl_transmit_bytes_in_buffer += data_length;
	//
	// re enable interrupts
	sei();
}

// name:	SRL_Get_data_byte_from_receive_buffer
// Desc:	returns a single byte from the recieve buffer.
unsigned char SRL_Get_data_byte_from_receive_buffer(void)
{
	unsigned char next_byte;
	
	// disable interrupts while we remove data from the buffer
	cli();
	//
	next_byte = srl_receive_data_buffer[srl_receive_output_index];
	//
	// if if there is more data in the buffer then adjust the index's
	if(srl_receive_bytes_in_buffer != 0)
	{
		srl_receive_bytes_in_buffer--;
		//
		srl_receive_output_index++;
		//
		if(MAXIMUM_RX_BUFFER_SIZE == srl_receive_output_index)
		{
			srl_receive_output_index = 0;
		}
	}
	//
	// re-enable interrupts 
	sei();
	//
	return next_byte;
}

// name:	SRL_Get_number_of_bytes_in_rx_buffer
// Desc:	returns the number of bytes in the rx buffer.
unsigned short SRL_Get_number_of_bytes_in_rx_buffer(void)
{
	return srl_receive_bytes_in_buffer;
}

// name:	SRL_Set_task_to_signal_on_data_rx
// Desc:	sets the task to signal when data is received.
void SRL_Set_task_to_signal_on_data_rx(unsigned char index_of_task_to_signal)
{
	srl_index_of_task_to_signal_on_rx = index_of_task_to_signal;
}

// name:	ISR(USART0_RX_vect)
// Desc:	UART receive interrupt.
ISR(USART0_RX_vect)
{
	unsigned char received_byte;
	unsigned char receiver_status;
	
	// get status and received byte 
	receiver_status = UCSR0A;
	received_byte = UDR0;
	//
	// only add the byte to software buffer if there are no errors
	if(NO_SERIAL_ERRORS == (receiver_status & ANY_SERIAL_ERRORS))
	{
		srl_receive_data_buffer[srl_receive_input_index++] = received_byte;
		srl_receive_bytes_in_buffer++;
		//
		// if there is a task to trigger and this is the 1st byte then trigger the task
		if((NO_TASK != srl_index_of_task_to_signal_on_rx)&&
			(1 == srl_receive_bytes_in_buffer))
		{
			SCH_Signal_task(srl_index_of_task_to_signal_on_rx, DATA_TRIGGERED);		
		}
		//
		if(MAXIMUM_RX_BUFFER_SIZE == srl_receive_input_index)
		{
			srl_receive_input_index = 0;
		}
	}
}

// name:	ISR(USART0_UDRE_vect)
// Desc:	UART Data register empty interrupt.
ISR(USART0_UDRE_vect)
{
	// this byte has been transmitted so subtract bytes in buffer by 1 and move the output index
	srl_transmit_bytes_in_buffer--;
	srl_transmit_output_index++;
	//
	if(MAXIMUM_TX_BUFFER_SIZE == srl_transmit_output_index)
	{
		srl_transmit_output_index = 0;
	}
	//
	// if there are more bytes in the tx buffer then send them out
	if(0 != srl_transmit_bytes_in_buffer)
	{
		// send the next byte out
		UDR0 = srl_transmit_data_buffer[srl_transmit_output_index];
	}
	else
	{
		// disable the UDRE interrupt
		UCSR0B &= ~UART_DATA_REGISTER_EMPTY_INTERRUPT_ENABLE;
	}
}
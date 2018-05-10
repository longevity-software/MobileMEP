/*
 * serial.c
 *
 * Created:		10/05/2018 19:51:00
 * Author:		Graham
 * Description:	Module responsible for receiving data from the serial port
 */ 

#include "serial.h"

#include <string.h>

#define MAXIMUM_TX_BUFFER_SIZE	256
#define MAXIMUM_RX_BUFFER_SIZE	256

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
}
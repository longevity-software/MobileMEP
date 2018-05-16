/*
 * communications.c
 *
 * Created:		12/05/2018 19:29:02
 * Author:		Graham
 * Description:	Module responsible for parsing and creating communications packets
 */ 

#include "communications.h"
#include "schedular.h"
#include "serial.h"
#include "utilities.h"
#include "crc.h"

#include <string.h>

#define MAX_PACKET_BYTES					200
#define MAX_RX_PACKETS						10

// packet byte position #defines
#define START_OF_PACKET_BYTE				0
#define START_OF_PACKET						0x73
#define BYTE_COUNT_BYTE						1
#define COMMAND_BYTE						2
#define COMMAND_IS_REQUEST_NOT_RESPONSE		0x80
#define STATUS_BYTE							3
#define START_OF_ADDITIONAL_DATA			4
#define CRC_LSB_BYTE(byte_count)			((byte_count) + START_OF_ADDITIONAL_DATA)
#define CRC_MSB_BYTE(byte_count)			((byte_count) + START_OF_ADDITIONAL_DATA + 1)
#define DEFAULT_BYTES_INCLUDED_IN_CRC		4
#define END_OF_PACKET_BYTE(byte_count)		((byte_count) + START_OF_ADDITIONAL_DATA + 2)
#define END_OF_PACKET						0xD9
#define DEFAULT_PACKET_SIZE					7

// commands #defines 
#define NO_ADDITIONAL_BYTES					0

#define COMMAND_GET_STATUS					0x10

static unsigned char cms_received_packets[MAX_RX_PACKETS][MAX_PACKET_BYTES];
static unsigned char cms_recieved_packet_input_index;
static unsigned char cms_received_packet_populate_index;
static unsigned char cms_received_packet_parse_index;

static unsigned char cms_received_packet_populate_task_index;
static unsigned char cms_received_packet_parse_task_index;
static unsigned char cms_packet_to_transmit[MAX_PACKET_BYTES];

static void populate_received_packet(void);
static void parse_received_packet(void);
static inline void process_received_command(unsigned char command);
static inline void process_received_response(unsigned char command);

// name:	CMS_Init
// Desc:	Module initialisation function.
void CMS_Init(void)
{
	memset((void*)&cms_received_packets[0][0], 0, (MAX_RX_PACKETS * MAX_PACKET_BYTES));
	memset((void*)&cms_packet_to_transmit[0], 0, MAX_PACKET_BYTES);
	//
	cms_recieved_packet_input_index = 0;
	//
	// add the receive packet tasks to the schedular task list
	cms_received_packet_populate_task_index = SCH_Add_task_to_list(populate_received_packet);
	cms_received_packet_parse_task_index = SCH_Add_task_to_list(parse_received_packet);
	//
	cms_received_packet_populate_index = 0;
	cms_received_packet_parse_index = 0;
	//
	SRL_Set_task_to_signal_on_data_rx(cms_received_packet_populate_task_index);
}

// name:	populate_received_packet
// Desc:	populates the receive packet with bytes from the serial port.
static void populate_received_packet(void)
{
	unsigned char rx_byte;
	
	// check if there are bytes in the serial rx buffer first 
	if(0 != SRL_Get_number_of_bytes_in_rx_buffer())	
	{
		// get the next rx byte 
		rx_byte = SRL_Get_data_byte_from_receive_buffer();
		//
		// populate the byte into the correct position of the packet based on the cms_received_packet_input_index
		switch(cms_recieved_packet_input_index)
		{
			case START_OF_PACKET_BYTE:
				//
				// we are looking for a start of packet so only add the byte to the packet and 
				// move on if this byte is a start of packet byte
				if(rx_byte == START_OF_PACKET)
				{
					cms_received_packets[cms_received_packet_populate_index][START_OF_PACKET_BYTE] = rx_byte;
					//
					cms_recieved_packet_input_index = BYTE_COUNT_BYTE;
				}
				//
				break;
			case BYTE_COUNT_BYTE:
				//
				cms_received_packets[cms_received_packet_populate_index][BYTE_COUNT_BYTE] = rx_byte;
				//
				cms_recieved_packet_input_index = COMMAND_BYTE;
				//
				break;
			default:
				//
				// now that the byte count has been received we know how long the packet needs to be so all bytes can come in here
				//
				// add this byte to the packet
				cms_received_packets[cms_received_packet_populate_index][cms_recieved_packet_input_index] = rx_byte;
				//
				// check if this is the last byte we need
				if(cms_recieved_packet_input_index == END_OF_PACKET_BYTE(cms_received_packets[cms_received_packet_populate_index][BYTE_COUNT_BYTE]))
				{
					// full packet received so reset input index
					cms_recieved_packet_input_index = 0;
					//
					// trigger the task to parse the packet
					SCH_Signal_task(cms_received_packet_parse_task_index, SELF_TRIGGERED);
					//
					// increment the index to populate the next packet 
					if(MAX_RX_PACKETS == ++cms_received_packet_populate_index)
					{
						cms_received_packet_populate_index = 0;
					}
				}
				else
				{
					// packet not yet full so increment input index
					cms_recieved_packet_input_index++;
				}
				break;
		}
		//
		// re-signal the task if there are more bytes in the serial rx buffer
		if(0 != SRL_Get_number_of_bytes_in_rx_buffer())
		{
			SCH_Signal_task(cms_received_packet_populate_task_index, DATA_TRIGGERED);
		}
	}
}

// name:	parse_received_packet
// Desc:	parses a received packet.
static void parse_received_packet(void)
{
	unsigned char byte_count;
	unsigned short packet_crc;
	unsigned short calculated_crc;
	
	// get the byte count from the received packet so we know where the end of packet and crc bytes will be.
	byte_count = cms_received_packets[cms_received_packet_parse_index][BYTE_COUNT_BYTE];
	//
	// check for end of packet 
	if(cms_received_packets[cms_received_packet_parse_index][END_OF_PACKET_BYTE(byte_count)] == END_OF_PACKET)
	{
		// pull crc from the packet
		packet_crc = MAKE_16_BITS(cms_received_packets[cms_received_packet_parse_index][CRC_MSB_BYTE(byte_count)],
									cms_received_packets[cms_received_packet_parse_index][CRC_LSB_BYTE(byte_count)]);
		//
		// calculate the crc on the packet
		calculated_crc = CRC_Calculate_crc(&cms_received_packets[cms_received_packet_parse_index][START_OF_PACKET_BYTE], (DEFAULT_BYTES_INCLUDED_IN_CRC + byte_count));
		//
		// confirm crc's match 
		if(packet_crc == calculated_crc)
		{
			// check if this is a request or a response to one of our requests
			if((cms_received_packets[cms_received_packet_parse_index][COMMAND_BYTE] & COMMAND_IS_REQUEST_NOT_RESPONSE) == COMMAND_IS_REQUEST_NOT_RESPONSE)
			{
				process_received_command(cms_received_packets[cms_received_packet_parse_index][COMMAND_BYTE]);
			}
			else
			{
				process_received_response(cms_received_packets[cms_received_packet_parse_index][COMMAND_BYTE]);				
			}
		}
	}
	//
	// increment index to parse the next packet when this task is signalled again
	if(MAX_RX_PACKETS == ++cms_received_packet_parse_index)
	{
		cms_received_packet_parse_index = 0;
	}
}

// name:	process_received_command
// Desc:	performs the specified action for the passed command.
static inline void process_received_command(unsigned char command)
{	
	unsigned short response_crc;
	Boolean valid_command;
	
	// assume a valid command
	valid_command = True;
	//
	// remove request/response bit as this will be a response
	command &= ~COMMAND_IS_REQUEST_NOT_RESPONSE;
	//
	// perform action associated with the received command
	switch(command)
	{
		case COMMAND_GET_STATUS:
			//
			// populate the response
			cms_packet_to_transmit[BYTE_COUNT_BYTE] = NO_ADDITIONAL_BYTES;
			break;
		default:
			//
			// command is not recognised so set valid_command to false
			valid_command = False;
			break;
	}
	//
	// only send repsonse if command is valid
	if(valid_command == True)
	{
		// populate beginning bytes
		cms_packet_to_transmit[START_OF_PACKET_BYTE] = START_OF_PACKET;
		cms_packet_to_transmit[COMMAND_BYTE] = command;
		cms_packet_to_transmit[STATUS_BYTE] = 0x01;
		//
		response_crc = CRC_Calculate_crc(&cms_packet_to_transmit[START_OF_PACKET_BYTE], (DEFAULT_BYTES_INCLUDED_IN_CRC + cms_packet_to_transmit[BYTE_COUNT_BYTE]));
		//
		cms_packet_to_transmit[CRC_LSB_BYTE(cms_packet_to_transmit[BYTE_COUNT_BYTE])] = GET_16_BIT_LSB(response_crc);
		cms_packet_to_transmit[CRC_MSB_BYTE(cms_packet_to_transmit[BYTE_COUNT_BYTE])] = GET_16_BIT_MSB(response_crc);
		//
		cms_packet_to_transmit[END_OF_PACKET_BYTE(cms_packet_to_transmit[BYTE_COUNT_BYTE])] = END_OF_PACKET;
		//
		// send the response to the serial port. 
		SRL_Add_data_to_transmit_buffer(&cms_packet_to_transmit[START_OF_PACKET_BYTE], (DEFAULT_PACKET_SIZE + cms_packet_to_transmit[BYTE_COUNT_BYTE]));
	}
	else
	{
		// do nothing this command will be retried 
	}
}

// name:	process_received_response
// Desc:	Fill in when we fill in the function.
static inline void process_received_response(unsigned char command)
{
	// TBD fill out when i reach the point of sending requests
}
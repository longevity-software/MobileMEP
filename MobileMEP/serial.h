/*
 * serial.h
 *
 * Created:		10/05/2018 19:51:15
 * Author:		Graham
 * Description:	Module responsible for receiving data from the serial port
 */ 


#ifndef SERIAL_H_
#define SERIAL_H_

void SRL_Init(void);

void SRL_Add_data_to_transmit_buffer(const unsigned char *data_to_add_ptr, unsigned short data_length);
unsigned char SRL_Get_data_byte_from_receive_buffer(void);
unsigned short SRL_Get_number_of_bytes_in_rx_buffer(void);
void SRL_Set_task_to_signal_on_data_rx(unsigned char index_of_task_to_signal);

#endif /* SERIAL_H_ */
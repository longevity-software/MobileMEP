/*
 * MobileMEP.c
 *
 * Created: 10/05/2018 18:58:24
 * Author : Graham
 */ 

#include "hardware.h"
#include "serial.h"

#include <util/delay.h>
#include <avr/interrupt.h>

static unsigned char temp_data[3] = {0x30,0x0A,0x0D};

// name:	main
// Desc:	main program entry point, initialises other modules and 
//			calls background processor in main super loop.
int main(void)
{
	// call module initialisation functions
	HDW_Init();
	//
	SRL_Init();
	//
	// enable interrupts now the modules are set up
	sei();
	//
	// main super loop 
	while (1) 
	{
		// testing code, will be reworked
		HDW_Set_heartbeat_led_state(LED_ON);
		//
		_delay_ms(1000);
		//
		HDW_Set_heartbeat_led_state(LED_OFF);
		//
		_delay_ms(1000);
		//
		SRL_Add_data_to_transmit_buffer(temp_data, 3);
	}
}


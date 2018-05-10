/*
 * MobileMEP.c
 *
 * Created: 10/05/2018 18:58:24
 * Author : Graham
 */ 

#include "hardware.h"
#include "serial.h"

#include <util/delay.h>

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
	// main super loop 
	while (1) 
	{
		// testing code, will be reworked
		HDW_Set_heartbeat_led_state(LED_ON);
		//
		_delay_ms(200);
		//
		HDW_Set_heartbeat_led_state(LED_OFF);
		//
		_delay_ms(1000);
	}
}


/*
 * MobileMEP.c
 *
 * Created: 10/05/2018 18:58:24
 * Author : Graham
 */ 

#include "hardware.h"
#include "serial.h"
#include "schedular.h"
#include "communications.h"

#include <util/delay.h>
#include <avr/interrupt.h>

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
	SCH_Init();
	//
	CMS_Init();
	//
	// enable interrupts now the modules are set up
	sei();
	//
	// main super loop 
	while (1) 
	{
		// if there is a background task then run it
		SCH_Run_background_tasks();
	}
}


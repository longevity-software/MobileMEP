/*
 * hardware.c
 *
 * Created:			10/05/2018 19:09:10
 * Author:			Graham
 * Description:	Module responsible for hardware (GPIO) set up
 */ 

#include "hardware.h"

#include <avr/io.h>

#define HEARTBEAT_LED_PORT		PORTB
#define HEARTBEAT_LED_PORT_DDR	DDRB
#define HEARTBEAT_LED_PIN		(1<<0)

#define SERIAL_PORT_PORT		PORTD
#define SERIAL_PORT_PORT_DDR	DDRD
#define SERIAL_PORT_RX_PIN		(1<<0)
#define SERIAL_PORT_TX_PIN		(1<<1)

static LED_STATES hdw_heartbeat_led_state;

// name:	HDW_Init
// Desc:	Module initialisation function sets ports up.
void HDW_Init(void)
{
	// set the heartbeat led pin as an output
	HEARTBEAT_LED_PORT_DDR |= HEARTBEAT_LED_PIN;
	//
	// set state to on to ensure it turns off with call to function
	hdw_heartbeat_led_state = LED_ON;
	HDW_Set_heartbeat_led_state(LED_OFF);
	//
	// set the serial port rx as an input and tx as an output 
	SERIAL_PORT_PORT_DDR |= SERIAL_PORT_TX_PIN;
	SERIAL_PORT_PORT_DDR &= ~SERIAL_PORT_RX_PIN;
	//
}

// name:	HDW_Set_heartbeat_led_state
// Desc:	Modifies the heartbeat led if the state is changing.
void HDW_Set_heartbeat_led_state(LED_STATES new_state)
{
	// if the state is changing then adjust the port line 
	if(new_state != hdw_heartbeat_led_state)
	{
		hdw_heartbeat_led_state = new_state;
		//
		if(LED_ON == hdw_heartbeat_led_state)
		{
			HEARTBEAT_LED_PORT &= ~HEARTBEAT_LED_PIN;
		}
		else
		{
			HEARTBEAT_LED_PORT |= HEARTBEAT_LED_PIN;
		}
	}
}
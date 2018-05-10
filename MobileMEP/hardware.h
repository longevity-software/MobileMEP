/*
 * hardware.h
 *
 * Created:			10/05/2018 19:09:29
 * Author:			Graham
 * Description:	Module responsible for hardware (GPIO) set up
 */ 

#ifndef HARDWARE_H_
#define HARDWARE_H_

#define F_CPU 8000000

typedef enum
{
	LED_OFF,
	LED_ON	
}LED_STATES;

void HDW_Init(void);
void HDW_Set_heartbeat_led_state(LED_STATES new_state);


#endif /* HARDWARE_H_ */
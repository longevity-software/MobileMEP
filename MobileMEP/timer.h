/*
 * timer.h
 *
 * Created:		13/05/2018 11:42:31
 * Author:		Graham
 * Description:	Module responsible for task timings
 */ 


#ifndef TIMER1_H_
#define TIMER1_H_

#include "utilities.h"

typedef enum
{
	TIMER_COUNT_NONE	=	0,
	TIMER_COUNT_10_MS	=	1,
	TIMER_COUNT_500_MS	= 50,
	TIMER_COUNT_1_S		= 100
}TIMER_COUNT;

void TMR_Init(void);
void TMR_Set_timer_to_signal_task(unsigned char task_to_signal, TIMER_COUNT timer_count, TIMER_COUNT reload_count);


#endif /* TIMER1_H_ */
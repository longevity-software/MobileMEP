/*
 * timer.c
 *
 * Created:		13/05/2018 11:43:03
 * Author:		Graham
 * Description:	Module responsible for task timings
 */ 

#include "timer.h"
#include "schedular.h"

#include <avr/interrupt.h>

#define MAX_TIMERS	20

#define CLEAR_TIMER_ON_COMPARE_MATCH			0x02
#define CLOCK_DIVIDED_BY_1024					0x05
#define TIMER_COMPARE_MATCH_INTERRUPT_ENABLE	0x02

#define OUTPUT_COMPARE_VALUE			78

typedef struct  
{
	Boolean timer_active;
	unsigned short timer_count;
	unsigned short reload_count;
	unsigned char task_to_signal;
}TIMER_STRUCT;

static inline void reset_timer(unsigned char timer_index);
static inline void process_all_timers(void);

static TIMER_STRUCT timers[MAX_TIMERS];

// name:	TMR_Init
// Desc:	Module initialisation function.
void TMR_Init(void)
{
	int i;
	
	// initialise the timers array
	for(i = 0; i < MAX_TIMERS; i++)
	{
		reset_timer(i);
	}
	//
	// set up timer interrupt
	TCCR0A = CLEAR_TIMER_ON_COMPARE_MATCH;
	TCCR0B = CLOCK_DIVIDED_BY_1024;
	TIMSK0 = TIMER_COMPARE_MATCH_INTERRUPT_ENABLE;
	//
	OCR0A = OUTPUT_COMPARE_VALUE;
}

// name:	TMR_Set_timer_to_signal_task
// Desc:	Sets up a timer to signal the passed task.
void TMR_Set_timer_to_signal_task(unsigned char task_to_signal, TIMER_COUNT timer_count, TIMER_COUNT reload_count)
{
	int i;
	Boolean timer_slot_found;
	unsigned char timer_slot_index;
	
	timer_slot_found = False;
	//
	// find the first empty slot
	for(i = 0; ((i < MAX_TIMERS) && (timer_slot_found == False)); i++)
	{
		if(False == timers[i].timer_active)
		{
			timer_slot_index = i;
			//
			timer_slot_found = True;			
		}
	}
	//
	// if a slot has been found then set up the timer
	if(True == timer_slot_found)
	{
		timers[timer_slot_index].timer_count = timer_count;
		timers[timer_slot_index].reload_count = reload_count;
		timers[timer_slot_index].task_to_signal = task_to_signal;
		timers[timer_slot_index].timer_active = True;
	}
	else
	{
		// TBD add error 
	}
}

// name:	reset_timer
// Desc:	resets the timer values to default.
static inline void reset_timer(unsigned char timer_index)
{
	timers[timer_index].timer_active = False;
	timers[timer_index].timer_count = TIMER_COUNT_NONE;
	timers[timer_index].reload_count = TIMER_COUNT_NONE;
	timers[timer_index].task_to_signal = NO_TASK;
}

// name:	process_all_timers
// Desc:	decrements all active timers and signals any 
//			tasks which the associated timer has elapsed.
static inline void process_all_timers(void)
{
	int i;
	
	for(i = 0; i < MAX_TIMERS; i++)
	{
		if(True == timers[i].timer_active)
		{
			if((unsigned char)TIMER_COUNT_NONE == --timers[i].timer_count)
			{
				// timer count has elapsed so signal task
				SCH_Signal_task(timers[i].task_to_signal, TIMER_TRIGGERED);
				//
				// if the reload count is not TIMER_COUNT_NONE then refresh 
				// the timer else kill it
				if(TIMER_COUNT_NONE == timers[i].reload_count)
				{
					reset_timer(i);
				}
				else
				{
					timers[i].timer_count = timers[i].reload_count;
				}
			}
		}
	}
}

// name:	ISR(TIMER0_COMPA_vect)
// Desc:	timer 0 compare match A interrupt.
ISR(TIMER0_COMPA_vect)
{
	// process all the timers for time out
	process_all_timers();
}
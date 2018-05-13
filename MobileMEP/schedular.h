/*
 * schedular.h
 *
 * Created:		11/05/2018 19:51:41
 * Author:		Graham
 * Description:	Module responsible for scheduling background tasks
 */ 


#ifndef SCHEDULAR_H_
#define SCHEDULAR_H_

#define NO_TASK 0xFF

// sources which can trigger a task to run
typedef enum
{
	NOT_TRIGGERED = 0,
	TIMER_TRIGGERED,
	SELF_TRIGGERED,
	DATA_TRIGGERED
}TASK_TRIGGER_SOURCE;

void SCH_Init(void);
unsigned char SCH_Add_task_to_list(void(*task_function_ptr)(void));
void SCH_Signal_task(unsigned char task_index, TASK_TRIGGER_SOURCE source);
void SCH_Run_background_tasks(void);
TASK_TRIGGER_SOURCE SCH_Get_task_trigger_source(void);


#endif /* SCHEDULAR_H_ */
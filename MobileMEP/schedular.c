/*
 * schedular.c
 *
 * Created:		11/05/2018 19:52:49
 * Author:		Graham
 * Description:	Module responsible for scheduling background tasks
 */ 

#include "schedular.h"

#include <string.h>

#define MAXIMUM_TASKS		50

// info related to tasks which are to be run
typedef struct  
{
	void (*task_function_ptr)(void);
	TASK_TRIGGER_SOURCE triggered_source;		
}RUN_TASK_INFO;

// list of all tasks and index to add next task 
static void (*task_lists[MAXIMUM_TASKS])(void);
static unsigned char task_list_input_index; 

// tasks to run array and associated tracking variables
static RUN_TASK_INFO tasks_to_run[MAXIMUM_TASKS];
static unsigned char tasks_to_run_input_index;
static unsigned char tasks_to_run_output_index;
static unsigned char tasks_to_run_count;

// name:	SCH_Init
// Desc:	Module initialisation function sets up the background task schedular.
void SCH_Init(void)
{
	// reset variables
	task_list_input_index = 0;
	//
	memset((void*)&task_lists[0], 0, (sizeof(task_lists[0]) * MAXIMUM_TASKS));
	//
	tasks_to_run_input_index = 0;
	tasks_to_run_output_index = 0;
	tasks_to_run_count = 0;
	//
	memset((void*)&tasks_to_run[0], 0, (sizeof(tasks_to_run[0]) * MAXIMUM_TASKS));
}

// name:	SCH_Add_task_to_list
// Desc:	adds a task to the list and returns a reference to that task.
unsigned char SCH_Add_task_to_list(void(*task_function_ptr)(void))
{
	unsigned char task_added_index = NO_TASK; // default to NO_TASK incase array is full
	
	// only add the task if the input index is less than maximum
	if(MAXIMUM_TASKS > task_list_input_index)
	{
		// set the task index to return and then increment index to the next position
		task_added_index = task_list_input_index++;
		//
		// add the task function to the list
		task_lists[task_added_index] = task_function_ptr;
	}
	else
	{
		// TBD add error 
	}
	
	return task_added_index;
}

// name:	SCH_Signal_task
// Desc:	adds the task at the index to the run task list with the source.
void SCH_Signal_task(unsigned char task_index, TASK_TRIGGER_SOURCE source)
{
	// add the task to the run array if the index is not NO_TASK
	if(NO_TASK != task_index)
	{
		tasks_to_run[tasks_to_run_input_index].task_function_ptr = task_lists[task_index];
		tasks_to_run[tasks_to_run_input_index].triggered_source = source;
		//
		if(MAXIMUM_TASKS == ++tasks_to_run_input_index)
		{
			tasks_to_run_input_index = 0;
		}
		//
		tasks_to_run_count++;	
	}
}

// name:	SCH_Run_background_tasks
// Desc:	runs one background task if there are any to run.
void SCH_Run_background_tasks(void)
{
	// if there are tasks to run
	if(0 != tasks_to_run_count)
	{
		// run the next task 
		tasks_to_run[tasks_to_run_output_index].task_function_ptr();
		//
		// reset the element in the run array 
		tasks_to_run[tasks_to_run_output_index].task_function_ptr = NULL;
		tasks_to_run[tasks_to_run_output_index].triggered_source = NOT_TRIGGERED;
		//
		// move on to the next task index
		if(MAXIMUM_TASKS == ++tasks_to_run_output_index)
		{
			tasks_to_run_output_index = 0;
		}
		//
		// this task has run so decrement the tasks to run count
		tasks_to_run_count--;
	}
}

// name:	SCH_Get_task_trigger_source
// Desc:	returns the trigger source of the running task.
TASK_TRIGGER_SOURCE SCH_Get_task_trigger_source(void)
{
	// return the source that signalled the task to run
	return tasks_to_run[tasks_to_run_output_index].triggered_source;
}
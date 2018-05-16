/*
 * adc.c
 *
 * Created:		13/05/2018 19:55:42
 * Author:		Graham
 * Description:	Module responsible for setting up and retrieving data from the ADC
 */ 

#include "adc.h"

#include <avr/interrupt.h>

#define INTERNAL_2_56V_REFERENCE	0x0C0

// name:	ADC_Init
// Desc:	Module initialisation function.
void ADC_Init(void)
{
	ADMUX = INTERNAL_2_56V_REFERENCE;
}
/*
 * timer.c
 *
 * Created: 2024/5/25 15:10:07
 * Author: SZZ0030
 */ 

#include "avr/io.h"
#include "avr/interrupt.h"
#include "timer.h"
#include "USBtoSerial.h"
//config timer1 overflow
void Timer_Init(void)
{
	TCCR1B |= ((0x01<<CS11));	//Prescaler 64
	TIMSK1 |= 0x01 << TOIE1 ;// enable overflow interrupt
	
}

ISR(TIMER1_OVF_vect)
{	
	Timer1_Handler();
}
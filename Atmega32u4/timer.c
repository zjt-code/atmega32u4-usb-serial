/*
 * timer.c
 *
 * Created: 2024/5/25 15:10:07
 *  Author: SZZ0030
 */ 

#include "avr/io.h"
#include "avr/interrupt.h"
#include "timer.h"
#include "USBtoSerial.h"
//config timer1 overflow
void Timer_Init(void)
{
	TCCR1B |= 0X01<<CS12; // Prescaler 256
	TIMSK1 |= 0x01 << TOIE1 ;// enable overflow interrupt
	
}

ISR(TIMER1_OVF_vect)
{
	
	if(PINB & (1<<PINB0))
	{
		PORTB&=~(0x01<<PORTB0);
	}else
	{
		PORTB|=(0x01<<PORTB0);
	}
	
	Timer1_Handler();
}
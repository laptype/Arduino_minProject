#ifndef F_CPU
#define F_CPU 16000000
#endif //F_CPU

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "HCSR04.h"

static volatile double first_reading = 0;
static volatile double second_reading = 0;
static volatile double duty_cycle = 0;

void HCSR04_Init(){
	cli(); //clear prior interrupts
	DDRD |= (1<<PIND7); //set PD7 as trigger output
	//Timer 1 running in normal mode
	DDRB &= ~(1<<DDB0); //PB0 as input (ICP1)
	TCCR1A = 0x00;
	TCCR1B = (1<<ICNC1)|(1<<ICES1)|(1<<CS11); //noise canceling + positive edge detection for input capture and Prescaler = 8.
	sei();		//enable global interrupts
	TIMSK1 |= (1<<ICIE1);	//enable timer1 input capture interrupt
}
/************************************************************************
Function: getDistance
Purpose:  send a 10us pulse to trigger HCSR04, and receive a pulse (echo)
Returns:  none
************************************************************************/
double getDistance(){	
	// PD7 as trigger output
	PORTD &= (~(1 << PIND7));
	_delay_us(2);
	PORTD |= (1 << PIND7);
	_delay_us(10);
	PORTD &= (~(1 << PIND7));
	
	static double echo_pulse_uS;
	static double distance_cm;
    //32768uS = 65536 clock ticks for Timer 1 with prescaler = 8
	echo_pulse_uS	= (double) duty_cycle * 32768 / 65536;
	distance_cm		= (double) echo_pulse_uS * 0.034 / 2;
	return distance_cm;
}

ISR(TIMER1_CAPT_vect){
	if ((TCCR1B & (1<<ICES1)) == (1<<ICES1)){
		first_reading = ICR1;
	}
	else{
		second_reading = ICR1;
	}
	if (first_reading != 0 && second_reading != 0){
		duty_cycle = second_reading - first_reading;
		first_reading = 0;
		second_reading = 0;
	}
	TCCR1B ^= (1<<ICES1);	//toggle edge detection bit
	TIFR1 = (1<<ICF1);		//clear Input Capture Flag
}



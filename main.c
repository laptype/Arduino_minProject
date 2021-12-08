/*
 * main.c
 *
 * Created: 2021/11/30 14:03:12
 *  Author: Bo Lan
 */ 
#define F_CPU 16000000
#define D4 eS_PORTB2
#define D5 eS_PORTB3
#define D6 eS_PORTB4
#define D7 eS_PORTB5
#define RS eS_PORTD3
#define EN eS_PORTD4

#define LED1 PINC2
#define LED2 PINC5
#define LED3 PINC4
#define LED4 PINC3


#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdlib.h>
#include <util/delay.h>
#include <stdio.h>

#include "HCSR04.h"
#include "UART.h"
#include "lcd.h"

int state = 1;
int button = 0;
char str_dis[10];
char str_temp[10];

void initADC(void) {
	ADMUX   |=  (1 << REFS0) | (1 << MUX0);  //reference voltage on AV_CC, and use ADC1
	ADCSRA  |=  (1 << ADPS1) | (1 << ADPS0); //ADC clock prescaler / 8
	ADCSRA  |=  (1 << ADEN);                 //enables the ADC
}

/************************************************************************
Function: led_dis()
Purpose:  display LEDs according to the distance 
Returns:  none
************************************************************************/
void led_dis(double distance) {
	if (distance>2 && distance<6){
		PORTC |= (1<<LED1)|(1<<LED2)|(1<<LED3)|(1<<LED4);
		} else if (distance>6 && distance < 10){
		PORTC |= (1<<LED2)|(1<<LED3)|(1<<LED4);
		PORTC &= ~(1<<LED1);
		} else if (distance>10 && distance < 14){
		PORTC |= (1<<LED3)|(1<<LED4);
		PORTC &= ~((1<<LED1)|(1<<LED2));
		} else if (distance>14 && distance < 18){
		PORTC |= (1<<LED4);
		PORTC &= ~((1<<LED1)|(1<<LED2)|(1<<LED3));
		} else {
		PORTC &= ~((1<<LED1)|(1<<LED2)|(1<<LED3)|(1<<LED4));
	}	
}
/************************************************************************
Function: lcd4_dis()
Purpose:  display LCD (mode 0 for distance; mode 1 for temperature)
Returns:  none
************************************************************************/
void lcd4_dis(char * value, int mode) {
	
	Lcd4_Clear();
	Lcd4_Set_Cursor(1,0);
	if (mode == 0) {
		Lcd4_Write_String("distance:");		
	} else {
		Lcd4_Write_String("temperature:");
	}
	Lcd4_Set_Cursor(2,0);
	Lcd4_Write_String(value);
	Lcd4_Write_Char(' ');
	if (mode == 0) {
		Lcd4_Write_Char('c');
		Lcd4_Write_Char('m');	
	} else {
		Lcd4_Write_Char((char)223);
		Lcd4_Write_Char('C');		
	}
}
/************************************************************************
Function: distance_dis()
Purpose:  get distance, and Remove outliers, average the distance, display on the LCD
Returns:  none
************************************************************************/
void distance_dis(void) {

	double distance[6];
	double sum = 0;
	double avg = 0;
	int count = 0;
	while (count == 0) {
		sum = 0;
		avg = 0;
		count = 0;
		for(int i=0; i<6; i++) {
			distance[i] = getDistance();
			sum += distance[i];
			_delay_ms(30);
		}
		avg = sum/6;	// average
		sum = 0;
		for(int i=0; i<8; i++) {
			// Remove outliers
			if (abs(distance[i]-avg) <= (avg/2)) {
				sum += distance[i];
				count++;
			}
		}
	}
	avg = sum/count;

	_delay_ms(100);
	// display LEDs
	led_dis(avg);	
	// double to string	
	dtostrf(avg, 2, 2, str_dis);	
	// display LCD
	lcd4_dis(str_dis,0); 
}
/************************************************************************
Function: temp_dis()
Purpose:  get Analog signal from LM35, Analog to digital, calculate temperature
Returns:  none
************************************************************************/
void temp_dis(void) {

	double value = 0;
    uint16_t temp_value;

	ADCSRA |= (1 << ADSC);			//start ADC conversion
    while((ADCSRA & (1 << ADSC)))	//wait until ADSC bit is clear, i.e., ADC conversion is done
    {}
    //read ADC value in
	uint8_t theLowADC  =  ADCL;
	temp_value =  ADCH << 8 | theLowADC; // 0 - 1023
	// temperature calculation
	value = (temp_value)*0.0048828125*85;
	
	dtostrf(value, 2, 2, str_temp);
	// turn off LEDs
	led_dis(100);
	// display LCD
	lcd4_dis(str_temp,1);
	_delay_ms(500);
}
/************************************************************************
Function: display()
Purpose:  state 0 for distance, state 1 for temperature
Returns:  none
************************************************************************/
void display(void) {
	switch (state) {
		case 0:{
			HCSR04_Init();
			distance_dis();
			break;
		}
		case 1:{
			temp_dis();
			break;
		}
	}	
}
/************************************************************************
Function: button interrupts
Purpose:  change state
Returns:  none
************************************************************************/
ISR (INT0_vect) {
	_delay_ms(20);
	// Preventing signal jitter
	button = PIND & (1<<PIND2);
	if (button == (1<<PIND2)) {
		state ++;
		if (state > 1) {
			state = 0;
		}
	}
}

int main(){
	DDRB |= ((1<<PINB5)|(1<<PINB4)|(1<<PINB3)|(1<<PINB2)|(1<<PINB1));
	DDRC |= (1<<LED1)|(1<<LED2)|(1<<LED3)|(1<<LED4);
	DDRC &= ~(1<<PINC1);
	DDRD |= ((1<<PIND4)|(1<<PIND3));
	DDRD &= ~(1<<PIND2);
	
	EIMSK = (1 << INT0); //enable external
	EICRA = 0x03;		//make INTO rising edge
	
	char rec;	// The character received
	
	initADC();	// enable ADC
	sei();		// enable interrupts

	USART_init();	// UART
	Lcd4_Init();	// LCD
	Lcd4_Clear();
	
	while (1){
		if (!(UCSR0A & (1<<RXC0))) {
			// no character received
			display();
		} else {
			rec = uart_getc();
			if (rec == 'Y') {
				state = 0;
				display();
				uart_outs("distance: ");
				uart_outs(str_dis);
				uart_outc('\n');				
			} else if (rec == 'N') {
				state = 1;
				display();
				uart_outs("temperature: ");
				uart_outs(str_temp);
				uart_outc('\n');
			}			

			if (rec == 'R') {
				TIMSK1	&=	~(1<<ICIE1); // disable timer1 input capture interrupt
				TCCR1A  |=  (1 << WGM11) | (1 << COM1A1);
				TCCR1B  |=  (1 << WGM13) | (1 << WGM12) |(1 << CS11);
				ICR1    =   40000;
				OCR1A   =   5000;
				_delay_ms(500);
			} else if (rec == 'L') {
				TIMSK1	&=	~(1<<ICIE1); // disable timer1 input capture interrupt
				TCCR1A  |=  (1 << WGM11) | (1 << COM1A1);
				TCCR1B  |=  (1 << WGM13) | (1 << WGM12) |(1 << CS11);
				ICR1    =   40000;
				OCR1A   =   1000;
				_delay_ms(500);							
			} else if (rec == 'U') {
				TIMSK1	&=	~(1<<ICIE1); // disable timer1 input capture interrupt
				TCCR1A  |=  (1 << WGM11) | (1 << COM1A1);
				TCCR1B  |=  (1 << WGM13) | (1 << WGM12) |(1 << CS11);
				ICR1    =   40000;
				OCR1A   =   2950;	
				_delay_ms(500);	
			}
		}
	}
}



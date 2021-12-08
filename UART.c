/*
 * UART.c
 *
 * Created: 2021/11/30 14:03:12
 *  Author: 12053
 */ 
#include <avr/io.h>

#ifndef F_CPU
#define F_CPU 16000000
#endif //F_CPU

#include "UART.h"

void USART_init(void){
	UBRR0H = (uint8_t)(BAUD_PRESCALLER>>8); //t baud rate
	UBRR0L = (uint8_t)(BAUD_PRESCALLER);
	UCSR0B = (1<<RXEN0)|(1<<TXEN0); //enable transmit
	UCSR0C = (3<<UCSZ00); //t 8-bit (default)
}
/************************************************************************
Function: uart_outc()
Purpose:  Transmitted byte to buffer
Returns:  none
************************************************************************/
void uart_outc(char ch) {
	/* Wait for empty transmit buffer */
	while(!(UCSR0A & (1<<UDRE0)))
	;
	/* Put data into buffer, sends the data */
	UDR0 = ch;
}
/************************************************************************
Function: uart_outs()
Purpose:  Transmitted String to buffer
Returns:  none
************************************************************************/
void uart_outs(char *calledstring) {
	for (int i = 0; i<255; i++) {
		if (calledstring[i] != 0) {
			/* send the char in the String */
			uart_outc(calledstring[i]);
			} else {
			/* until String end '\0' */
			break;
		}
	}
}
/************************************************************************
Function: uart_getc()
Purpose:  Received byte from buffer
Returns:  (char) received byte
************************************************************************/
char uart_getc(void) {
	/* Wait for data to be received */
	while(!(UCSR0A & (1<<RXC0)))
	;
	/* Get and return received data from buffer */
	return UDR0;
}
/************************************************************************
Function: uart_gets()
Purpose:  Received String from buffer
Returns:  none
************************************************************************/
void uart_gets(char *calledstring){
	char ch;
	int cntr = 0;
	while(1){
		ch = uart_getc();	// read a byte from buffer
		if(ch == 13 || ch == 10) {
			/* if the byte is '\r' or '\n' */
			calledstring[cntr] = 0;
			return;
			} else {
			/* load into a string */
			calledstring[cntr++] = ch;
		}
	}
}

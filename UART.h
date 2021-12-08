/*
 * UART.h
 *
 * Created: 2021/11/30 14:02:52
 *  Author: 12053
 */ 


#ifndef UART_H_
#define UART_H_

#ifndef BAUDRATE
#define BAUDRATE 9600
#define BAUD_PRESCALLER (((F_CPU / (BAUDRATE * 16UL))) - 1)

#endif /* BAUDRATE */

void USART_init(void);
void uart_outc(char);
void uart_outs(char *);
char uart_getc(void);
void uart_gets(char *);

#endif /* UART_H_ */


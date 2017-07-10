/*
 * uart.c
 *
 *  Created on: 14 июн. 2017 г.
 *      Author: alchemist
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include "uart.h"
#include "uart_fifo.h"

FIFO(128) uart_tx_fifo;
FIFO(256) uart_rx_fifo;



ISR(USART_RX_vect)
{
	unsigned char rxbyte = UDR0;
	if(!FIFO_IS_FULL(uart_rx_fifo))
	{
		FIFO_PUSH(uart_rx_fifo, rxbyte);
	}
}

ISR(USART_UDRE_vect)
{
	if(FIFO_IS_EMPTY(uart_tx_fifo))
	{
		UCSR0B  &= ~(1<<UDRIE0);
	}
	else
	{
		char txbyte = FIFO_FRONT(uart_tx_fifo);
		FIFO_POP(uart_tx_fifo);
		UDR0 = txbyte;
	}
}

int uart_putc(char c, FILE *file)
{
	int ret = -1;
	cli();
	if(!FIFO_IS_FULL(uart_tx_fifo))
	{
		FIFO_PUSH(uart_tx_fifo, c);
		UCSR0B |= (1<<UDRIE0);
		ret = 0;
	}

	sei();

	return ret;
}

int uart_getc(FILE *file)
{
	int ret = _FDEV_EOF;
	cli();
	if(!FIFO_IS_EMPTY(uart_rx_fifo))
	{
		ret = FIFO_FRONT(uart_rx_fifo);
		FIFO_POP(uart_rx_fifo);
	}

	sei();

	return ret;
}

FILE uart_stream = FDEV_SETUP_STREAM(uart_putc, uart_getc, _FDEV_SETUP_RW);

void uart_init()
{
	stdout = stdin = &uart_stream;

	UBRR0H = 0;
	UBRR0L = 8;
	UCSR0B = (1<<RXEN0)|(1<<TXEN0)|(1<<RXCIE0);
	UCSR0C = (1<<UCSZ00)|(1<<UCSZ01);

	sei();



}

void uart_flush()
{
	FIFO_FLUSH(uart_rx_fifo);
}

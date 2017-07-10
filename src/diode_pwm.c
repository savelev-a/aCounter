/*
 * diode_pwm.c
 *
 *  Created on: 14 июн. 2017 г.
 *      Author: Savelev A.
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include "diode_pwm.h"
#include "rtc_clock.h"
#include "data_io.h"

uint8_t active_diode = DIODE_LEFT_PIN;
uint8_t trigger = 0;


void diode_pwm_init()
{
	DDRC |= (1<<DIODE_LEFT_PIN)|(1<<DIODE_RIGHT_PIN);
	DDRB &= ~(1<<PB0);
	DDRB |= (1<<DIODE_INDICATOR_PIN);

	TCCR0A = 0;
	TCNT0 = 58;
	TIMSK0 = (1<<TOIE0);
	TCCR0B = (0<<CS02)|(0<<CS01)|(1<<CS00);

	sei();
}

ISR(TIMER0_OVF_vect)
{
	TCNT0 = 58;

	if(active_diode == DIODE_LEFT_PIN)
	{
		PORTC &= ~(1<<DIODE_RIGHT_PIN);
	}
	else
	{
		PORTC &= ~(1<<DIODE_LEFT_PIN);
	}

	PORTC ^= (1<<active_diode);
}

void diode_pwm_setleft()
{
	active_diode = DIODE_LEFT_PIN;
}

void diode_pwm_setright()
{
	active_diode = DIODE_RIGHT_PIN;
}


void diode_check_pass()
{
	uint8_t left_visible = 0;
	uint8_t right_visible = 0;

	diode_pwm_setleft();
	_delay_ms(5);
	if(DIODE_SENSE)
	{
		left_visible = 1;
	}
	_delay_ms(25);

	diode_pwm_setright();
	_delay_ms(5);
	if(DIODE_SENSE)
	{
		right_visible = 1;
	}
	_delay_ms(25);

	if(left_visible && right_visible)
	{
		PORTB |= (1<<DIODE_INDICATOR_PIN);
		trigger = 0;
	}
	else if(left_visible)
	{
		PORTB &= ~(1<<DIODE_INDICATOR_PIN);
		if(trigger)
		{
			pass_register(LEFT);

			trigger = 0;
		}
	}
	else if(right_visible)
	{
		PORTB &= ~(1<<DIODE_INDICATOR_PIN);
		if(trigger)
		{
			pass_register(RIGHT);

			trigger = 0;
		}
	}
	else
	{
		PORTB &= ~(1<<DIODE_INDICATOR_PIN);
		trigger = 1;
	}
}



/*
 * diode_pwm.h
 *
 *  Created on: 14 июн. 2017 г.
 *      Author: Savelev A.
 */

#ifndef DIODE_PWM_H_
#define DIODE_PWM_H_

#define DIODE_LEFT_PIN PC3
#define DIODE_RIGHT_PIN PC2
#define DIODE_INDICATOR_PIN PB1

#define DIODE_SENSE !(PINB & (1<<PB0))

void diode_pwm_init();
void diode_pwm_setleft();
void diode_pwm_setright();

void diode_check_pass();


#endif /* DIODE_PWM_H_ */

/*
 * rtc_clock.h
 *
 *  Created on: 15 июн. 2017 г.
 *      Author: alchemist
 */

#ifndef RTC_CLOCK_H_
#define RTC_CLOCK_H_

#include <stdint.h>

void rtc_clock_init();

void rtc_clock_getDate(char* date);
void rtc_clock_getTime(char* time);
void rtc_clock_getDateTime(char* datetime);

void rtc_clock_setDate(char* date);
void rtc_clock_setTime(char* time);

uint32_t get_fattime();

#endif /* RTC_CLOCK_H_ */

/*
 * rtc_clock.c
 *
 *  Created on: 15 июн. 2017 г.
 *      Author: alchemist
 */

#include <avr/io.h>
#include <stdio.h>
#include <string.h>

#include "rtc_clock.h"

#define TW_START 0xA4
#define TW_SEND 0x84
#define TW_STOP 0x94
#define TW_NACK 0x84

#define TW_READY (TWCR & 0x80)
#define TW_STATUS (TWSR & 0xF8)

#define DS1307_ADDR 0xD0

#define SECONDS_REGISTER   0x00
#define MINUTES_REGISTER   0x01
#define HOURS_REGISTER     0x02
#define DAYOFWK_REGISTER   0x03
#define DAYS_REGISTER      0x04
#define MONTHS_REGISTER    0x05
#define YEARS_REGISTER     0x06

#define I2C_Stop() TWCR = TW_STOP


void I2C_init()
{
	TWSR = 0;
	TWBR = 72;
}

uint8_t I2C_Start()
{
	TWCR = TW_START;
	while(!TW_READY);
	return (TW_STATUS == 0x08);
}

uint8_t I2C_SendAddr(uint8_t addr)
{
	TWDR = addr;
	TWCR = TW_SEND;
	while(!TW_READY);
	return (TW_STATUS == 0x18);
}

uint8_t I2C_SendData(uint8_t data)
{
	TWDR = data;
	TWCR = TW_SEND;
	while(!TW_READY);
	return (TW_STATUS == 0x28);
}

uint8_t I2C_ReadNACK()
{
	TWCR = TW_NACK;
	while(!TW_READY);
	return TWDR;
}



void I2C_WriteRegister(uint8_t deviceRegister, uint8_t data)
{
	I2C_Start();
	I2C_SendAddr(DS1307_ADDR);
	I2C_SendData(deviceRegister);
	I2C_SendData(data);
	I2C_Stop();
}

uint8_t I2C_ReadRegister(uint8_t busAddr, uint8_t deviceRegister)
{
	uint8_t data = 0;
	I2C_Start();
	I2C_SendAddr(busAddr);
	I2C_SendData(deviceRegister);
	I2C_Start();
	I2C_SendAddr(busAddr + 1);
	data = I2C_ReadNACK();
	I2C_Stop();
	return data;
}

uint8_t rtc_clock_getYear()
{
	return I2C_ReadRegister(DS1307_ADDR, YEARS_REGISTER);
}

uint8_t rtc_clock_getDay()
{
	return I2C_ReadRegister(DS1307_ADDR, DAYS_REGISTER);
}

uint8_t rtc_clock_getMonth()
{
	return I2C_ReadRegister(DS1307_ADDR, MONTHS_REGISTER);
}


uint8_t rtc_clock_getHour()
{
	uint8_t hours = I2C_ReadRegister(DS1307_ADDR, HOURS_REGISTER);
	hours &= 0x3F;
	return hours;
}

uint8_t rtc_clock_getMinute()
{
	return I2C_ReadRegister(DS1307_ADDR, MINUTES_REGISTER);
}

uint8_t rtc_clock_getSecond()
{
	return I2C_ReadRegister(DS1307_ADDR, SECONDS_REGISTER);
}

void two_digits(uint8_t data, char* result)
{
	uint8_t temp = data>>4;
	data &= 0x0F;
	sprintf(result, "%u%u", temp, data);
}

void rtc_clock_getDate(char* date)
{
	char years[3], months[3], days[3];

	two_digits(rtc_clock_getYear(), years);
	two_digits(rtc_clock_getMonth(), months);
	two_digits(rtc_clock_getDay(), days);

	sprintf(date, "%s.%s.%s", days, months, years);
}

void rtc_clock_getTime(char* time)
{
	char hours[3], minutes[3], seconds[3];

	two_digits(rtc_clock_getHour(), hours);
	two_digits(rtc_clock_getMinute(), minutes);
	two_digits(rtc_clock_getSecond(), seconds);

	sprintf(time, "%s:%s:%s", hours, minutes, seconds);
}

void rtc_clock_getDateTime(char* datetime)
{
	char date[9], time[9];

	rtc_clock_getDate(date);
	rtc_clock_getTime(time);

	sprintf(datetime, "%s %s", date, time);
}


void rtc_clock_setDate(char* date)
{
	if(strlen(date) != 8) return;

	uint8_t day = 0, month = 0, year = 0;
	day = (date[0] - '0') * 10 + (date[1] - '0');
	month = (date[3] - '0') * 10 + (date[4] - '0');
	year = (date[6] - '0') * 10 + (date[7] - '0');


	I2C_WriteRegister(DAYS_REGISTER, ((day/10)<<4) + day%10);
	I2C_WriteRegister(MONTHS_REGISTER, ((month/10)<<4) + month%10);
	I2C_WriteRegister(YEARS_REGISTER, ((year/10)<<4) + year%10);
}

void rtc_clock_setTime(char* time)
{
	if(strlen(time) != 8) return;

	uint8_t hour = 0, minute = 0, second = 0;
	hour = (time[0] - '0') * 10 + (time[1] - '0');
	minute = (time[3] - '0') * 10 + (time[4] - '0');
	second = (time[6] - '0') * 10 + (time[7] - '0');

	I2C_WriteRegister(HOURS_REGISTER, ((hour/10)<<4) + hour%10);
	I2C_WriteRegister(MINUTES_REGISTER, ((minute/10)<<4) + minute%10);
	I2C_WriteRegister(SECONDS_REGISTER, ((second/10)<<4) + second%10);
}

void rtc_clock_init()
{
	I2C_init();
}

uint32_t get_fattime()
{
	return 0;
}

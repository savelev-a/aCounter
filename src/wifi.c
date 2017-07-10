/*
 * wifi.c
 *
 *  Created on: 3 июл. 2017 г.
 *      Author: alchemist
 */

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include "wifi.h"
#include "rtc_clock.h"
#include <string.h>
#include "uart.h"
#include "fatfs/ff.h"


uint8_t wifi_send_command(char *command)
{
	uint8_t retry = 0;
	uint8_t at_ok = 0;

	_delay_ms(5);
	printf("%s\r\n", command);  //Send AT command
	retry = 0;
	do
	{
		char line[128];
		scanf("%120s\n", line);

		_delay_ms(5);

		retry++;
		if(retry > 32) return 1;

		if(strncmp(line, "OK", 2) == 0) at_ok = 1;

	} while(!at_ok);

	printf("%s\r\n", "--");

	return 0;
}

uint8_t wifi_init()
{
	_delay_ms(200);
	uint8_t retry = 0;
	uint8_t is_ready = 0;

	do
	{
		char line[128];
		scanf("%s\n", line);

		_delay_ms(5);

		retry++;
		if(retry > 20) return WIFI_NOTREADY_ERROR;
		if(strstr("ready", line) != NULL) is_ready = 1;
	} while(!is_ready);

	wifi_send_command("ATE0");

	if(wifi_send_command("AT")) return WIFI_INIT_ERROR;
	if(wifi_send_command("AT+CWMODE=1")) return WIFI_INIT_ERROR;
	if(wifi_send_command("AT+CWDHCP=1,0")) return WIFI_INIT_ERROR;
	if(wifi_send_command("AT+CIPSTA=\"192.168.1.230\"")) return WIFI_INIT_ERROR;
	if(wifi_send_command("AT+CIPMUX=1")) return WIFI_INIT_ERROR;
	if(wifi_send_command("AT+CIPSERVER=1")) return WIFI_INIT_ERROR;

	wifi_send_command("AT+CWJAP=\"ISCOMPANY\",\"BFmoFT5F\"");

	return 0;
}

void wifi_process()
{
	char line[256];
	scanf(" %120[^\n]", line);


	//if(strncmp(line, "WIFI CONN", 9) == 0) printf("%s\r\n", "Wifi conn ok!");
	if(strncmp(line, "+IPD,", 5) == 0)
	{
		uint8_t conn_num = line[5] - '0';

		if(strstr(line, "get_time") != NULL)
		{
			char datetime[20];
			rtc_clock_getDateTime(datetime);
			printf("AT+CIPSEND=%u,%u\r\n", conn_num, strlen(datetime) + 2);
			_delay_ms(5);
			printf("%s\r\n", datetime);
		}
		else if(strstr(line, "set_time") != NULL)
		{
			char *settimestr = strstr(line, "set_time");
			if(!strlen(settimestr) < 26)
			{
				char *datetoset = settimestr + 9;
				settimestr[17] = 0x00;
				char *timetoset = datetoset + 9;
				timetoset[8] = 0x00;

				rtc_clock_setDate(datetoset);
				rtc_clock_setTime(timetoset);

				printf("AT+CIPSEND=%u,%u\r\n", conn_num, 2);
				_delay_ms(5);
				printf("%s\r\n", "OK");
			}
		}
		else if(strstr(line, "clear_data") != NULL)
		{
			FIL outputFile;
			FRESULT fr;

			fr = f_open(&outputFile, "pass.dat", FA_READ|FA_WRITE);

			if(!fr)
			{
				f_truncate(&outputFile);
				printf("AT+CIPSEND=%u,%u\r\n", conn_num, 2);
				_delay_ms(5);
				printf("%s\r\n", "OK");
			}

			f_close(&outputFile);
		}
		else if(strstr(line, "get_data") != NULL) //responce with pass.dat
		{


			FIL outputFile;
			FRESULT fr;

			fr = f_open(&outputFile, "pass.dat", FA_READ);

			if(!fr)
			{
				char buff[65];
				uint16_t bytes_read;

				printf("AT+CIPSEND=%u,%u\r\n", conn_num, f_size(&outputFile));
				_delay_ms(5);
				do
				{
					f_read(&outputFile, buff, 64, &bytes_read);
					_delay_ms(5);
					buff[bytes_read] = 0x00;
					fputs(buff, stdout);
				} while (bytes_read == 64);

				printf("\r\n");
			}


			f_close(&outputFile);
		}
	}
}

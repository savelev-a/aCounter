/*
 * main.c
 *
 *  Created on: 14 июн. 2017 г.
 *      Author: Savelev A.
 */

#include <avr/io.h>
#include <stdio.h>
#include <avr/pgmspace.h>
#include <stdlib.h>
#include "diode_pwm.h"
#include "uart.h"
#include "rtc_clock.h"
#include "sdhccard.h"
#include "fatfs/ff.h"
#include "wifi.h"

int main()
{
	diode_pwm_init();
	uart_init();
	rtc_clock_init();
	uint8_t sdcard_status = sd_init();

	FATFS fatfs;
	f_mount(&fatfs, "", 0);

	uint8_t wifi_status = wifi_init();

	uint8_t wifi_c = 0;


	while(1)
	{
		diode_check_pass();

		wifi_c++;
		if(wifi_c >= 4)
		{
			wifi_c = 0;
			wifi_process();
		}

	}
}

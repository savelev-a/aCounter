/*
 * data_io.c
 *
 *  Created on: 22 июн. 2017 г.
 *      Author: alchemist
 */

#include <avr/io.h>
#include <stdio.h>
#include "data_io.h"
#include "sdhccard.h"
#include "rtc_clock.h"
#include "fatfs/ff.h"

void pass_register(PASS_DIRECTION direction)
{
	char date[15];
	char strToWrite[32];
	char strBuffer[32];
	rtc_clock_getDate(date);

	FIL outputFile;
	FRESULT fr;

	uint16_t lefts = 1 - direction;
	uint16_t rights = direction;
	uint8_t newDayAppend = 0;

	fr = f_open(&outputFile, "pass.dat", FA_READ|FA_WRITE|FA_OPEN_APPEND);
	if((fr != 0x00) && (fr != 0x08))
	{
		return;
	}

	FSIZE_t pos = f_tell(&outputFile);
	if(pos != 0x00)  //file exists and not empty
	{
		f_lseek(&outputFile, 0);

		while(!f_eof(&outputFile))
		{
			pos = f_tell(&outputFile);
			f_gets(strBuffer, 32, &outputFile);
		}

		for(uint8_t i = 0; i < 8; i++)
		{
			if(strBuffer[i] != date[i])
			{
				newDayAppend = 1;
				break;
			}
		}

		if(!newDayAppend)  //rewrite old day
		{
			sscanf(strBuffer, "%*s%u%u", &lefts, &rights);
			lefts += (1 - direction);
			rights += direction;
		}
		else			//append new day
		{
			pos = f_size(&outputFile);
		}
	}

	sprintf(strToWrite, "%s %u %u", date, lefts, rights);

	f_lseek(&outputFile, pos);
	f_printf(&outputFile,"%s\n", strToWrite);


	f_close(&outputFile);
}

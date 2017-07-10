/*
 * sdhccard.c
 *
 *  Created on: 19 июн. 2017 г.
 *      Author: alchemist
 */

#include <avr/io.h>
#include <stdio.h>
#include "sdhccard.h"

#define MISO PB4
#define MOSI PB3
#define SCK PB5
#define CS PB2

#define GO_IDLE_STATE            0
#define SEND_OP_COND             1
#define SEND_IF_COND			 8
#define SEND_CSD                 9
#define STOP_TRANSMISSION        12
#define SEND_STATUS              13
#define SET_BLOCK_LEN            16
#define READ_SINGLE_BLOCK        17
#define READ_MULTIPLE_BLOCKS     18
#define WRITE_SINGLE_BLOCK       24
#define WRITE_MULTIPLE_BLOCKS    25
#define ERASE_BLOCK_START_ADDR   32
#define ERASE_BLOCK_END_ADDR     33
#define ERASE_SELECTED_BLOCKS    38
#define SD_SEND_OP_COND			 41   //ACMD
#define APP_CMD					 55
#define READ_OCR				 58
#define CRC_ON_OFF               59

#define SD_CS_ASSERT     PORTB &= ~(1<<CS);
#define SD_CS_DEASSERT   PORTB |= (1<<CS);

uint8_t spi_transmit(uint8_t data)
{
	SPDR = data;

	while(!(SPSR & (1<<SPIF)));
	data = SPDR;

	return data;
}

uint8_t spi_recieve()
{
	uint8_t data;

	SPDR = 0xff;
	while(!(SPSR & (1<<SPIF)));
	data = SPDR;

	return data;
}


uint8_t sd_send_command(uint8_t cmd, uint32_t arg)
{
	uint8_t response, retry = 0, status;

	if(SDHC_flag == 0)
		if( cmd == READ_SINGLE_BLOCK      ||
			cmd == READ_MULTIPLE_BLOCKS   ||
			cmd == WRITE_SINGLE_BLOCK     ||
			cmd == WRITE_MULTIPLE_BLOCKS  ||
			cmd == ERASE_BLOCK_START_ADDR ||
			cmd == ERASE_BLOCK_END_ADDR)
		{
			arg = arg << 9;
		}

	SD_CS_ASSERT;

	spi_transmit(cmd | 0x40);
	spi_transmit(arg>>24);
	spi_transmit(arg>>16);
	spi_transmit(arg>>8);
	spi_transmit(arg);
	if(cmd == SEND_IF_COND)
		spi_transmit(0x87);
	else
		spi_transmit(0x95);

	while((response = spi_recieve()) == 0xff)
		if(retry++ > 0xfe) break;

	if(response == 0x00 && cmd == 58)
	{
		status = spi_recieve() & 0x40;
		if(status == 0x40) SDHC_flag = 1;
		else SDHC_flag = 0;

		spi_recieve();
		spi_recieve();
		spi_recieve();
	}

	if(response == 0x01 && cmd == SEND_IF_COND)
	{
		spi_recieve();
		spi_recieve();
		spi_recieve();
		spi_recieve();
		spi_recieve();
	}

	spi_recieve();

	SD_CS_DEASSERT;

	return response;
}


uint8_t sd_init()
{
	sd_initialized = 0;

	DDRB |= (1<<SCK)|(1<<CS)|(1<<MOSI)|(0<<MISO);
	SPCR = 0x53;
	SPSR = 0x00;

	uint8_t response, SD_version;
	uint8_t retry = 0 ;

	for(uint8_t i = 0; i < 10; i++)
		spi_transmit(0xff);

	SD_CS_ASSERT;

	do
	{
		response = sd_send_command(GO_IDLE_STATE, 0);
		//printf("CMD0 -> %u\r\n", response);
		retry++;
		if(retry > 0x20) return 1;
	} while (response != 0x01);

	SD_CS_DEASSERT;
	spi_transmit(0xff);
	spi_transmit(0xff);

	retry = 0;
	SD_version = 2;

	do
	{
		response = sd_send_command(SEND_IF_COND, 0x000001AA);
		//printf("CMD8 -> %u\r\n", response);
		retry++;
		if(retry > 0xfe)
		{
			SD_version = 1;
			cardType = 1;
			break;
		}
	} while (response != 0x01);

	retry = 0;

	do
	{
		response = sd_send_command(APP_CMD, 0);
		//printf("CMD55 -> %u\r\n", response);
		response = sd_send_command(SD_SEND_OP_COND, 0x40000000);
		//printf("ACMD41 -> %u\r\n", response);

		retry++;
		if(retry > 0xfe)
		{
			return 2;
		}
	} while (response != 0x00);

	retry = 0;
	SDHC_flag = 0;

	if(SD_version == 2)
	{
		do
		{
			response = sd_send_command(READ_OCR, 0);
			retry++;
			if(retry > 0xfe)
			{
				cardType = 0;
				break;
			}
		} while (response != 0x00);

		if(SDHC_flag == 1) cardType = 2;
		else cardType = 3;
	}

	sd_initialized = 1;

	return 0;
}

uint8_t sd_read_block(uint8_t *buffer, uint32_t startBlock)
{
	uint8_t response;
	uint16_t i, retry = 0;

	response = sd_send_command(READ_SINGLE_BLOCK, startBlock);

	if(response != 0x00) return response;

	SD_CS_ASSERT;

	retry = 0;
	while(spi_recieve() != 0xfe)
	{
		if(retry++ > 0xfffe)
		{
			SD_CS_DEASSERT;
			return 1;
		}
	}

	for(i = 0; i < 512; i++)
	{
		buffer[i] = spi_recieve();
	}

	spi_recieve();
	spi_recieve();
	spi_recieve();

	SD_CS_DEASSERT;

	return 0;
}

uint8_t sd_write_block(uint8_t *buffer, uint32_t startBlock)
{
	uint8_t response;
	uint16_t i, retry = 0;

	response = sd_send_command(WRITE_SINGLE_BLOCK, startBlock);

	if(response != 0x00) return response;

	SD_CS_ASSERT;

	spi_transmit(0xfe);

	for(i = 0; i < 512; i++)
		spi_transmit(buffer[i]);

	spi_transmit(0xff);
	spi_transmit(0xff);

	response = spi_recieve();

	if((response & 0x1f) != 0x05)
	{
		SD_CS_DEASSERT;
		return response;
	}

	while(!spi_recieve())
	{
		if(retry++ > 0xfffe)
		{
			SD_CS_DEASSERT;
			return 1;
		}
	}

	SD_CS_DEASSERT;
	spi_transmit(0xff);
	SD_CS_ASSERT;

	while(!spi_recieve())
	{
		if(retry++ > 0xfffe)
		{
			SD_CS_DEASSERT;
			return 1;
		}
	}

	SD_CS_DEASSERT;

	return 0;
}

uint8_t sd_read_multiple(uint8_t *buffer, uint32_t startBlock, uint16_t totalBlocks)
{
	uint8_t response, idx = 0;
	uint16_t i, retry = 0;

	response = sd_send_command(READ_MULTIPLE_BLOCKS, startBlock);

	if(response != 0x00) return response;

	SD_CS_ASSERT;

	while(totalBlocks)
	{
		retry = 0;
		while(spi_recieve() != 0xfe)
		{
			if(retry++ > 0xfffe)
			{
				SD_CS_DEASSERT;
				return 1;
			}
		}

		for(i = 0; i < 512; i++)
		{
			buffer[i + idx*512] = spi_recieve();
		}

		spi_recieve();
		spi_recieve();
		spi_recieve();

		totalBlocks--;
		idx++;
	}

	sd_send_command(STOP_TRANSMISSION, 0);
	SD_CS_DEASSERT;
	spi_recieve();

	return 0;
}

uint8_t sd_write_multiple(const uint8_t *buffer, uint32_t startBlock, uint16_t totalBlocks)
{
	uint8_t response;
	uint16_t i, retry = 0;
	uint32_t idx = 0;

	response = sd_send_command(WRITE_MULTIPLE_BLOCKS, startBlock);

	if(response != 0x00) return response;

	SD_CS_ASSERT;

	while(totalBlocks)
	{
		spi_transmit(0xfc);

		for(i = 0; i < 512; i++)
		{
			spi_transmit(buffer[i + idx*512]);
		}

		spi_transmit(0xff);
		spi_transmit(0xff);

		response = spi_recieve();
		if((response & 0x1f) != 0x05)
		{
			SD_CS_DEASSERT;
			return response;
		}

		while(!spi_recieve())
		{
			if(retry++ > 0xfffe)
			{
				SD_CS_DEASSERT;
				return 1;
			}
		}
		spi_recieve();

		totalBlocks--;
		idx++;
	}

	spi_transmit(0xfd);

	retry = 0;
	while(!spi_recieve())
	{
		if(retry++ > 0xfffe)
		{
			SD_CS_DEASSERT;
			return 1;
		}
	}

	SD_CS_DEASSERT;
	spi_transmit(0xff);
	SD_CS_ASSERT;

	retry = 0;
	while(!spi_recieve())
	{
		if(retry++ > 0xfffe)
		{
			SD_CS_DEASSERT;
			return 1;
		}
	}

	SD_CS_DEASSERT;

	return 0;

}

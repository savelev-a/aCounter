/*
 * sdhccard.h
 *
 *  Created on: 19 июн. 2017 г.
 *      Author: alchemist
 */

#ifndef SDHCCARD_H_
#define SDHCCARD_H_

#include <stdint.h>

volatile uint32_t startBlock, totalBlocks;
volatile uint8_t SDHC_flag, cardType;
volatile uint8_t sd_initialized;

uint8_t sd_init();

uint8_t sd_read_block(uint8_t *buffer, uint32_t startBlock);
uint8_t sd_write_block(uint8_t *buffer, uint32_t startBlock);
uint8_t sd_read_multiple(uint8_t *buffer, uint32_t startBlock, uint16_t totalBlocks);
uint8_t sd_write_multiple(const uint8_t *buffer, uint32_t startBlock, uint16_t totalBlocks);

#endif /* SDHCCARD_H_ */

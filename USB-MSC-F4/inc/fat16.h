/*
 * fat16.h
 *
 *  Created on: 12. 11. 2019
 *  Author:     Priesol Vladimir
 */

#ifndef FAT16_H_
#define FAT16_H_

#include "stm32f4xx.h"
#include <stdbool.h>

void FAT16_Init(void);

void FAT16_CreateBlock(uint8_t *buf, uint32_t blk_addr, uint16_t blk_len);
void FAT16_CreateBlockFAT(uint8_t *buf, uint16_t nBlockOffset);
void FAT16_CreateBlockDIR(uint8_t *buf, uint16_t nBlockOffset);
void FAT16_CreateBlockDATA(uint8_t *buf, uint16_t nBlockOffset);

uint32_t FAT16_GetStorageSize(void);

#endif /* FAT16_H_ */

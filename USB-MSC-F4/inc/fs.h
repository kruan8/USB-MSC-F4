/*
 * fs.h
 *
 *  Created on: 1. 3. 2019
 *  Author:     Priesol Vladimir
 */

#ifndef FS_H_
#define FS_H_

#include "stm32f4xx.h"
#include "stdbool.h"

#define FS_FILENAME_LEN     50


bool FS_Init(void);

bool FS_NewDataFile(const char* strSurvey);
int32_t FS_WriteData(const char* strCave, uint8_t* pBuffer, uint16_t nLen);
int32_t FS_ReadData(const char* strCave, uint8_t* pBuffer, uint16_t nLen, uint32_t nOffset);

int32_t FS_GetNextFilename(const char* strPath, char* strFilename, int32_t nPos);
bool FS_DeleteDataFile(char* strFilename);
bool FS_SaveData(const char* strFilename, uint8_t* pBuffer, uint16_t nLen);

int32_t FS_WriteSettings(uint8_t* pBuffer, uint16_t nLen);
int32_t FS_ReadSettings(uint8_t* pBuffer, uint16_t nLen);

int FS_Format(void);
int32_t FS_GetFreeSize(int32_t* pnSize, int32_t* pnFreeSize);

#endif /* FS_H_ */

/*
 * FlashG25D10B.c
 *
 *  Created on: 7. 11. 2016
 *      Author: priesolv
 */


#include "flashG25.h"
#include "spi.h"
#include "common.h"


#define CS_ENABLE           (GET_PORT(FLASH25_CS)->BSRRH = GET_PIN(FLASH25_CS))
#define CS_DISABLE          (GET_PORT(FLASH25_CS)->BSRRL = GET_PIN(FLASH25_CS))

#define DUMMY_BYTE          0xFF

#define G25_COMID_WRITE_ENABLE      0x06
#define G25_COMID_WRITE_DISABLE     0x04
#define G25_COMID_READ_SR           0x05
#define G25_COMID_WRITE_SR          0x01
#define G25_COMID_READ_DATA         0x03
#define G25_COMID_PAGE_PROGRAM      0x02
#define G25_COMID_SECTOR_ERASE      0x20
#define G25_COMID_DEVICE_ID         0x90
#define G25_COMID_IDENTIFICATION    0x9F
#define G25_COMID_DEEP_POWER        0xB9

#define G25_STATUS_WIP              0x01     // write in progress
#define G25_STATUS_WEL              0x02
#define G25_STATUS_SRP              0x80

#define GIGADEVICE_ID               0xC8
#define ADESTO_ID                   0x1F


static const FlashG25Identify_t g_G25types[] = {
    // ID      | pages | sectors
    { 0xC84011,    512,    32, "G25D10/1Mbit" },        // G25D10 1Mbit
    { 0xC84013,   2048,   128, "G25Q41/4Mbit" },        // G25Q41 4Mbit
    { 0x1F8401,   2048,   128, "AT25SF041/4Mbit" },     // AT25SF041 4Mbit
    { 0xEF4015,   8192,   512, "25Q16/16Mbit" },        // WINBOND 25Q16 16Mbit
    { 0xC84017,  32768,  2048, "G25Q64/64Mbit" },       // GD25Q64 64Mbit
};

static uint32_t g_nPages = 0;           // memory page count
static uint32_t g_nSectors = 0;         // memory sector count
static const char* g_pTypeString;       // memory description string

#define FLASH25_CS                  PB14  // taobao

static spi_drv_t *g_pDrv;

bool FlashG25_Init(void)
{
  g_pDrv = spi1;
  spi_Init(g_pDrv, PB3, PB5, PB4);
  g_pDrv->ePrescaler = spi_br_4;

  // set CS for output
  GPIO_InitTypeDef GPIO_InitStructure;

  GPIO_ClockEnable(GPIO_GetPort(FLASH25_CS));
  GPIO_InitStructure.GPIO_Pin = GPIO_GetPin(FLASH25_CS);
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_Init(GPIO_GetPort(FLASH25_CS), &GPIO_InitStructure);

  CS_DISABLE;

  if (!FlashG25_IsPresent())
  {
    return false;
  }

  if (g_nSectors == 0)
  {
    return false;
  }

  return true;
}

FlashG25Status_t FlashG25_GetStatus()
{
  CS_ENABLE;
  spi_SendData8(g_pDrv, G25_COMID_READ_SR);
  uint8_t sr = spi_SendData8(g_pDrv, DUMMY_BYTE);
  CS_DISABLE;

  FlashG25Status_t *pStatus = (FlashG25Status_t *) &sr;
  return *pStatus;
}

void FlashG25_ReadData(uint32_t nAddr, uint8_t* pBuffer, uint32_t length)
{
  CS_ENABLE;
  spi_SendData8(g_pDrv, G25_COMID_READ_DATA);
  FlashG25_Send24bit(nAddr);
  while (length--)
  {
    *pBuffer++ = spi_SendData8(g_pDrv, DUMMY_BYTE);
  }

  CS_DISABLE;
}

void FlashG25_WriteData(uint32_t nAddr, uint8_t* pBuffer, uint32_t length)
{
  uint16_t nBlockSize;
  uint16_t nPhysSize;
  while (length)
  {
    nBlockSize = length;
    nPhysSize = G25_PAGE_SIZE - (nAddr % G25_PAGE_SIZE);  // hranice fyzické stránky (PAGE SIZE)
    if (length > nPhysSize)
    {
      nBlockSize = nPhysSize;
    }

    FlashG25_WriteEnable();
    CS_ENABLE;
    spi_SendData8(g_pDrv, G25_COMID_PAGE_PROGRAM);
    FlashG25_Send24bit(nAddr);
    uint16_t nSize = nBlockSize;
    while (nSize--)
    {
      spi_SendData8(g_pDrv, *pBuffer++);
    }

    CS_DISABLE;

    // cekani na ukonceni programovaciho cyklu
    while (FlashG25_GetStatus().WIP);
    nAddr += nBlockSize;
    length -= nBlockSize;
  }
}

void FlashG25_SectorErase(uint32_t nSectorNumber)
{
  nSectorNumber *= G25_SECTOR_SIZE;
  FlashG25_WriteEnable();
  CS_ENABLE;
  spi_SendData8(g_pDrv, G25_COMID_SECTOR_ERASE);
  FlashG25_Send24bit(nSectorNumber);
  CS_DISABLE;

  while (FlashG25_GetStatus().WIP);
}

void FlashG25_WriteEnable()
{
  CS_ENABLE;
  spi_SendData8(g_pDrv, G25_COMID_WRITE_ENABLE);
  CS_DISABLE;
}

void FlashG25_WriteDisable()
{
  CS_ENABLE;
  spi_SendData8(g_pDrv, G25_COMID_WRITE_DISABLE);
  CS_DISABLE;
}

uint32_t FlashG25_GetID()
{
  CS_ENABLE;

  uint32_t nID = 0;
  spi_SendData8(g_pDrv, G25_COMID_IDENTIFICATION);
  nID = spi_SendData8(g_pDrv, DUMMY_BYTE) << 16;
  nID |= spi_SendData8(g_pDrv, DUMMY_BYTE) << 8;
  nID |= spi_SendData8(g_pDrv, DUMMY_BYTE);

  CS_DISABLE;

  return nID;
}

const char* FlashG25_GetTypeString()
{
  return g_pTypeString;
}

bool FlashG25_IsPresent()
{
  bool bFounded = false;
  uint32_t nId = FlashG25_GetID();

  uint8_t nTabSize = sizeof (g_G25types) / sizeof(FlashG25Identify_t);
  for (uint8_t i = 0; i < nTabSize; i++)
  {
    if (g_G25types[i].identificationID == nId)
    {
      g_nPages = g_G25types[i].pages;
      g_nSectors = g_G25types[i].sectors;
      g_pTypeString = g_G25types[i].strType;
      bFounded = true;
      break;
    }
  }

  return bFounded;
}

void FlashG25_SetDeepPower()
{
  CS_ENABLE;
  spi_SendData8(g_pDrv, G25_COMID_DEEP_POWER);
  CS_DISABLE;
  }

uint32_t FlashG25_GetSectors()
{
  return g_nSectors;
}

uint32_t FlashG25_GetPages()
{
  return g_nPages;
}

void FlashG25_Send24bit(uint32_t nValue)
{
  spi_SendData8(g_pDrv, nValue >> 16);
  spi_SendData8(g_pDrv, nValue >> 8);
  spi_SendData8(g_pDrv, nValue);
}

/*
 * SPI1.c
 *
 *  Created on: 13. 3. 2019
 *      Author: priesolv
 *
 *      implementace pro STM32F407
 */

#include "spi.h"

const spi_hw_t spi1_hw =
{
  .reg = SPI1,
  .nAPBclock = RCC_APB2Periph_SPI1,
  .nGpioAF = GPIO_AF_SPI1,
  .irq = SPI1_IRQn,
};

const spi_hw_t spi2_hw =
{
  .reg = SPI2,
  .nAPBclock = RCC_APB1Periph_SPI2,
  .nGpioAF = GPIO_AF_SPI2,
  .irq = SPI2_IRQn,
};

const spi_hw_t spi3_hw =
{
  .reg = SPI3,
  .nAPBclock = RCC_APB1Periph_SPI3,
  .nGpioAF = GPIO_AF_SPI3,
  .irq = SPI3_IRQn,
};

spi_drv_t _spi1_drv = { &spi1_hw, spi_mode_0, spi_br_256, spi_dir_mode_2Lines_FullDuplex, 0, 0, NULL, NULL, false, false, false };
spi_drv_t _spi2_drv = { &spi2_hw, spi_mode_0, spi_br_256, spi_dir_mode_2Lines_FullDuplex, 0, 0, NULL, NULL, false, false, false };
spi_drv_t _spi3_drv = { &spi3_hw, spi_mode_0, spi_br_256, spi_dir_mode_2Lines_FullDuplex, 0, 0, NULL, NULL, false, false, false };

void spi_Init(spi_drv_t* pDrv, gpio_pins_e eClkPin, gpio_pins_e eMosiPin, gpio_pins_e eMisoPin)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  GPIO_ClockEnable(GPIO_GetPort(eClkPin));
  GPIO_ClockEnable(GPIO_GetPort(eMosiPin));
  GPIO_ClockEnable(GPIO_GetPort(eMisoPin));

  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;

  if (pDrv->pHW->reg == SPI1)
  {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
  }

  if (pDrv->pHW->reg == SPI2)
  {
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
  }
  else if (pDrv->pHW->reg == SPI3)
  {
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);
  }

  /* SPI SCK pin configuration */
  GPIO_ClockEnable(GPIO_GetPort(eClkPin));
  GPIO_InitStructure.GPIO_Pin = GPIO_GetPin(eClkPin);
  GPIO_Init(GPIO_GetPort(eClkPin), &GPIO_InitStructure);
  GPIO_PinAFConfig(GPIO_GetPort(eClkPin), GPIO_GetPinSource(GPIO_GetPin(eClkPin)), pDrv->pHW->nGpioAF);

  /* SPI  MOSI pin configuration */
  GPIO_ClockEnable(GPIO_GetPort(eMosiPin));
  GPIO_InitStructure.GPIO_Pin = GPIO_GetPin(eMosiPin);
  GPIO_Init(GPIO_GetPort(eMosiPin), &GPIO_InitStructure);
  GPIO_PinAFConfig(GPIO_GetPort(eMosiPin), GPIO_GetPinSource(GPIO_GetPin(eMosiPin)), pDrv->pHW->nGpioAF);

  /* SPI  MISO pin configuration */
  if (eMisoPin != P_UNUSED)
  {
    GPIO_ClockEnable(GPIO_GetPort(eMisoPin));
    GPIO_InitStructure.GPIO_Pin = GPIO_GetPin(eMisoPin);
    GPIO_Init(GPIO_GetPort(eMisoPin), &GPIO_InitStructure);
    GPIO_PinAFConfig(GPIO_GetPort(eMisoPin), GPIO_GetPinSource(GPIO_GetPin(eMisoPin)), pDrv->pHW->nGpioAF);
  }

  /* SPI configuration -------------------------------------------------------*/
  SPI_I2S_DeInit(pDrv->pHW->reg);

  SPI_InitTypeDef  SPI_InitStructure;
  SPI_InitStructure.SPI_Direction = pDrv->eDirMode;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = pDrv->ePrescaler;  // SPI speed
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_Init(pDrv->pHW->reg, &SPI_InitStructure);

  spi_SetMode(pDrv, pDrv->eMode);
  SPI_Cmd(pDrv->pHW->reg, ENABLE);
}

uint16_t spi_SendData16(spi_drv_t* pDrv, uint16_t nValue)
{
  while (!(pDrv->pHW->reg->SR & SPI_I2S_FLAG_TXE));
  pDrv->pHW->reg->DR = nValue >> 8;
  while (!(pDrv->pHW->reg->SR & SPI_I2S_FLAG_TXE));
  pDrv->pHW->reg->DR = nValue;
  while (pDrv->pHW->reg->SR & SPI_I2S_FLAG_BSY);
  return pDrv->pHW->reg->DR;
}

uint8_t spi_SendData8(spi_drv_t* pDrv, uint8_t nValue)
{
  while (!(pDrv->pHW->reg->SR & SPI_I2S_FLAG_TXE));
  pDrv->pHW->reg->DR = nValue;

  while (!(pDrv->pHW->reg->SR & SPI_I2S_FLAG_RXNE));
  return pDrv->pHW->reg->DR;
}

void spi_WriteBidirectionalByte(spi_drv_t* pDrv, uint16_t nValue)
{
  while (!(pDrv->pHW->reg->SR & SPI_I2S_FLAG_TXE));
  pDrv->pHW->reg->DR = nValue;
  while (!(pDrv->pHW->reg->SR & SPI_I2S_FLAG_TXE));
  while (pDrv->pHW->reg->SR & SPI_I2S_FLAG_BSY);
}

uint8_t spi_ReadBidirectionalByte(spi_drv_t* pDrv)
{
  spi_SetDirection(pDrv, spi_dir_rx);
  while (!(pDrv->pHW->reg->SR & SPI_I2S_FLAG_RXNE));
  uint8_t nValue = pDrv->pHW->reg->DR;
  spi_SetDirection(pDrv, spi_dir_tx);
  return nValue;
}

void spi_SetPrescaler(spi_drv_t* pDrv, spi_br_e ePrescaler)
{
  // stop SPI peripherial
  pDrv->pHW->reg->CR1 &= (uint16_t)~((uint16_t)SPI_CR1_SPE);

  pDrv->pHW->reg->CR1 = (pDrv->pHW->reg->CR1 & ~(SPI_CR1_BR)) | ePrescaler;

  // start SPI peripherial
  pDrv->pHW->reg->CR1 |= SPI_CR1_SPE;
}

void spi_SetMode(spi_drv_t* pDrv, spi_mode_e eMode)
{
  // stop SPI peripherial
  pDrv->pHW->reg->CR1 &= (uint16_t)~((uint16_t)SPI_CR1_SPE);

  pDrv->pHW->reg->CR1 = (pDrv->pHW->reg->CR1 & ~(SPI_CR1_CPHA | SPI_CR1_CPOL)) | eMode;

  // start SPI peripherial
  pDrv->pHW->reg->CR1 |= SPI_CR1_SPE;
}

void spi_WaitForNoBusy(spi_drv_t* pDrv)
{
  while (pDrv->pHW->reg->SR & SPI_I2S_FLAG_BSY);
}

void spi_SetDirection(spi_drv_t* pDrv, spi_direction_e bDirection)
{
  SPI_BiDirectionalLineConfig(pDrv->pHW->reg, bDirection);
}

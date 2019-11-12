/*
 * common.c
 *
 *  Created on: 9. 8. 2016
 *      Author: priesolv
 */

#include "common.h"

/*******************************************************************************
* Function Name  : BTL_USART_GpioClock
* Description    : Enable GPIO clock
* Input          : - gpio: GPIO port
*          : - state: new clock state
* Return         : None
*******************************************************************************/
void GPIO_ClockEnable(GPIO_TypeDef* gpio)
{
  uint16_t nPort = ((uint32_t)gpio - (GPIOA_BASE)) / ((GPIOB_BASE) - (GPIOA_BASE));
  RCC->AHB1ENR |= (1 << nPort);
}

void GPIO_ClockDisable(GPIO_TypeDef* gpio)
{
  uint16_t nPort = ((uint32_t)gpio - (GPIOA_BASE)) / ((GPIOB_BASE) - (GPIOA_BASE));
  RCC->AHB1ENR &= ~(1 << nPort);
}

GPIO_TypeDef* GPIO_GetPort(gpio_pins_e ePortPin)
{
  GPIO_TypeDef* port;
  port = (GPIO_TypeDef*)(GPIOA_BASE + ((ePortPin >> 4) * ((GPIOB_BASE) - (GPIOA_BASE))));
  return port;
}

uint16_t GPIO_GetPin(gpio_pins_e ePortPin)
{
  return (1 << (ePortPin & 0x0F));
}

uint16_t GPIO_GetPinSource(uint16_t GPIO_Pin)
{
  uint16_t pinsource = 0;

  /* Get pinsource */
  while (GPIO_Pin > 1)
  {
    pinsource++;
    GPIO_Pin >>= 1;
  }

  /* Return source */
  return pinsource;
}

void TIM_ClockEnable(TIM_TypeDef* TIM)
{
  if (TIM == TIM1)
  {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
  }
  else if (TIM == TIM2)
  {
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
  }
  else if (TIM == TIM3)
  {
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
  }
  else if (TIM == TIM4)
  {
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
  }
  else if (TIM == TIM5)
  {
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
  }
  else if (TIM == TIM6)
  {
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);
  }
  else if (TIM == TIM7)
  {
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);
  }
  if (TIM == TIM8)
  {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE);
  }
  if (TIM == TIM9)
  {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM9, ENABLE);
  }
  if (TIM == TIM10)
  {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM10, ENABLE);
  }
  if (TIM == TIM11)
  {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM11, ENABLE);
  }
  else if (TIM == TIM12)
  {
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM12, ENABLE);
  }
  else if (TIM == TIM13)
  {
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM13, ENABLE);
  }
  else if (TIM == TIM14)
  {
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM14, ENABLE);
  }
}


float vsqrtf(float op1)
{
  float result;
  __ASM volatile ("vsqrt.f32 %0, %1" : "=w" (result) : "w" (op1) );
  return (result);
}



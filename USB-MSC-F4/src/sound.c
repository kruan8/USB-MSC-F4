/*
 * sound.c
 *
 *  Created on: 14. 1. 2019
 *      Author: V.Priesol
 */


#include "sound.h"
#include "common.h"
#include "timer.h"
#include "sounds.inc"

#define HW_PIEZO                     PA1
#define SND_TIM                      TIM2

#define SOUND_BUS_FREQ                42000000
#define SOUND_PERIOD                  10
#define SOUND_COMPARE                 (SOUND_PERIOD / 2)


// ukazatele na tabulky jednotlivych zvuku
const snd_tone_t* g_pSounds[] =
{
     t3x20,      // snd_3x20,
     t3x19,      // snd_3x19,
     t3x18,      // snd_3x18,
     t3x17,      // snd_3x17,
     t2x25,      // snd_2x25,
     pip1,       // snd_pip1,
     pip2,       // snd_pip2,
     pip3,       // snd_pip3,
     pip2i,      // snd_pip2i,
     pip3i,      // snd_pip3i,
     end,        // snd_prehozeno,
     end,        // snd_end,
     end,        // snd_asf,
     end,        // snd_nothrow40,
     song1,      // snd_song1,
     song2,      // snd_song2,
     song3,      // snd_song3,
     song4,      // snd_song4,
     song5,      // snd_song5,
     song6,      // snd_song6,
     testSound,  // snd_song6,
};

const snd_tone_t*   g_pActualSound = NULL;      // ukazatel na prehravany zvuk
uint16_t       g_nSoundPos;                     // pozice v datech zvuku
uint16_t       g_nActualFreq;                   // momentalni frekvence tonu
uint16_t       g_nSoundLen;                     // citac delky tonu
int16_t        g_nFreqChange;                   // zmena kmitoctu mezi vzorky

uint16_t       g_nCounter_10ms;                 // citac 10 ms, ve kterych jsou delky tonu

uint8_t        g_nSongNumber = 0;               // song counter

static void _SetFrequency(uint16_t nFreq);
static void _Timer_cb(void);

void Snd_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  GPIO_ClockEnable(GPIO_GetPort(HW_PIEZO));

  // LED PWM configuration
  GPIO_InitStructure.GPIO_Pin = GPIO_GetPin(HW_PIEZO);
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIO_GetPort(HW_PIEZO), &GPIO_InitStructure);
  GPIO_PinAFConfig(GPIO_GetPort(HW_PIEZO), GPIO_GetPinSource(GPIO_GetPin(HW_PIEZO)), GPIO_AF_TIM2);

  TIM_ClockEnable(SND_TIM);

  TIM_TimeBaseInitTypeDef timBase;
  timBase.TIM_ClockDivision = TIM_CKD_DIV1;
  timBase.TIM_Prescaler = 4200;             // 10 kHz
  timBase.TIM_Period = SOUND_PERIOD;                  // 1 kHz
  timBase.TIM_CounterMode = TIM_CounterMode_Up;
  timBase.TIM_RepetitionCounter = 0;
  TIM_TimeBaseInit(SND_TIM, &timBase);

  // PWM mode TIM12_CH2
  TIM_OCInitTypeDef TIM_OCInitStructure;
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = SOUND_COMPARE;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
  TIM_OC2Init(SND_TIM, &TIM_OCInitStructure);                 // CH2
  TIM_OC2PreloadConfig(SND_TIM, TIM_OCPreload_Enable);

  Timer_SetSysTickCallback(_Timer_cb);
}

void Snd_Play(snd_e eSound)
{
  g_pActualSound = g_pSounds[eSound];
  g_nSoundPos = 0;
  g_nSoundLen = 0;
  g_nCounter_10ms = 10;

  TIM_Cmd(SND_TIM, ENABLE);
}

void Snd_PlayNextSong(void)
{
  Snd_Play(sound_song1 + g_nSongNumber);
  g_nSongNumber++;
  if (g_nSongNumber == 6)
  {
    g_nSongNumber = 0;
  }
}

void _SetFrequency(uint16_t nFreq)
{
  SND_TIM->PSC = SOUND_BUS_FREQ / SOUND_PERIOD / nFreq;
}

void _Timer_cb(void)
{
  if (g_pActualSound == NULL)
  {
    return;
  }

  g_nCounter_10ms++;
  if (g_nCounter_10ms < 10)
  {
    return;
  }

  g_nCounter_10ms = 0;

  // zmena kmitoctu
  if (g_nFreqChange != 0)
  {
    g_nActualFreq += g_nFreqChange;
    _SetFrequency(g_nActualFreq);
  }

  if (g_nSoundLen == 0)
  {
    g_nSoundLen = g_pActualSound[g_nSoundPos].nLen;
    if (g_nSoundLen == 0)
    {
      // ukoncit
      TIM_Cmd(SND_TIM, DISABLE);
      g_pActualSound = NULL;
    }

    g_nActualFreq = g_pActualSound[g_nSoundPos].eNote;
    _SetFrequency(g_nActualFreq);
    g_nSoundLen = g_pActualSound[g_nSoundPos].nLen;
    g_nFreqChange = g_pActualSound[g_nSoundPos++].nFreqChange;
  }

  g_nSoundLen--;

}

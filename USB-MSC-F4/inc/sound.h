/*
 * sound.h
 *
 *  Created on: 14. 1. 2019
 *      Author: V.Priesol
 */

#ifndef INC_SOUND_H_
#define INC_SOUND_H_

#include "stm32f4xx.h"
#include <stdbool.h>
#include <stddef.h>

typedef enum
{
  sound_3x20 = 0,           // 3x20
  sound_3x19,               // 3x19
  sound_3x18,               // 3x18
  sound_3x17,               // 3x17
  sound_2x25,               // 2x25
  sound_pip1,               // pipnuti
  sound_pip2,               // pipnuti dvojite
  sound_pip3,               // pipnuti trojite
  sound_pip2i,              // pipnuti dvojite - ruzna vyska
  sound_pip3i,              // pipnuti trojite - ruzna vyska
  sound_prehozeno,          // prehozeno
  sound_end,                // konec na kola
  sound_asf,                // asfalt
  sound_nothrow40,          // nic nehozeno pri 40
  sound_song1,
  sound_song2,
  sound_song3,
  sound_song4,
  sound_song5,
  sound_song6,
  sound_testSound,
} snd_e;

void Snd_Init(void);
void Snd_Play(snd_e eSound);
void Snd_PlayNextSong(void);


#endif /* INC_SOUND_H_ */

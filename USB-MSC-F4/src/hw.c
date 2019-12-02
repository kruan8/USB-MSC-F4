/*
 * hw.c
 *
 *  Created on: 22. 11. 2019
 *  Author:     Priesol Vladimir
 */

#include "hw.h"
#include "common.h"
#include "timer.h"
#include "sound.h"

void HW_Init()
{
  Timer_Init();
  Snd_Init();

//  Snd_Play(sound_song1);
}

#ifndef __Audio__
#define __Audio__

#include "SDL_mixer.h"
enum S_GROUPS   {FX_NULL,FX_Bullets,FX_Impacts,FX_Monsters,FX_Player};

typedef struct SOUND_T
{
  Mix_Chunk *sound;
  char filename[80];
  int used;
  int volume;
}Sound;


void Init_Audio();
void InitSoundList();
void ClearSoundList();
void FreeSound(Sound *sound);
Sound *LoadSound(char filename[80],int volume);


#endif

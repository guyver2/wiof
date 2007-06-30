/****************************************************************************/
/*  PSON : module de sonor de Pgck, developpe par Antoine Letouzey          */
/*  ameboure@yahoo.fr                                                       */
/****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "Pson.h"
#include <SDL/SDL_mixer.h>


// variables globales
Mix_Music *fondMusique = NULL;
Mix_Chunk* tabSons[NB_SONS];



void PInitSon()
 {
  Mix_OpenAudio( 22050, MIX_DEFAULT_FORMAT, 2, 4096 );
  fondMusique = Mix_LoadMUS("datas/audio/fond.s3m");
  Mix_PlayMusic( fondMusique, -1 );
  tabSons[0] = Mix_LoadWAV("./datas/audio/help.wav");
  tabSons[1] = Mix_LoadWAV("./datas/audio/safe.wav");
  tabSons[2] = Mix_LoadWAV("./datas/audio/mort.wav");
  tabSons[3] = Mix_LoadWAV("./datas/audio/goutte.wav");
  tabSons[4] = Mix_LoadWAV("./datas/audio/menu.wav");
  tabSons[5] = Mix_LoadWAV("./datas/audio/gagne.wav");
  tabSons[6] = Mix_LoadWAV("./datas/audio/perdu.wav");
  tabSons[7] = Mix_LoadWAV("./datas/audio/select.wav");
  /*FILE* fic = fopen("std.out2","w");
  fprintf(fic, "musique de fond (%d)\n", fondMusique==NULL);
  int i;
  for(i=0; i<8; i++) fprintf(fic, "son num %d (%d)\n", i, tabSons[i]==NULL);
  fclose(fic);
  */
  atexit(PUnInitSon);
 }


void PUnInitSon()
 {
  Mix_FreeMusic(fondMusique);
  int i;
  for(i=0; i<NB_SONS; i++) Mix_FreeChunk(tabSons[i]);
  Mix_CloseAudio();
 }


/* change la musique de fond */
void PChangeMusiqueFond(char* path)
 {
  fondMusique = Mix_LoadMUS(path);
 }

/* adapte le volume */
int PSetVolume(int vol)
 {
  if (vol > MIX_MAX_VOLUME) vol = MIX_MAX_VOLUME;
  if (vol < 0) vol = 0;
  return Mix_VolumeMusic(vol);
 }

 int PGetVolume()
  {
   return Mix_VolumeMusic(-1);
  }

 /* joue ou coupe le fond */
void PPlayStopFond(int flag)
 {
  if (flag && !Mix_PlayingMusic()) Mix_PlayMusic( fondMusique, -1 );
  else {
   Mix_HaltMusic();
   Mix_HaltChannel(-1);
  }
 }

/* joue un son court sur un channel */
void PPlaySound(int sample, int channel)
 {
  if (sample >= 0 && sample <= NB_SONS)
   {
    Mix_HaltChannel(channel);
    Mix_PlayChannel(channel, tabSons[sample], 0);
   }
 }

/* coupe un channel */
void PStopChannel(int channel)
 {

 }

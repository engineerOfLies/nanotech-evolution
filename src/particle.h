#ifndef __PARTICLE__
#define __PARTICLE__

#include "graphics.h"


enum ParticleFlags {PF_NONE = 0,PF_WINDED = 1,PF_SPRITE = 2,PF_RADIAL = 4};

typedef struct Particle_T
{
  float sx,sy,sz;         /*coordinates of where the particle is getting rendered*/
  float vx,vy,vz;       /*vector of particle*/
  float ax,ay,az;       /*how is the particle accellerating*/
  Uint32 Color;         /*what color be I?*/
  Uint16  flags;        /*so far the only flag set is wind effected or not.*/
  Sint16  R,G,B;         /*Color Direction*/
  int lifespan;       /*how many updates before I die*/
        /*Some sprites may not be points, but sprites*/
  int frame,loop;          /*the frame it is on, and when to loop it*/
  int delay,delayrate;    /*how much time to pause between updating the frame*/
  Sprite *sprite;
  int fx,fy;              /*if we flip the sprite at draw*/
  int used;
}Particle;

float OffSet(float die);   /*this will give a random float between die and -die*/


void ResetAllParticles();
Particle *SpawnParticle();    /*give a pointer to newly created particles.*/
void DrawAllParticles();
void SpawnFountain(Uint32 Color,int mx,int my,Uint8 time);
void SpawnBloodSpray(Uint32 Color,int mx,int my,float vx,float vy,Uint8 time,int volume);
void SpawnDrip(Uint32 Color,int sx,int sy,int sz,Uint8 time);
void SpawnFire(Uint32 Color,int mx,int my,Uint8 time);
void ItsRaining(Uint32 Color,Uint8 time,int volume,float decent);
void SpawnRainSpot(Uint32 Color,int mx,int my,Uint8 time);
void SpawnBoltHail(Uint32 Color,int mx,int my,Uint8 time);
void SpawnThrust(Uint32 Color,int sx,int sy,float vx,float vy,Uint8 time,int volume);
void SpawnFadingParticle(Uint32 Color,int sx,int sy,Uint8 time);
void SpawnFallingParticle(Uint32 Color,int sx,int sy,int vx,int vy,Uint8 time);
void SpawnSpriteParticle(char filename[80],int w,int h,int frames,int sx,int sy,float vx,float vy,Uint8 time);
void SpawnSwapSpriteParticle(char filename[80],int w,int h,int frames,int c1,int c2, int c3, int sx,int sy,float vx,float vy,Uint8 time);
void SpawnSwapFlipSpriteParticle(char filename[80],int w,int h,int frames,int c1,int c2, int c3, int sx,int sy,float vx,float vy,int fx,int fy, Uint8 time);
void ExplodingParticle(int sx,int sy,float vx,float vy);
void ThrustingParticle(int sx,int sy,float vx,float vy);

#endif

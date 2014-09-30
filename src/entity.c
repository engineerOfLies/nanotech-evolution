/*
 *    Donald Kehoe
 *    Sometime in February
 *    Last Modified: 3/21/05
 *
 *    Description: definitions for Entity handling functions (methods).
 *      
*/
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "entity.h"
/*
  Entity function definitions
*/
#define   Depth       1
#define   RPixel_W    64
#define   RPixel_H    64
#define   SHADOWS 10

/*
  With this design I am intending to create a 2 dimensional bucket sort for collision detection.
  Each region will have a list of buckets.  We will start off with a simple bucket depth of 16 and increase the bucket depth as needed.  When the map is done, we will de-allocate the memory allocated by all buckets.
 */


extern SDL_Surface *screen;
extern SDL_Surface *clipmask;
extern SDL_Surface *background;
extern SDL_Rect Camera;
extern Uint32 NOW;

Entity EntityList[MAXENTITIES];  /*the last column is the world*/


/*unit and building information*/
/*I will make this like I make sprites, each "peon" will point to the same peon information, per player*/


int NumEnts = 0;
int MOUSEMOVE = 1;

Sprite *shadow[SHADOWS];
Sprite *auras[2];

/************************************************************************

                      Entity Creation and Management Functions

 ************************************************************************/
void InitEntityList()
{
  int i,j;
  NumEnts = 0;
  for(i = 0;i < MAXENTITIES; i++)
  {
    EntityList[i].sprite = NULL;
    EntityList[i].owner = NULL;
    EntityList[i].think = NULL;
    EntityList[i].target = NULL;
    EntityList[i].update = NULL;
    for(j = 0;j < SOUNDSPERENT;j++)
    {
      EntityList[i].sound[j] = NULL;
    }
    EntityList[i].shown = 0;
    EntityList[i].used = 0;
  }
  /*lets make sure our deadspace entity is blank and safe.*/
  NewEntity();/*just to start things off*/
}


/*
  draw all of the active entities in the view of the camera.  
  I my come up with a more eficient algorithm
 */

void DrawEntities()
{
  int i;
  for(i = 0;i < MAXENTITIES;i++)
  {
    if((EntityList[i].used == 1)&&(EntityList[i].shown == 1))
    {
      DrawEntity(&EntityList[i]);
    }
  }
}

void ThinkEntities()
{
  int i;
  for(i = 0;i < MAXENTITIES;i++)
  {
      if(EntityList[i].used)
      {
        if(EntityList[i].NextThink < NOW)
        {
          if(EntityList[i].think != NULL)
          {
            EntityList[i].think(&EntityList[i]);
            EntityList[i].NextThink = NOW + EntityList[i].ThinkRate;
          }
        }
      }
  }
}

void UpdateEntities()
{
  int i;
  int checked = 0;
  for(i = 0;i < MAXENTITIES;i++)
  {
      if(EntityList[i].used)
      {
        checked++;
        if(EntityList[i].NextUpdate < NOW)
        {
          if(EntityList[i].update != NULL)
          {
            EntityList[i].update(&EntityList[i]);
            EntityList[i].NextUpdate = NOW + EntityList[i].UpdateRate;
          }
        }
      }
  }
  
}

void DrawEntity(Entity *ent)
{
  if(ent == NULL)return;
  if(ent->shown)
  {
    if(ent->shadow != -1)
    {
      if(shadow[ent->shadow] != NULL)
      {
        DrawSprite(shadow[ent->shadow] ,screen,ent->s.x - Camera.x - (shadow[ent->shadow]->w/2),ent->s.y - Camera.y - (shadow[ent->shadow]->h/2),0);
      }
    }
    if(ent->state == MS_Evolving)
    {
      DrawSprite(auras[0],screen,ent->s.x - Camera.x - (auras[0]->w/2),(ent->s.y - Camera.y) - (auras[0]->h * .75), ent->fcount);
    }
      if(ent->sprite != NULL)
      {
        if(ent->alpha != SDL_ALPHA_OPAQUE)/*since many entities share the same sprite, we gotte do this on a per entity basis.*/
        {
          SDL_SetAlpha(ent->sprite->image,SDL_SRCALPHA|SDL_RLEACCEL, (Uint8)ent->alpha);
        }
        
        if(ent->xflip)DrawSpriteFlipped(ent->sprite,screen,ent->s.x - Camera.x - (ent->sprite->w/2),(ent->s.y - Camera.y) - ent->height,1,0, ent->frame);
        else DrawSprite(ent->sprite,screen,ent->s.x - Camera.x - (ent->sprite->w/2),(ent->s.y - Camera.y) - ent->height,ent->frame);
        
        if(ent->alpha != SDL_ALPHA_OPAQUE)/*since many entities share the same sprite, we gotte do this on a per entity basis.*/
        {
          SDL_SetAlpha(ent->sprite->image,SDL_SRCALPHA|SDL_RLEACCEL, SDL_ALPHA_OPAQUE);
        }
      }
      if(ent->state == MS_Evolving)
      {
        DrawSprite(auras[1],screen,ent->s.x - Camera.x - (auras[1]->w/2),(ent->s.y - Camera.y) - (auras[1]->h * .75), ent->fcount);
      }
  }
}

void LoadShadows()
{
  int i;
  auras[0] = LoadSwappedSprite("images/auraback.png",256,256,LightBlue,LightBlue,LightViolet);
  auras[1] = LoadSwappedSprite("images/aurafront.png",256,256,LightBlue,LightBlue,LightViolet);
  SDL_SetAlpha(auras[0]->image,SDL_SRCALPHA|SDL_RLEACCEL, 128);
  SDL_SetAlpha(auras[1]->image,SDL_SRCALPHA|SDL_RLEACCEL, 128);
  shadow[0] = LoadSprite("images/shadow_16.png",16,12);
  shadow[1] = LoadSprite("images/shadow_24.png",24,18);
  shadow[2] = LoadSprite("images/shadow_32.png",32,24);
  shadow[3] = LoadSprite("images/shadow_48.png",48,36);
  shadow[4] = LoadSprite("images/shadow_64.png",64,48);
  shadow[5] = LoadSprite("images/shadow_96.png",96,72);
  shadow[6] = LoadSprite("images/shadow_128.png",128,96);
  for(i = 0;i < SHADOWS;i++)
  {
    if(shadow[i] != NULL)
      SDL_SetAlpha(shadow[i]->image,SDL_SRCALPHA|SDL_RLEACCEL, 128);
  }
}

void FreeShadows()
{
  int i;
  for(i = 0;i < 7;i++)FreeSprite(shadow[i]);
}


/*this is just the prep for generic Buildings, each unit will have more specific data set later*/

/*
  returns NULL if all filled up, or a pointer to a newly designated Entity.
  Its up to the other function to define the data.
*/
Entity *NewEntity()
{
  int i;
  if(NumEnts + 1 >= MAXENTITIES)
  {
    return NULL;
  }
  NumEnts++;
  for(i = 0;i < MAXENTITIES;i++)
  {
    if(!EntityList[i].used)break;
  }
  EntityList[i].used = 1;
  EntityList[i].Player = -1;
  EntityList[i].alpha =SDL_ALPHA_OPAQUE ;
  return &EntityList[i];
}

/*done with an entity, now give back its water..I mean resources*/
void FreeEntity(Entity *ent)
{
  int j;
  /*  fprintf(stdout,"Freeing %s\n",ent->EntName);*/
  ent->used = 0;
  NumEnts--;
  if(ent->sprite != NULL)FreeSprite(ent->sprite);
  for(j = 0;j < SOUNDSPERENT;j++)
  {
    if(ent->sound[j] != NULL)FreeSound(ent->sound[j]);
    ent->sound[j] = NULL;
  }
  ent->sprite = NULL;
  ent->owner = NULL;
  ent->think = NULL;
  ent->target = NULL;
  ent->update = NULL;
      /*delete the infor first, then close the pointer*/
}

/*kill them all*/
void ClearEntities()
{
  int i;
  for(i = 0;i < MAXENTITIES;i++)
    {
      FreeEntity(&EntityList[i]);
    }
}

/************************************************************************

                      Entity Maintanence functions

 ************************************************************************/
void DamageTarget(Entity *attacker,Entity *inflictor,Entity *defender,int damage,int dtype,float kick,float kickx,float kicky)
{     /*inflictor is the entity that is physically doing the damage, attacker is the entity that initiated it.*/
      /*Exampe: the player is the attacker, but the shotgun shell is the inflictor*/
  if(defender == NULL)return;   /*you never know*/
  
  
  if(damage <= 0)damage = 1;/*you will at LEAST take 1 damage*/
  /*adding Id style obituary code here*/
}

Coord AddVectors(Coord v1,Coord v2)
{
  Coord result;
  result.x = (v1.x + v2.x)*0.5;
  result.y = (v1.y + v2.y)*0.5;
  result.z = (v1.z + v2.z)*0.5;
  return result;
}

Coord FastAddVectors(Coord v1,Coord v2)
{
  Coord result;
  result.x = (int)(v1.x + v2.x)>>1;
  result.y = (int)(v1.y + v2.y)>>1;
  result.z = (int)(v1.z + v2.z)>>1;
  return result;
}


/************************************************************************

                      Simple generic helper functions follow

 ************************************************************************/



/**/








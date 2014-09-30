#ifndef _ENTITY_
#define _ENTITY_

#include "graphics.h"
#include "audio.h"

/*
  This file contains the data structures and function prototypes for handling entities.
  Entities all contain information about the associate sprite that may be unique to itself
  as well as the animation information, if it has any.
  
*/

#define MAXENTITIES   16
#define SOUNDSPERENT  4
#define MAXITEMS    256


/*random functions stolen shamelessly from quake2*/
#define random()  ((rand () & 0x7fff) / ((float)0x7fff))
#define crandom() (2.0 * (random() - 0.5))

enum SOUNDTYPE  {SF_ALERT,SF_IMPACT,SF_PAIN,SF_DYING};
enum MonsterStates {MS_Normal,MS_Happy,MS_Sad,MS_Combat,MS_Event,MS_Evolving = -1};


typedef struct POINT_T
{
  int x,y,z;
}Point;

typedef struct COORD_T
{
  float x,y,z;
}Coord;


typedef struct ENTITY_T
{
  int used;                 /*used is for keeping track of what entities are free and should NEVER be touched.*/
  Uint32 LastDrawn;         /*another one that should never be touched by anything but the maintainence functions*/
  int Player;               /*references the player controlling the unit, -1 implies it is owned by the 'world'*/
  int EntClass;             /***/
  int Unit_Type;            /*ET_* for hit detection masking*/
  char EntName[40];         /*the name of the entity*/
  Sprite *sprite;           /*the sprite to be drawn for the main part of the entity*/
  Sound *sound[SOUNDSPERENT];/*a list of pointers to the wav files that this entity will produce*/
  
  struct ENTITY_T *owner;   /*for bullets, drones and other things spawned by other entities*/
  struct ENTITY_T *target;  /*used for MANY units: attack target, healing target, etc*/
  
  void (*think) (struct ENTITY_T *self);    /*called by the engine every so often to handle input and make decisions*/
  Uint32 NextThink;         /*used for how often the entity thinks*/
  Uint16 ThinkRate;         /*used for incrementing above*/
  
  void (*update) (struct ENTITY_T *self);   /*called by the engine every so often to update the position and check for collisions*/
  Uint32 NextUpdate;        /*used for how often the entity is updated, updating is merely animations*/
  Uint16 UpdateRate;        /*used for incrementing above*/
  
  int Color;                /*the index color for bullets and trails and such*/
  int alpha;                /*translucency*/
  int xflip;                /*if its drawn mirrored X*/
  int shadow;               /*which shadow size we have*/
  int height;               /*how far up we need to offet the sprite for drawing*/
  int shown;                /*if 1 then it will be rendered when it is on screen, if 0, but owned by the player, then it will be rendered translucent*/
  int frame;                /*current frame to render*/
  int frdir;                /*direction of animation*/
  int fcount;               /*used for animation, the loop variable*/
  int frate;                /*how often we update our frames*/
  Uint32 framedelay;        /*ammount of delay between frames*/
  int face;                 /*the direction we are moving*/
  
  int asleep;               /*keeps track of the player's sleeping*/
  int state;                /*making each entity a finite state machine.*/
  
  SDL_Rect  Boundingbox;    /*the bounding box for collision detection*/
  
  Coord s;                  /*screen coordinates*/
  Coord a;                  /*acceleration*/
  Coord v;                  /*vector values*/
  int updatedelay;          /*further delay used in updating...*/
  int switchdelay;          /*stance switching delay*/
  int health,healthmax;     /*entity health, not monster health*/
  
  int skillconflict;        /*set if it needs to be resolved at evolution*/
  
  int decisiondelay;        /*for AI*/
/*
  
  player character section
*/
  int sleeptime;
  int playernum;            /*which number this player is in the save list*/
  int combatindex;          /*which player this is for combat*/
/*end player character section*/
}Entity;

/*startup and clean up*/
void InitEntityList();
void ClearEntities();

/*creation and destruction*/
Entity *NewEntity();
void FreeEntity(Entity *ent);

/*update functions*/
void DrawEntity(Entity *ent);
void DrawEntityTrail(Entity *ent);
void DrawEntities();
void UpdateEntities();
void ThinkEntities();
void LoadShadows();
void FreeShadows();

/*character updating functions that have to go here so we don't create a causality paradox and accidentally kill our grandparents before our parents are conceived.  Or do the nasty in the pasty.*/
void TrainSkill(char *name,Entity *self,int uses);  /*when using a skill, other skills are trained a little bit too, elements may be updated as well.*/



#endif

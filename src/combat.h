#ifndef __COMBAT__
#define __COMBAT__
/*
 
  The handling of combat in the game...
 */

#include "entity.h"


typedef struct
{
  char name[80];                    /*monster's nickname*/
  char stagename[80];               /*the name of the stage of evolution*/
  int stageid;                      /*the unique number for the stage*/
  int Aspect;                       /**/
  int Species;                      /**/
  float good,evil;                    /*alignment*/
  float Element[5];                 /*elemental strengths*/
  int attributes[6];                /*attribute scores*/
  int attributetrains[6];           /*attribute training*/
  int numdefenses;                  /*for tracking how many*/
  int numattacks;
  char attacks[8][80];              /*available attacks*/
  char defenses[8][80];             /*available defenses and non-combat skills*/
  int nummovements;                 /*number of defenses that are movements*/
  int Effects[10];                  /*(Sick, Poison, Slow, Stun, )*/
  float health,stamina;             /*spent in damage and skill use*/
  int healthmax,staminamax;         /*calculated from attributes*/
  int speed;                        /*calculated based on attributes*/
  float fullness;                   /*how much food is in our stomache*/
  int fatigue;
  int inventory[MAXITEMS];          /*ammounts of items found*/
}NetStatInfo;

typedef struct
{
  Entity *Combatant;      /*pointers to the player ents.*/
  Uint32     Type;           /*player controlled, AI, or net controlled*/
  float   AttackCooldown;  /*for attacks, items and recovery*/
  float   DefenseCooldown; /*for use of active defenses*/
  Sint32     ChosenAttack;    /*which attack will be executed*/
  Sint32     ChosenDefense;   /*which defense will be executed when attacked*/
  Uint32     realtivespeed;   /*speed at which cool downs happen*/
  int attributes[6];          /*attribute bonuses*/
}Combat_Info;

typedef struct
{
  Uint32 Frame;
  Uint32 State;
}FrameData;

void CombatBeginEncounter(char uname[80],int level);/*for random encounters*/
void CombatBeginNet(int host);
void CombatEnd();
void UpdateCombatMode();

#endif

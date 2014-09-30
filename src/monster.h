#ifndef __MONSTER__
#define __MONSTER__
/*
This file will contain the info for the monsters in the game and all related functions

*/
#include "entity.h"
#include "area.h"
#include "game.h"

#define MAXSTAGES   1024
#define MAXPLAYERS  8

enum Element_Types   {E_Earth, E_Water, E_Air, E_Fire, E_Lightning, E_Ice, E_Mist, E_Light, E_Radiation, E_Thunder};
enum Combat_Types    {CS_Melee,CS_Speed,CS_Tank,CS_Energy,CS_Effects};
enum Attribute_Types {AT_Strength,AT_Dexterity,AT_Focus,AT_Endurance,AT_Agility,AT_Resistance};
enum P_Aspects {PA_Beast,PA_Techno,PA_Spirit,PA_Humon,PA_Mech};
enum P_Species {PA_Lizard,PS_Bird,PS_Mammal,PS_Insect,PS_Plant};
enum S_Stages  {S_Egg,S_Baby,S_Child,S_Teen,S_Adult,S_Final};

#define ThePlayerStats PlayerStats[Player->playernum]

typedef struct
{
  char name[80];
  int stage;                        /*stage of growth*/
  int id;                        /*unique monster ID*/
  int Aspect;
  int Species;
  int CombatStyle;                  /* (Attacker, Tank, INT, AGI)*/
  int attributes[6];                /*attribute modifications*/
  float attributenumbers[6];        /*attribute modifications*/
  char attacks[8][80];              /*available attacks*/
  int numattacks;
  char defenses[8][80];             /*available defenses*/
  int numdefenses;
  int Element[5];                   /*modifiers*/
  char description[120];
  char spritename[80];
  int framew,frameh;
  int height;
  char soundset[80];
  int colorswaps[3];
  char conditions[8][80];           /*list of things to check against*/
  int conditionvalue[8];            /*how much of condition needs to be met*/
}StageInfo;

typedef struct
{
  int majorversion;                 /*for making sure the data structures are compatible for net*/
  int minorversion;                 /*for keeping track of what features are available*/
  char name[80];                    /*monster's nickname*/
  char stagename[80];               /*the name of the stage of evolution*/
  int stageid;                      /*the unique number for the stage*/
  char stagehistory[8][80];         /*previous stages in case of devolving*/
  int stage;                        /*stage of growth*/
  int Aspect;                       /**/
  int Species;                      /**/
  int Age;                          /*how old (game time) years*/
  int AgeStep;                      /*days for the year....*/
  float good,evil;                    /*alignment*/
  float Element[5];                 /*elemental strengths*/
  int attributes[6];                /*attribute scores*/
  int attributetrains[6];           /*attribute training*/
  int numdefenses;                  /*for tracking how many*/
  int numattacks;
  char attacks[8][80];              /*available attacks*/
  char defenses[8][80];             /*available defenses and non-combat skills*/
  int nummovements;                 /*number of defenses that are movements*/
  int wins,loses;                   /*battle records*/
  int Effects[10];                  /*(Sick, Poison, Slow, Stun, )*/
  float health,stamina;             /*spent in damage and skill use*/
  int healthmax,staminamax;         /*calculated from attributes*/
  int speed;                        /*calculated based on attributes*/
  float fullness;                   /*how much food is in our stomache*/
  int independance;
  int Respect;
  int Mood;
  int emotional_state[6];
  int fatigue;
  int asleep;
  int sleeptime;
  char areaname[80];                /*the current area we reside in*/
  int inventory[MAXITEMS];          /*ammounts of items found*/
  int foodsupply;                   /*how much food has been forraged*/
  int enterredbattle;               /*set so we can see if someone quits a battle...*/
  AreaHistory history[MAXAREAS];    /*the history of the world part 1*/

}SaveInfo;

/*filename is the name of the save file to load or the name of the monster file to create from.*/
Entity *SpawnMonster(int brandnew,char filename[80],int playernum);
void MonsterUpdate(Entity *self);
void MonsterThink(Entity *self);
void ResetStats(int num);
void ReorganizeSkills(SaveInfo *stats);

/*monster controls*/
void WakeMonster();

/*these would be in area.h if it wasn't so confusing*/
void AddAreaToHistory(Area *area , SaveInfo *info);
int GetHistoryIndexByUName(char uname[80],SaveInfo *info);
void MigrateTo(char uname[80]);

int GetPlayerSkillIndexByName(char uname[80]);
int GetPlayerAttackIndexByName(char uname[80]);
void ResolveSkillConflict();
void ResolveDefSkillConflict();


void Matriculate();
void SkillUsed(SaveInfo *stats,char uname[80]);   /*called after a skill is used to record growth*/
void AttributeUsed(Entity *self,int attindex, int uses);
float  ElementalWeight(float AE[5],float DE[5]);    /*attack elements vs defense elements*/
int  DominantElement(int Element[5]);          /*returns the dominant element or -1 if neutral.*/
char *GetElementPrefix(int Element,int stage);
int  DominantAlignment(int good,int evil);     /*returns -1 for evil, 1 for good, 0 for balance, 2 for not clear.*/
char *GetAlignmentPrefix(int alignment, int level);

/*the saving loading and parsing of stages*/

StageInfo *GetNextStageByAge(StageInfo *start,int age);
StageInfo *GetStageInfoByName(char name[80]);
void ClearStageInfo();
void LoadStageInfoBinary();
void LoadStageInfoText();
void SaveStageInfoBinary();
void SaveStageInfoText();

int  SaveGame(Entity *self);    /*saves the game to disk*/
int  LoadGame(Entity *self);    /*loads the game from disk*/

#endif

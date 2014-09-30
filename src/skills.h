#ifndef __SKILLS__
#define __SKILLS__
/*
  DJ Kehoe
  11/2007
  This file set handles the access and use of skills in the game
*/


/*
  Notes on skill functions:
  
 */
enum EffectPower      {EP_None, EP_Minimal, EP_Light, EP_Medium, EP_Heavy, EP_Great, EP_Massive};
enum EffectFunctions  {EF_Hit, EF_Chargehit, EF_DoubleHit, EF_TripleHit, EF_Dodge, EF_Brace, EF_Counter, EF_Negate, EF_Reflect, EF_Movement};
#define MAXSKILLS 512

typedef struct
{
  char uname[80];       /*unique name of the skill to search by*/
  char skillname[80];   /*Plain name of skill to draw to screen*/
  int skilltype;        /*what is it, attack, defense, movement or train*/
  int staminacost;      /*use of this skill costs this much stamina*/
  int healthcost;       /*use of this skill costs this much health*/
  int animation;        /*which monster animation to use*/
  int projectile;       /*its its a projectile or melee*/
  char sprite[80];      /*the sprite for the projectile*/
  int sw;               /*the width & height of the sprite*/
  int sh;               /*the width & height of the sprite*/
  char hitevent[80];    /*the event for an impact (explosion)*/
  float hitrate;        /*how often this attack hits before stats are factored*/
  int hitbonus;         /*what attribute this skill gains bonuses to hit from.*/
  char function[80];    /*this is the function to call for this skill*/
  char effect[80];      /*this is the function to calculate the damage (or whatever)*/
  int primary;          /*this is the primary attribute used to determine damage*/
  int secondary;        /*secondary attribute if its used*/
  int trains[6];        /*which attributes are trained with this skill*/
  int elements[5];      /*how much elemental weight is given to the attack/defense*/
  int good,evil;        /*how much alignment is given to the attack/defense*/
  int statuseffect;     /*the attack can have a status effect, but only 1 to be fair*/
  int cooldown;         /*Use of this will effect the delay between moves*/
  int colorswaps[3];    /*for the sprite*/
  char description[120];/*text about the attack*/
}Skill_Info;

Skill_Info *GetSkillInfoByName(char uname[80]);
void SetupTrainWindow();
void UpdateTrainTypeWindow(int pressID);
int  attrLetToIndex(int l);
void SetupTrainWindow();
void ClearSkillInfo();
void LoadSkillInfoBinary();
void LoadSkillInfoText();
void SaveSkillInfoBinary();
void SaveSkillInfoText();
int  SkillRoll();       /*returns a random number from 1 to 10 with chances for graces and bogies*/
int  GetPower(char powertype[80],int primary,int secondary);
char *PowerLevelName(int level);

#endif


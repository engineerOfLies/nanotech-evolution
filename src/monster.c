#include "monster.h"
#include "area.h"
#include "hud.h"
#include "skills.h"

Entity *Player;
SaveInfo PlayerStats[MAXPLAYERS];
extern const Uint32 MajorVersion;
extern const Uint32 MinorVersion;
extern Area *CurrentArea;
extern SDL_Rect Camera;
extern int CombatMode;
extern HUDInfo hud;
enum SKILLTYPES {ST_Attacks,ST_Defenses};
int GAMESPEED = 1000;

void EvolutionCheck(Entity *self);

StageInfo StageList[MAXSTAGES];
int NumStages = 0;

Entity *SpawnMonster(int brandnew,char name[80],int playernum)
{
  int i;
  Entity *self;
  StageInfo *stage = NULL;
  self = NewEntity();
  if(self == NULL)return NULL;
  Player = self;
  self->playernum = playernum;
  switch(brandnew)
  {
    case 0:
      if(LoadGame(self) == 1)
      {
        stage = GetStageInfoByName(PlayerStats[playernum].stagename);
        self->sprite = LoadSwappedSprite(stage->spritename,stage->framew,stage->frameh,stage->colorswaps[0],stage->colorswaps[1],stage->colorswaps[2]);
        LoadArea(PlayerStats[playernum].areaname);
        break;
      }
    case 1:
      stage = GetStageInfoByName(name);
      if(stage == NULL)
      {
        stage = GetStageInfoByName("egg");
        if(stage == NULL)
        {
          fprintf(stderr,"unable to find stage information!\n");
          exit(0);
        }
      }
      self->sprite = LoadSwappedSprite(stage->spritename,stage->framew,stage->frameh,stage->colorswaps[0],stage->colorswaps[1],stage->colorswaps[2]);
      strncpy(PlayerStats[playernum].stagename,stage->name,80);
      PlayerStats[playernum].stageid = stage->id;
      strncpy(PlayerStats[playernum].areaname,"plains001",80);
      PlayerStats[playernum].attributes[AT_Strength] = 10 + (crandom()*5);
      PlayerStats[playernum].attributes[AT_Dexterity] = 10 + (crandom()*5);
      PlayerStats[playernum].attributes[AT_Focus] = 10 + (crandom()*5);
      PlayerStats[playernum].attributes[AT_Endurance] = 10 + (crandom()*5);
      PlayerStats[playernum].attributes[AT_Agility] = 10 + (crandom()*5);
      PlayerStats[playernum].attributes[AT_Resistance] = 10 + (crandom()*5);
      Matriculate();
      PlayerStats[playernum].health = PlayerStats[playernum].healthmax;
      PlayerStats[playernum].stamina = PlayerStats[playernum].staminamax;
      strncpy(PlayerStats[playernum].stagehistory[0],stage->name,80);
      PlayerStats[playernum].foodsupply = 10;
      PlayerStats[playernum].fullness = 10;
      PlayerStats[playernum].majorversion = MajorVersion;
      PlayerStats[playernum].minorversion = MinorVersion;
      PlayerStats[playernum].independance = 0;
      PlayerStats[playernum].Respect = 0;
      PlayerStats[playernum].Mood = 0;
      PlayerStats[playernum].numattacks = 0;
      PlayerStats[playernum].numdefenses = 0;
      for(i = 0;i < 8;i++)
      {
        if((strncmp(stage->attacks[i],"NULL",80) != 0)&&(strlen(stage->attacks[i]) > 0))
        {
          PlayerStats[playernum].numattacks++;
          strncpy(PlayerStats[playernum].attacks[i],stage->attacks[i],80);
        }
      }
      for(i = 0;i < 2;i++)
      {
        if((strncmp(stage->defenses[i],"NULL",80) != 0)&&(strlen(stage->defenses[i]) > 0))
        {
          PlayerStats[playernum].numdefenses++;
          strncpy(PlayerStats[playernum].defenses[i],stage->defenses[i],80);
        }
      }
      
      LoadArea(PlayerStats[playernum].areaname);
      AddAreaToHistory(CurrentArea , &PlayerStats[playernum]);
      break;
  }
  self->shown = 1;
  self->shadow = 4;
  self->height = stage->height;
  self->s.y = 200;
  self->s.x = 240;
  self->frame = 0;
  self->update = MonsterUpdate;
  self->NextUpdate = 0;
  self->UpdateRate = 240;
  self->think = MonsterThink;
  self->NextThink = 0;
  self->ThinkRate = 80;
  self->state = 0;
  self->frdir = 1;
  self->playernum = playernum;
  return self;
}

void UpdateDummy(Entity *self)
{
  self->fcount = (self->fcount + 1)%32;
}

Entity *SpawnDummyEntity(Entity *me)
{
  Entity *self;
  self = NewEntity();
  if(self == NULL)return NULL;
  self->shown = 1;
  self->shadow = me->shadow;
  self->height = me->height;
  self->s.y = 200;
  self->s.x = 240;
  self->frame = 0;
  self->state = MS_Evolving;
  self->sprite = me->sprite;
  self->update = UpdateDummy;
  self->UpdateRate = 80;
  return self;
}

void MonsterThink(Entity *self)
{
  if((self->state == MS_Evolving)||(self->state == MS_Event))return;
  if((!self->asleep)&&(PlayerStats[self->playernum].stage > 0))
  {
    /*recovery while awake*/
    PlayerStats[self->playernum].health += PlayerStats[self->playernum].healthmax * 0.0001;
    if(PlayerStats[self->playernum].health > PlayerStats[self->playernum].healthmax)
      PlayerStats[self->playernum].health = PlayerStats[self->playernum].healthmax;
       
    PlayerStats[self->playernum].stamina += PlayerStats[self->playernum].staminamax * 0.0001;
    if(PlayerStats[self->playernum].stamina > PlayerStats[self->playernum].staminamax)
      PlayerStats[self->playernum].stamina = PlayerStats[self->playernum].staminamax;

    /*other stats tick down*/
    PlayerStats[self->playernum].fullness -= (0.0002 + (PlayerStats[self->playernum].fatigue * 0.000001));
    if(PlayerStats[self->playernum].fullness < 0)
    {
      PlayerStats[self->playernum].fullness = 0;
      if(PlayerStats[self->playernum].health > 0.002)
        PlayerStats[self->playernum].health-= 0.002;
      if(crandom() > 0.5)
        PlayerStats[self->playernum].fatigue++;
    }
    
    if(crandom() > 0.5)
      PlayerStats[self->playernum].fatigue++;
    if(PlayerStats[self->playernum].fatigue >= 1000)
    {
      self->state = MS_Sad;
      if(PlayerStats[self->playernum].stamina <= 0)
      {
        PlayerStats[self->playernum].stamina = 0;
        if(PlayerStats[self->playernum].health > 1)
          PlayerStats[self->playernum].health-= 0.02;
      }
      else PlayerStats[self->playernum].stamina-= 0.0001;
    }
    if(PlayerStats[self->playernum].fatigue >= 2000)self->asleep = 1;
  }
  else    /*we're asleep*/
  {
    /*skill growth*/
    Matriculate();
    /*recovery*/
    if(PlayerStats[self->playernum].health < PlayerStats[self->playernum].healthmax)
      PlayerStats[self->playernum].health+= ThePlayerStats.healthmax * 0.001;
    if(PlayerStats[self->playernum].health > PlayerStats[self->playernum].healthmax)
      PlayerStats[self->playernum].health = PlayerStats[self->playernum].healthmax;
    if(PlayerStats[self->playernum].stamina < PlayerStats[self->playernum].staminamax)
      PlayerStats[self->playernum].stamina+= ThePlayerStats.staminamax * 0.001;
    if(PlayerStats[self->playernum].stamina > PlayerStats[self->playernum].staminamax)
      PlayerStats[self->playernum].stamina = PlayerStats[self->playernum].staminamax;
    
    /*handle sleep*/
    PlayerStats[self->playernum].fullness -= (0.0003);

      PlayerStats[self->playernum].fatigue--;
    if(PlayerStats[self->playernum].fatigue < 0)PlayerStats[self->playernum].fatigue = 0;
    self->sleeptime++;
    if((self->sleeptime >= 2000) || ((PlayerStats[self->playernum].fatigue == 0) && (PlayerStats[self->playernum].stamina >= PlayerStats[self->playernum].staminamax)))
    {
      self->sleeptime = 0;
      if(hud.subwindow == 0)
        strncpy(hud.buttons[5].text,"Rest",80);
      self->asleep = 0;
    }
    return;
  }
  switch(self->state)
  {
    case MS_Normal:
      if((PlayerStats[self->playernum].health < (PlayerStats[self->playernum].healthmax / 4))||(PlayerStats[self->playernum].stamina < (PlayerStats[self->playernum].staminamax / 4))||(PlayerStats[self->playernum].fullness < (PlayerStats[self->playernum].stage * 2))||(PlayerStats[self->playernum].fullness > (PlayerStats[self->playernum].stage * 25)))
      {
        self->state = MS_Sad;
      }
      break;
    case MS_Sad:
      if((PlayerStats[self->playernum].health > (PlayerStats[self->playernum].healthmax / 4))&&(PlayerStats[self->playernum].stamina > (PlayerStats[self->playernum].staminamax / 4))&&(PlayerStats[self->playernum].fullness > (PlayerStats[self->playernum].stage * 2)))
      {
        self->state = MS_Normal;
      }
      break;
  }
}

/*
 *basic animation maintenance
 */

void MonsterUpdate(Entity *self)
{
  int i;
  int areaindex;
  /*while evolving nothing else will happen*/
  if(self->state == MS_Event)
  {
    return;
  }
  if(self->state == MS_Evolving)
  {
    if(self->updatedelay >= 20)
    {
      self->updatedelay = 0;
      self->state = MS_Normal;
      FreeEntity(self->target);
      self->shown = 1;
      if(self->skillconflict)
      {
        self->skillconflict = 0;
        ResolveSkillConflict();
      }
      else TextDialog("Nickname:",ThePlayerStats.name,NULL);
    }else self->updatedelay++;
    return;
  }
  /*age growth*/
  PlayerStats[self->playernum].AgeStep++;
  if(PlayerStats[self->playernum].AgeStep >= GAMESPEED)
  {
    PlayerStats[self->playernum].AgeStep -= GAMESPEED;
    PlayerStats[self->playernum].Age++;
    areaindex = GetHistoryIndexByUName(CurrentArea->uname, &PlayerStats[self->playernum]);
    if(areaindex != -1)
      PlayerStats[self->playernum].history[areaindex].timespent++;
    if((self->state == MS_Normal)||(self->state == MS_Happy))
    {
      EvolutionCheck(self);
      if(self->state == MS_Evolving)return;
    }
  }
  /*environmental update*/
  if(self->updatedelay >= 100)
  {
    self->updatedelay = 0;
    for(i = 0; i < 5; i++)
    {
      PlayerStats[self->playernum].Element[i] += (CurrentArea->elements[i] * 0.1);
    }
    if(CurrentArea->alignment > 0)PlayerStats[self->playernum].good++;
    if(CurrentArea->alignment < 0)PlayerStats[self->playernum].evil++;
    
  }else self->updatedelay++;
  /*animation handling*/
  if(self->asleep)
  {
    if(crandom() > 0.5)
      NewMessage("                             Z",IndexColor(DarkGrey));
    NewMessage(" ",IndexColor(DarkGrey));
    self->frame = 7;
    return;
  }
  switch(self->state)
  {
    case MS_Normal:
      if(self->fcount > 0)
      {
        self->fcount--;
        break;
      }
      if((random()*2) > 0.8)self->frame+= self->frdir;
      if(self->frame == 0)
      {
        self->frdir = 1;
      }
      if(self->frame == 2)
      {
        self->frdir = -1;
      }
      if((self->frame > 2)||(self->frame < 0))self->frame = 1;
      break;
    case MS_Happy:
      if((self->frame >= 4)||(self->frame < 3))self->frame = 3;
      else if((random()*2) > 0.95)
      {
        self->frame++;
        self->fcount = 4;
        self->state = MS_Normal;
      }
      break;
    case MS_Sad:
      if(PlayerStats[self->playernum].health < (PlayerStats[self->playernum].healthmax / 4))
      {
        if(crandom() > 0.7)
          NewMessage("Oww...",IndexColor(DarkRed));
      }
      if(PlayerStats[self->playernum].stamina < (PlayerStats[self->playernum].staminamax / 4))
      {
        if(crandom() > 0.7)
          NewMessage("I'm Exhausted...",IndexColor(DarkRed));
      }
      if(PlayerStats[self->playernum].fullness < (PlayerStats[self->playernum].stage * 2))
      {
        if(crandom() > 0.7)
          NewMessage("Soo hungry!",IndexColor(DarkRed));
      }
      if(PlayerStats[self->playernum].fullness > (PlayerStats[self->playernum].stage * 25))
      {
        if(crandom() > 0.7)
          NewMessage("Too Full!",IndexColor(DarkRed));
      }
      if((self->frame >= 6)||(self->frame < 5))self->frame = 5;
      else  if((random()*2) > 0.95)self->frame++;
      break;
    default:
      self->state = MS_Normal;
  }
}

/*
 Monster skills and growth
 */
void SkillUsed(SaveInfo *stats,char uname[80])
{
  int i;
  Skill_Info *skill;
  skill = GetSkillInfoByName(uname);
  if(skill == NULL)
  {
    fprintf(stdout,"skill not found.\n");
    return;
  }
  for(i = 0;i < 6;i++)
  {
    stats->attributetrains[i] += skill->trains[i];
  }
  for(i = 0;i < 5;i++)
  {
    stats->Element[i] += (skill->elements[i] * 0.1);
  }
  stats->good += (skill->good * 0.1);
  stats->evil += (skill->evil * 0.1);
}

void AttributeUsed(Entity *self,int attindex,int uses)
{
  PlayerStats[self->playernum].attributetrains[attindex]+=uses;
}

float  ElementalWeight(float AE[5],float DE[5])
{
  int i;
  float total = 0;
  for(i = 0;i < 5;i++)
  {
    total += AE[i] - DE[i];
  }
  return total;
}

int GetPlayerSkillIndexByName(char name[80])
{
  int i;
  for(i = 0;i < 8;i++)
  {
    if(strncmp(ThePlayerStats.defenses[i],name,80)==0)
    {
      return i;
    }
  }
  return -1;
}

int GetPlayerAttackIndexByName(char name[80])
{
  int i;
  for(i = 0;i < 8;i++)
  {
    if(strncmp(ThePlayerStats.attacks[i],name,80)==0)
    {
      return i;
    }
  }
  return -1;
}


void Matriculate()
{
  int i;
  for(i = 0;i < 6;i++)
  {
    if(ThePlayerStats.attributetrains[i] >= 10)
    {
      ThePlayerStats.attributetrains[i] -= 10;
      ThePlayerStats.attributes[i]++;
    }
  }
  ThePlayerStats.speed = ((ThePlayerStats.attributes[AT_Strength] + ThePlayerStats.attributes[AT_Dexterity] + ThePlayerStats.attributes[AT_Agility]) / 3);
  ThePlayerStats.healthmax = ((ThePlayerStats.attributes[AT_Strength] + ThePlayerStats.attributes[AT_Endurance] + ThePlayerStats.attributes[AT_Resistance]) / 3) * 10;
  ThePlayerStats.staminamax = ((ThePlayerStats.attributes[AT_Focus] + ThePlayerStats.attributes[AT_Dexterity] + ThePlayerStats.attributes[AT_Agility]) / 3) * 10;
}


void WakeMonster()
{
  SetupGameMenu();
  Player->asleep = 0;
  Player->sleeptime = 0;
  /*piss him off too*/
}

void ResetStats(int num)
{
  memset(&PlayerStats[num],0,sizeof(SaveInfo));
}
/*returns the dominant element or -1 if neutral.*/
int  DominantElement(int Element[5])
{
  int first = 0;
  int second = 0;
  int i;
  int total = 0;
  int average;
  for(i = 0;i < 5;i++)
  {
    total += Element[i];
    if(Element[i] > Element[first])
    {
      first = i;
    }
  }
  for(i = 0;i < 5;i++)
  {
    if(Element[i] > Element[second])
    {
      if(second != first)
        second = i;
    }
  }
  average = (total / 5);
  if(first < 10)return -1;  /*no element*/
  if(first > (average + (average* 0.1)))
  {
    if(second > (first * 0.9))/*if second is within 10 percent of first*/
    {
      if((abs(second - first) == 1)||(abs(second - first) == 1))
      {/*we have a secondary element*/
        switch(first)
        {
          case E_Earth:
            if(second == E_Water)return E_Ice;
            if(second == E_Lightning)return E_Thunder;
            break;
          case E_Water:
            if(second == E_Earth)return E_Ice;
            if(second == E_Air)return E_Mist;
            break;
          case E_Air:
            if(second == E_Water)return E_Mist;
            if(second == E_Fire)return E_Light;
            break;
          case E_Fire:
            if(second == E_Lightning)return E_Radiation;
            if(second == E_Air)return E_Light;
            break;
          case E_Lightning:
            if(second == E_Fire)return E_Radiation;
            if(second == E_Earth)return E_Thunder;
            break;
        }
      }
      return first;
    }
  }
  return -1;
}

char *GetElementPrefix(int element,int stage)
{
  switch(element)
  {
    case -1:
      switch(stage)
      {
        case 1:
          return "Baby";
        case 2:
          return "Kila";
        case 3:
          return "Mega";
        case 4:
          return "Giga";
        case 5:
          return "Tera";
        default:
          return "-";
      }
    case E_Earth:
      switch(stage)
      {
        case 1:
          return "Rock";
        case 2:
          return "Stone";
        case 3:
          return "Boulder";
        case 4:
          return "Tectonic";
        case 5:
          return "Gaia";
        default:
          return "-";
      }
    case E_Ice:
      switch(stage)
      {
        case 1:
          return "Chilly";
        case 2:
          return "Cold";
        case 3:
          return "Frost";
        case 4:
          return "SubZero";
        case 5:
          return "AbsoluteZero";
        default:
          return "-";
      }
    case E_Water:
      switch(stage)
      {
        case 1:
          return "Bubble";
        case 2:
          return "Spray";
        case 3:
          return "Aqua";
        case 4:
          return "Hydro";
        case 5:
          return "Maelstrom";
        default:
          return "-";
      }
    case E_Mist:
      switch(stage)
      {
        case 1:
          return "Fog";
        case 2:
          return "Vapor";
        case 3:
          return "Gas";
        case 4:
          return "Steam";
        case 5:
          return "Caustic";
        default:
          return "-";
      }
    case E_Air:
      switch(stage)
      {
        case 1:
          return "Breeze";
        case 2:
          return "Gust";
        case 3:
          return "Gail";
        case 4:
          return "Whirlwind";
        case 5:
          return "Cyclone";
        default:
          return "-";
      }
    case E_Light:
      switch(stage)
      {
        case 1:
          return "Glowing";
        case 2:
          return "Bright";
        case 3:
          return "Brilliant";
        case 4:
          return "Shining";
        case 5:
          return "Aurora";
        default:
          return "-";
      }
    case E_Fire:
      switch(stage)
      {
        case 1:
          return "Hot";
        case 2:
          return "Ember";
        case 3:
          return "Burning";
        case 4:
          return "Flaming";
        case 5:
          return "Blazing";
        default:
          return "-";
      }
    case E_Radiation:
      switch(stage)
      {
        case 1:
          return "Micro";
        case 2:
          return "Radio";
        case 3:
          return "Energy";
        case 4:
          return "Nuclear";
        case 5:
          return "Atomic";
        default:
          return "-";
      }
    case E_Lightning:
      switch(stage)
      {
        case 1:
          return "Shock";
        case 2:
          return "Static";
        case 3:
          return "Electric";
        case 4:
          return "Dynamic";
        case 5:
          return "Voltaic";
        default:
          return "-";
      }
    case E_Thunder:
      switch(stage)
      {
        case 1:
          return "Loud";
        case 2:
          return "Acoustic";
        case 3:
          return "Booming";
        case 4:
          return "Shattering";
        case 5:
          return "Supersonic";
        default:
          return "-";
      }
  }
  return NULL;
}
 /*returns -1 for evil, 1 for good, 0 for balance, 2 for not clear.*/
int  DominantAlignment(int good,int evil)
{
  return 1;
}

char *GetAlignmentPrefix(int alignment, int stage)
{
  switch(alignment)
  {
    case 0:  /*balance*/
      switch(stage)
      {
        case 1:
          return "Even";
        case 2:
          return "Level";
        case 3:
          return "Stable";
        case 4:
          return "Grey";
        case 5:
          return "True";
        default:
          return "-";
      }
    case 1:  /*Holy*/
      switch(stage)
      {
        case 1:
          return "Good";
        case 2:
          return "Virtue";
        case 3:
          return "Blessed";
        case 4:
          return "Holy";
        case 5:
          return "Divine";
        default:
          return "-";
      }
    case -1:  /*Shadow*/
      switch(stage)
      {
        case 1:
          return "Dark";
        case 2:
          return "Vile";
        case 3:
          return "Shadow";
        case 4:
          return "Corrupt";
        case 5:
          return "Chaos";
        default:
          return "-";
      }

  }
  return NULL;
}


/*
 * Migration handling
 */

void MigrateTo(char uname[80])
{
  int historyindex;
  Area *area;
  char message[80];
  char oldareaname[80];
  int exitindex;
  exitindex = GetExitIndexByUName(uname);
  if((strlen(CurrentArea->exit_conditions[exitindex]) > 0) && (strncmp(CurrentArea->exit_conditions[exitindex],"none",80) != 0))
  {
    /*check elements here*/
    /*default to checking against skills*/
    if(GetPlayerSkillIndexByName(CurrentArea->exit_conditions[exitindex]) != -1)
    {
      sprintf(message,"Using the %s Skill!",CurrentArea->exit_conditions[exitindex]);
      NewMessage(message,IndexColor(Blue));
      SkillUsed(&PlayerStats[Player->playernum],CurrentArea->exit_conditions[exitindex]);
    }
    else
    {
      sprintf(message,"Need the %s Skill!",CurrentArea->exit_conditions[exitindex]);
      NewMessage(message,IndexColor(Red));
      return;
    }
  }
  strncpy(oldareaname,CurrentArea->uname,80);
  historyindex = GetHistoryIndexByUName(uname,&PlayerStats[Player->playernum]);
  area = GetAreaByUName(uname);
  
  if(historyindex == -1)    /*havn't been here yet!*/
  {
    if(area != NULL)
    {
      AddAreaToHistory(GetAreaByUName(uname), &PlayerStats[Player->playernum]);
      if(area->encounter_level > ThePlayerStats.stage)
      {
        NewMessage("The other Monsters",IndexColor(LightViolet));
        NewMessage("look strong here!",IndexColor(LightViolet));
      }
      if(area->encounter_rate > 1)
      {
        NewMessage("There are a lot of",IndexColor(LightViolet));
        NewMessage("Monsters here!",IndexColor(LightViolet));
      }
      if(area->foodavailable < 6)
      {
        NewMessage("Very little food here!",IndexColor(LightViolet));
      }
      else if(area->foodavailable < 8)
      {
        NewMessage("Not much food here!",IndexColor(LightViolet));
      }
    }
  }
  if(area != NULL)
  {
    LoadArea(uname);
    strncpy(PlayerStats[Player->playernum].areaname,uname,80);
    FindExitByUName(oldareaname);/*can always backtrack*/
    ThePlayerStats.fullness -= 1;
    ThePlayerStats.fatigue += CurrentArea->searchcost;
  }
}



/*
 *  Evolution handling functions
*/

void ReorganizeSkills(SaveInfo *stats)
{
  int swaps = 0;
  char temp[80];
  int i;
  Skill_Info *skilla;
  Skill_Info *skillb;
  do
  {
    swaps = 0;
    for(i = 0;i < (stats->nummovements + stats->numdefenses - 1);i++)
    {
      skilla = GetSkillInfoByName(stats->defenses[i]);
      if(skilla->skilltype == 1)continue; /*the defense is already on top*/
      skillb = GetSkillInfoByName(stats->defenses[i + 1]);
      if(skillb->skilltype == 1)/*we need to swap here*/
      {
        strncpy(temp,stats->defenses[i],80);
        strncpy(stats->defenses[i],stats->defenses[i + 1],80);
        strncpy(stats->defenses[i + 1],temp,80);
        swaps++;
      }
    }
  }while(swaps);
}


/*returns 1 if conflict, 0 otherwise*/
int AddSkill(SaveInfo *stats,char uname[80],int listtype)
{
  int i;
  Skill_Info * skill;
  for(i = 0;i < 8;i++)
  {
    if(listtype == ST_Attacks)
    {
      if(strncmp(stats->attacks[i],uname,80) == 0)
      {
        return 0;/*already got it*/
      }
    }
    else 
    {
      if(strncmp(stats->defenses[i],uname,80) == 0)return 0;/*already got it*/
    }
  }
  for(i = 0;i < 8;i++)
  {
    if(listtype == ST_Attacks)
    {
      if((strlen(stats->attacks[i]) <= 0)||(strncmp(stats->attacks[i],"NULL",80) == 0))
      {
        strncpy(stats->attacks[i],uname,80);
        stats->numattacks++;
        return 0;
      }
    }
    if(listtype == ST_Defenses)
    {
      if((strlen(stats->defenses[i]) <= 0)||(strncmp(stats->defenses[i],"NULL",80) == 0))
      {
        skill = GetSkillInfoByName(uname);
        strncpy(stats->defenses[i],uname,80);
        if(strncmp("Movement",skill->function,80) != 0)
          stats->numdefenses++;
        else stats->nummovements++;
        ReorganizeSkills(stats);
        return 0;
      }
    }
  }
  return 1;
}

void EvolveTo(Entity *self, StageInfo *stage)
{
  int i;
  int conflicts;
  PlayerStats[self->playernum].stage++;
  self->sprite = LoadSwappedSprite(stage->spritename,stage->framew,stage->frameh,stage->colorswaps[0],stage->colorswaps[1],stage->colorswaps[2]);
  strncpy(PlayerStats[self->playernum].stagename,stage->name,80);
  PlayerStats[self->playernum].stageid = stage->id;
  ThePlayerStats.Species = stage->Species;
  ThePlayerStats.Aspect = stage->Aspect;
  for(i = 0;i < 6;i++)
  {
    switch(stage->attributes[i])    /*what ARRR we doing to our stats*/
    {
      case 1:
        PlayerStats[self->playernum].attributes[i] += stage->attributenumbers[i];
        break;
      case 2:
        PlayerStats[self->playernum].attributes[i] += (PlayerStats[self->playernum].attributes[i] * stage->attributenumbers[i]);
        break;
    }
  }
  self->height = stage->height;
  Matriculate();
  PlayerStats[self->playernum].health = PlayerStats[self->playernum].healthmax;
  PlayerStats[self->playernum].stamina = PlayerStats[self->playernum].staminamax;
  conflicts = 0;
  for(i = 0;i < 8;i++)
  {
    if((strlen(stage->attacks[i]) > 0)&&(strncmp(stage->attacks[i],"NULL",80) != 0))
      conflicts += AddSkill(&PlayerStats[self->playernum],stage->attacks[i],ST_Attacks);
  }
  for(i = 0;i < 2;i++)
  {
    if((strlen(stage->defenses[i]) > 0)&&(strncmp(stage->defenses[i],"NULL",80) != 0))
      conflicts += AddSkill(&PlayerStats[self->playernum],stage->defenses[i],ST_Defenses);
  }
  if(conflicts > 0)
  {
    self->skillconflict = 1;
  }
  else self->skillconflict = 0;

}

void EvolutionCheck(Entity *self)
{
  int count = 0;
  int i;
  int failed = 0;
  int targetstage = PlayerStats[self->playernum].stage + 1;
  StageInfo *check = NULL;
  if(PlayerStats[self->playernum].health == PlayerStats[self->playernum].healthmax)
  {
    do
    {
      check = GetNextStageByAge(check,targetstage);
      failed = 0;
      if(check != NULL)
      {
        for(i = 0;i < 8;i++)
        {
          if(strncmp(check->conditions[i],"age",80) == 0)
          {
            if(PlayerStats[self->playernum].Age < check->conditionvalue[i])
            {
              failed = 1;
              break;
            }
          }
          if(strncmp(check->conditions[i],"species",80) == 0)
          {
            if(PlayerStats[self->playernum].Species != check->conditionvalue[i])
            {
              failed = 1;
              break;
            }
          }
          if(strncmp(check->conditions[i],"specific",80) == 0)
          {
            if(PlayerStats[self->playernum].stageid != check->conditionvalue[i])
            {
              failed = 1;
              break;
            }
          }
          if(strncmp(check->conditions[i],"earth",80) == 0)
          {
            if(PlayerStats[self->playernum].Element[E_Earth] < check->conditionvalue[i])
            {
              failed = 1;
              break;
            }
          }
          if(strncmp(check->conditions[i],"water",80) == 0)
          {
            if(PlayerStats[self->playernum].Element[E_Water] < check->conditionvalue[i])
            {
              failed = 1;
              break;
            }
          }
          if(strncmp(check->conditions[i],"air",80) == 0)
          {
            if(PlayerStats[self->playernum].Element[E_Air] < check->conditionvalue[i])
            {
              failed = 1;
              break;
            }
          }
          if(strncmp(check->conditions[i],"fire",80) == 0)
          {
            if(PlayerStats[self->playernum].Element[E_Fire] < check->conditionvalue[i])
            {
              failed = 1;
              break;
            }
          }
          if(strncmp(check->conditions[i],"lightning",80) == 0)
          {
            if(PlayerStats[self->playernum].Element[E_Lightning] < check->conditionvalue[i])
            {
              failed = 1;
              break;
            }
          }
          if(strncmp(check->conditions[i],"strength",80) == 0)
          {
            if(PlayerStats[self->playernum].attributes[AT_Strength] < check->conditionvalue[i])
            {
              failed = 1;
              break;
            }
          }
          if(strncmp(check->conditions[i],"endurance",80) == 0)
          {
            if(PlayerStats[self->playernum].attributes[AT_Endurance] < check->conditionvalue[i])
            {
              failed = 1;
              break;
            }
          }
          if(strncmp(check->conditions[i],"agility",80) == 0)
          {
            if(PlayerStats[self->playernum].attributes[AT_Agility] < check->conditionvalue[i])
            {
              failed = 1;
              break;
            }
          }
          if(strncmp(check->conditions[i],"focus",80) == 0)
          {
            if(PlayerStats[self->playernum].attributes[AT_Focus] < check->conditionvalue[i])
            {
              failed = 1;
              break;
            }
          }
          if(strncmp(check->conditions[i],"dexterity",80) == 0)
          {
            if(PlayerStats[self->playernum].attributes[AT_Dexterity] < check->conditionvalue[i])
            {
              failed = 1;
              break;
            }
          }
          if(strncmp(check->conditions[i],"resistance",80) == 0)
          {
            if(PlayerStats[self->playernum].attributes[AT_Resistance] < check->conditionvalue[i])
            {
              failed = 1;
              break;
            }
          }
        }
        if(!failed)
        {
          SetupGameMenu();
          self->target = SpawnDummyEntity(self);
          self->shown = 0;
          self->updatedelay = 0;
          self->state = MS_Evolving;
          EvolveTo(self, check);
          return;
        }
      }
      count++;
    }
    while(check != NULL);
  }
}


/*
 *
 *      Save Game Handling
 *
 */

int SaveGame(Entity *self)
{
  FILE *file;
  file = fopen("saves/savegame.sav", "wb");
  if(file == NULL)
  {
    fprintf(stderr,"Unable to open file for writing: %s\n",SDL_GetError());
    return -1;
  }
  if(fwrite(&PlayerStats[self->playernum], sizeof(SaveInfo), 1, file)!= 1)
  {
    fprintf(stderr,"Unable to write to file: %s",SDL_GetError());
    return 0;
  }
  fclose(file);
  return 1;
}

int LoadGame(Entity *self)
{
  FILE *file;
  file = fopen("saves/savegame.sav", "rb");
  if(file == NULL)
  {
    fprintf(stderr,"Unable to open file for reading: %s",SDL_GetError());
    return -1;
  }
  if(fread(&PlayerStats[self->playernum], sizeof(SaveInfo), 1, file)!= 1)
  {
    fprintf(stderr,"Unable to read from file: %s",SDL_GetError());
    return 0;
  }
  fclose(file);
  return 1;
}

/*
 *
 *      Stage Info Handling
 *
 */
 
StageInfo *GetNextStageByAge(StageInfo *start,int age)
{
  StageInfo *stage = NULL;
  if(start == NULL)stage = &StageList[0];
  else 
  {
    stage = start;
    stage++;
  }
  while(stage <  &StageList[NumStages])
  {
    if(stage->stage == age)return stage;
    stage++;
  }
  return NULL;
}

StageInfo *GetStageInfoByName(char name[80])
{
  int i;
  for(i = 0;i < NumStages;i++)
  {
    if(strncmp(name,StageList[i].name,80)==0)
    {
      return &StageList[i];
    }
  }
  return NULL;
}

void ClearStageInfo()
{
  memset(StageList,0,sizeof(StageInfo) * MAXSTAGES);
  NumStages = 0;
}

void LoadStageInfoBinary()
{
  FILE *file;
  file = fopen("system/stagelist.stg", "rb");
  if(file == NULL)
  {
    fprintf(stderr,"Unable to open file for reading: %s",SDL_GetError());
    exit(0);
  }
  ClearStageInfo();
  while(!feof(file))
  {
    if(fread(&StageList[NumStages], sizeof(StageInfo), 1, file)!= 1)
    {
      fprintf(stderr,"Unable to read from file: %s",SDL_GetError());
      break;
    }
    NumStages++;
  }
  fclose(file);
}

void SaveStageInfoBinary()
{
  FILE *file;
  int i;
  file = fopen("system/stagelist.stg", "wb");
  if(file == NULL)
  {
    fprintf(stderr,"Unable to open file for writing: %s",SDL_GetError());
    exit(0);
  }
  for(i = 0;i < NumStages;i++)
  {
    if(fwrite(&StageList[i], sizeof(StageInfo), 1, file)!= 1)
    {
      fprintf(stderr,"Unable to write to file: %s",SDL_GetError());
      break;
    }
  }
  fclose(file);
}

void LoadStageInfoText()
{
  int i;
  int temp;
  char buf[128];
  char *c;
  FILE *file;
  file = fopen("system/stagelist.cfg","r");
  if(file == NULL)
  {
    fprintf(stderr,"unable to open stage file!: %s\n",SDL_GetError());
    return;
  }
  memset(StageList,0,sizeof(StageInfo) * MAXSTAGES);
  i = 0;
  while((fscanf(file, "%s", buf) != EOF)&&(i < MAXSTAGES))
  {
    if(strcmp(buf,"#") ==0)
    {
      fgets(buf, sizeof(buf), file);
      continue;/*ignore the rest of the line.*/
    }
    if(strcmp(buf,"end") ==0)
    {
      i++;
      continue;
    }
    if(strcmp(buf,"name") ==0)
    {
      fgetc(file);  /*clear the space before the word*/
      fgets(StageList[i].name, 80, file);
      c = strchr(StageList[i].name, '\n');
      if(c != NULL)
        *c = '\0';    /*replace trailing return with terminating character*/
      continue;
    }
    if(strcmp(buf,"info") ==0)
    {
      fgetc(file);  /*clear the space before the word*/
      fgets(StageList[i].description, 120, file);
      c = strchr(StageList[i].description, '\n');
      if(c != NULL)
        *c = '\0';    /*replace trailing return with terminating character*/
      continue;
    }
    if(strcmp(buf,"stage") ==0)
    {
      fscanf(file, "%i", &StageList[i].stage);
      continue;
    }
    if(strcmp(buf,"idnum") ==0)
    {
      fscanf(file, "%i", &StageList[i].id);
      continue;
    }
    if(strcmp(buf,"aspect") ==0)
    {
      fscanf(file, "%i", &StageList[i].Aspect);
      continue;
    }
    if(strcmp(buf,"species") ==0)
    {
      fscanf(file, "%i", &StageList[i].Species);
      continue;
    }
    if(strcmp(buf,"strength") ==0)
    {
      fscanf(file, "%i %f", &StageList[i].attributes[AT_Strength], &StageList[i].attributenumbers[AT_Strength]);
      continue;
    }
    if(strcmp(buf,"dexterity") ==0)
    {
      fscanf(file, "%i %f", &StageList[i].attributes[AT_Dexterity], &StageList[i].attributenumbers[AT_Dexterity]);
      continue;
    }
    if(strcmp(buf,"focus") ==0)
    {
      fscanf(file, "%i %f", &StageList[i].attributes[AT_Focus],&StageList[i].attributenumbers[AT_Focus]);
      continue;
    }
    if(strcmp(buf,"endurance") ==0)
    {
      fscanf(file, "%i %f", &StageList[i].attributes[AT_Endurance],&StageList[i].attributenumbers[AT_Endurance]);
      continue;
    }
    if(strcmp(buf,"agility") ==0)
    {
      fscanf(file, "%i %f", &StageList[i].attributes[AT_Agility], &StageList[i].attributenumbers[AT_Agility]);
      continue;
    }
    if(strcmp(buf,"resistance") ==0)
    {
      fscanf(file, "%i %f", &StageList[i].attributes[AT_Resistance], &StageList[i].attributenumbers[AT_Resistance]);
      continue;
    }
    if(strcmp(buf,"attack") ==0)
    {
      fscanf(file, "%i", &temp);
      fgetc(file);  /*clear the space before the word*/
      fscanf(file, "%s", StageList[i].attacks[temp]);
      c = strchr(StageList[i].attacks[temp], '\n');
      if(c != NULL)
        *c = '\0';    /*replace trailing return with terminating character*/
      StageList[i].numattacks++;
      continue;
    }
    if(strcmp(buf,"condition") ==0)
    {
      fscanf(file, "%i", &temp);
      fgetc(file);  /*clear the space before the word*/
      fscanf(file, "%i %s",&StageList[i].conditionvalue[temp], StageList[i].conditions[temp]);
      c = strchr(StageList[i].conditions[temp], '\n');
      if(c != NULL)
        *c = '\0';    /*replace trailing return with terminating character*/
      continue;
    }
    if(strcmp(buf,"defense") ==0)
    {
      fscanf(file, "%i", &temp);
      fgetc(file);  /*clear the space before the word*/
      fscanf(file, "%s", StageList[i].defenses[temp]);
      c = strchr(StageList[i].defenses[temp], '\n');
      if(c != NULL)
        *c = '\0';    /*replace trailing return with terminating character*/
      StageList[i].numdefenses++;
      continue;
    }
    if(strcmp(buf,"sprite") ==0)
    {
      fgetc(file);  /*clear the space before the word*/
      fscanf(file, "%s", StageList[i].spritename);
      c = strchr(StageList[i].spritename, '\n');
      if(c != NULL)
        *c = '\0';    /*replace trailing return with terminating character*/
      continue;
    }
    if(strcmp(buf,"framew") ==0)
    {
      fscanf(file, "%i", &StageList[i].framew);
     continue;
    }
    if(strcmp(buf,"frameh") ==0)
    {
      fscanf(file, "%i", &StageList[i].frameh);
      continue;
    }
    if(strcmp(buf,"height") ==0)
    {
      fscanf(file, "%i", &StageList[i].height);
      continue;
    }
    if(strcmp(buf,"soundset") ==0)
    {
      fgetc(file);  /*clear the space before the word*/
      fscanf(file, "%s", StageList[i].soundset);
      c = strchr(StageList[i].soundset, '\n');
      if(c != NULL)
        *c = '\0';    /*replace trailing return with terminating character*/
      continue;
    }
    if(strcmp(buf,"colorswap") ==0)
    {
      fscanf(file, "%i %i %i", &StageList[i].colorswaps[0], &StageList[i].colorswaps[1], &StageList[i].colorswaps[2]);
      continue;
    }
    if(strcmp(buf,"earth") ==0)
    {
      fscanf(file, "%i", &StageList[i].Element[E_Earth]);
      continue;
    }
    if(strcmp(buf,"water") ==0)
    {
      fscanf(file, "%i", &StageList[i].Element[E_Water]);
      continue;
    }
    if(strcmp(buf,"air") ==0)
    {
      fscanf(file, "%i", &StageList[i].Element[E_Air]);
      continue;
    }
    if(strcmp(buf,"fire") ==0)
    {
      fscanf(file, "%i", &StageList[i].Element[E_Fire]);
      continue;
    }
    if(strcmp(buf,"lightning") ==0)
    {
      fscanf(file, "%i", &StageList[i].Element[E_Lightning]);
      continue;
    }
    if(strcmp(buf,"style") ==0)
    {
      if(fscanf(file, "%s", buf) == EOF)
      {
        fclose(file);
        return;
      }
      if(strcmp(buf,"melee") ==0)
      {
        StageList[i].CombatStyle = CS_Melee;
        continue;
      }
      if(strcmp(buf,"speed") ==0)
      {
        StageList[i].CombatStyle = CS_Speed;
        continue;
      }
      if(strcmp(buf,"tank") ==0)
      {
        StageList[i].CombatStyle = CS_Tank;
        continue;
      }
      if(strcmp(buf,"energy") ==0)
      {
        StageList[i].CombatStyle = CS_Energy;
        continue;
      }
      if(strcmp(buf,"effects") ==0)
      {
        StageList[i].CombatStyle = CS_Melee;
        continue;
      }
    }
  }
  fclose(file);
  NumStages = i;
}


void SaveStageInfoText()
{
  int i,j;
  FILE *file;
  file = fopen("system/stagelist.cfg","w");
  if(file == NULL)
  {
    fprintf(stderr,"unable to open stage file!: %s\n",SDL_GetError());
    return;
  }
  fprintf(file,"# This file generated by game.  DO NOT EDIT!\n");
  for(i = 0;i < NumStages;i++)
  {
    fprintf(file,"name %s\n",StageList[i].name);
    fprintf(file,"idnum %i\n",StageList[i].id);
    fprintf(file,"stage %i\n",StageList[i].stage);
    fprintf(file,"aspect %i\n",StageList[i].Aspect);
    fprintf(file,"species %i\n",StageList[i].Species);
    fprintf(file,"style ");
    switch(StageList[i].CombatStyle)
    {
      case 0:
        fprintf(file,"melee\n");
        break;
      case 1:
        fprintf(file,"speed\n");
        break;
      case 2:
        fprintf(file,"tank\n");
        break;
      case 3:
        fprintf(file,"energy\n");
        break;
      case 4:
        fprintf(file,"effects\n");
        break;
    }
    fprintf(file,"strength %i %f\n",StageList[i].attributes[AT_Strength], StageList[i].attributenumbers[AT_Strength]);
    fprintf(file,"dexterity %i %f\n",StageList[i].attributes[AT_Dexterity], StageList[i].attributenumbers[AT_Dexterity]);
    fprintf(file,"focus %i %f\n",StageList[i].attributes[AT_Focus], StageList[i].attributenumbers[AT_Focus]);
    fprintf(file,"endurance %i %f\n",StageList[i].attributes[AT_Endurance], StageList[i].attributenumbers[AT_Endurance]);
    fprintf(file,"agility %i %f\n",StageList[i].attributes[AT_Agility], StageList[i].attributenumbers[AT_Agility]);
    fprintf(file,"resistance %i %f\n",StageList[i].attributes[AT_Resistance], StageList[i].attributenumbers[AT_Resistance]);
    for(j = 0;j < 8;j++)
      fprintf(file,"attack %i %s\n",j,StageList[i].attacks[j]);
    fprintf(file,"defense %i %s\n",0,StageList[i].defenses[0]);
    fprintf(file,"defense %i %s\n",1,StageList[i].defenses[1]);
    fprintf(file,"earth %i\n",StageList[i].Element[E_Earth]);
    fprintf(file,"water %i\n",StageList[i].Element[E_Water]);
    fprintf(file,"air %i\n",StageList[i].Element[E_Air]);
    fprintf(file,"fire %i\n",StageList[i].Element[E_Fire]);
    fprintf(file,"lightning %i\n",StageList[i].Element[E_Lightning]);
    fprintf(file,"info %s\n",StageList[i].description);
    fprintf(file,"sprite %s\n",StageList[i].spritename);
    fprintf(file,"framew %i\n",StageList[i].framew);
    fprintf(file,"frameh %i\n",StageList[i].frameh);
    fprintf(file,"height %i\n",StageList[i].height);
    fprintf(file,"soundset %s\n",StageList[i].soundset);
    fprintf(file,"colorswap %i %i %i\n", StageList[i].colorswaps[0],StageList[i].colorswaps[1],StageList[i].colorswaps[2]);
    for(j = 0;j < 8;j++)
      fprintf(file,"condition %i %i %s\n",j, StageList[i].conditionvalue[j], StageList[i].conditions[j]);
    fprintf(file,"end");
  }
  fclose(file);
}


/*eol @ eof*/

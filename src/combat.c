#include <math.h>
#include "particle.h"
#include "combat.h"
#include "hud.h"
#include "skills.h"
#include "monster.h"
#include "game.h"
#include "net.h"

enum CombatStates {CS_Normal = 1000,CS_Attack1,CS_Attack2,CS_Attack3,CS_Pain,CS_Dead, CS_Happy};
enum CombatButton {CB_Attacks, CB_Recover, CB_Defense, CB_Items, CB_Info, CB_Flee, CB_ListStart};
enum CombatTypes  {CT_Player,CT_Client,CT_AI,CT_Net,CT_Server};

extern const Uint32 MajorVersion;
extern const Uint32 MinorVersion;
extern Entity *Player;
extern HUDInfo hud;
extern SaveInfo PlayerStats[MAXPLAYERS];
extern SDL_Surface *screen;
extern Sprite *attributes;
extern SDL_Rect Camera;
extern Sprite *buttonarrows[3];
extern int IsServer;
int Netmode = 0;
int CombatMode = 0;
int CombatOver = 0;

Combat_Info Combatant[2];
FrameData framedata[2]; /*for net syncing*/

void CombatThink(Entity *self);
void CombatNetThink(Entity *self);
void CombatAIThink(Entity *self);
void CombatUpdate(Entity *self);
void SetupCombatMenu();
void SetupAttackMenu();
void SetupDefenseMenu();
void DoCombat(Entity *attacker,Entity *defender);
void DoCombatViaNet();
void ReceiveMessage();
void SendMessage(char message[80],Uint32 color);/*calls new message on the other side*/
void NetInfoToSaveInfo(SaveInfo *stats,NetStatInfo *netstats);
void SaveInfoToNetInfo(SaveInfo *stats,NetStatInfo *netstats);
void DoNetAttack();


Entity *SpawnEnounterMonster(char uname[80],int level,int AI)
{
  int i;
  Entity *self;
  int playernum = 1;
  StageInfo *stage = NULL;
  self = NewEntity();
  if(self == NULL)return NULL;
  self->playernum = 1;    /*hardcoding this player as player 2*/
  switch(AI)
  {
    case 0:
      /*handling of net info here*/
      fprintf(stdout,"monster stage to load: %s\n",uname);
      stage = GetStageInfoByName(uname);
      if(stage == NULL)
      {
        fprintf(stdout,"monster dater corrupt\n");
        NewMessage("Invalid Monster",IndexColor(LightViolet));
        NewMessage("Net error",IndexColor(LightViolet));
        CombatEnd();
        return NULL;
      }
      self->sprite = LoadSprite(stage->spritename,stage->framew,stage->frameh);
      if(!IsServer)
        self->think = CombatNetThink;
      else self->think = CombatThink;
      fprintf(stdout,"get here too: 74\n");
      break;
    case 1:
      ResetStats(1);
      stage = GetStageInfoByName(uname);
      if(stage == NULL)
      {
        stage = GetStageInfoByName("dorumon");
        if(stage == NULL)
        {
          fprintf(stderr,"unable to find stage information for monster!\n");
          exit(0);
        }
      }
      self->sprite = LoadSprite(stage->spritename,stage->framew,stage->frameh);
      strncpy(PlayerStats[playernum].stagename,stage->name,80);
      for(i = 0;i < 6;i++)
      {
        PlayerStats[playernum].attributes[i] = (10 * (level + 1)) + (random()*5);
        if(stage->attributes[i] == 1)
          PlayerStats[playernum].attributes[i] += stage->attributenumbers[i];
        else if(stage->attributes[i] == 2)
          PlayerStats[playernum].attributes[i] += PlayerStats[playernum].attributes[i] * (stage->attributenumbers[i] * stage->stage);
      }
      
      PlayerStats[playernum].fullness = stage->stage * 20;
      
      PlayerStats[playernum].healthmax = ((PlayerStats[playernum].attributes[AT_Strength] + PlayerStats[playernum].attributes[AT_Endurance] + PlayerStats[playernum].attributes[AT_Resistance]) / 3) * 10;
      PlayerStats[playernum].staminamax = ((PlayerStats[playernum].attributes[AT_Focus] + PlayerStats[playernum].attributes[AT_Dexterity] + PlayerStats[playernum].attributes[AT_Agility]) / 3) * 10;
      PlayerStats[playernum].health = PlayerStats[playernum].healthmax;
      PlayerStats[playernum].stamina = PlayerStats[playernum].staminamax;
      PlayerStats[playernum].speed = ((PlayerStats[playernum].attributes[AT_Strength] + PlayerStats[playernum].attributes[AT_Dexterity] + PlayerStats[playernum].attributes[AT_Agility]) / 3);
      
      for(i = 0;i < 5;i++)
      {
        PlayerStats[playernum].Element[i] = stage->Element[i] * (10 * level);
      }
          
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
      self->think = CombatAIThink;
      break;
  }
  self->shown = 1;
  self->shadow = 4;
  self->height = stage->height;
  self->s.y = 200;
  self->s.x = 384;
  self->xflip = 1;
  self->frame = 0;
  self->update = CombatUpdate;
  self->NextUpdate = 0;
  self->UpdateRate = 240;
  self->NextThink = 0;
  self->ThinkRate = 80;
  self->state = CS_Normal;
  self->frdir = 1;
  self->playernum = 1;
  self->combatindex = 1;
  return self;
}

void CombatBeginEncounter(char uname[80],int level)
{
  int averagespeed;
  Combatant[0].Combatant = Player;
  Combatant[0].Type = CT_Player;
  Combatant[0].AttackCooldown = 0;
  Combatant[0].DefenseCooldown = 0;
  Combatant[0].ChosenAttack = -1;
  Combatant[0].ChosenDefense = -1;
  Player->combatindex = 0;
  Player->update = CombatUpdate;
  Player->think = CombatThink;
  Player->s.x = 96;
  SetCameraTarget(239,0, 2,5);
  CombatMode = 1;
  CombatOver = 0;
  SetupCombatMenu();
  Combatant[1].Combatant = SpawnEnounterMonster(uname,level,1);
  fprintf(stdout,"monster: %s\n",uname);
  Combatant[1].Type = CT_AI;
  Combatant[1].AttackCooldown = 0;
  Combatant[1].DefenseCooldown = 0;
  Combatant[1].ChosenAttack = -1;
  Combatant[1].ChosenDefense = -1;
  Combatant[1].Combatant->decisiondelay = 100 - (PlayerStats[Combatant[1].Combatant->playernum].stage * 15);
  averagespeed = (PlayerStats[Combatant[1].Combatant->playernum].speed +  PlayerStats[Combatant[0].Combatant->playernum].speed)/2;
  if(averagespeed != 0)
  {
    Combatant[1].realtivespeed = PlayerStats[Combatant[1].Combatant->playernum].speed / averagespeed;
    Combatant[0].realtivespeed = PlayerStats[Combatant[0].Combatant->playernum].speed / averagespeed;
  }
  if(Combatant[0].realtivespeed <= 0)Combatant[0].realtivespeed = 1;
  if(Combatant[1].realtivespeed <= 0)Combatant[1].realtivespeed = 1;
}

void CombatBeginNet(int host)
{
  int averagespeed;
  char message[80];
  char bugger[80];
  int rec;
  int check = 0;
  static NetStatInfo netstats;
  Netmode = 1;
  if(host)
  {
    IsServer = 1;
    /*exchange version numbers, if out of sync, give error messages and return to game*/
    fprintf(stdout,"Here 1\n");
    do
    {
      rec = PassiveReceive(bugger,sizeof(char) * 80);
      if(rec == -1)
      {
        NewMessage("Connection Lost",IndexColor(Red));
        CombatEnd();
        return;
      }
      if((rec)&&(bugger[0] == 'v'))
      {
        sprintf(message,"v %i",MajorVersion);
        if(strncmp(bugger,message,80) == 0)
        {
          check++;
        }
        else
        {
          NewMessage("Version Mismatch",IndexColor(LightRed));
          CombatEnd();
          return;
        }
      }
      if((rec)&&(bugger[0] == '.'))
      {
        sprintf(message,". %i",MinorVersion);
        if(strncmp(bugger,message,80) == 0)
        {
          check++;
        }
        else
        {
          NewMessage("Version Mismatch",IndexColor(LightRed));
          CombatEnd();
          return;
        }
      }
    }while(check < 2);
    fprintf(stdout,"Here 2\n");

    /*Get Monster Info from client*/
    check = 0;
    while(!check)
    {
      rec = PassiveReceive(&netstats,sizeof(NetStatInfo));
      if(rec == -1)
      {
        NewMessage("Connection Lost",IndexColor(Red));
        CombatEnd();
        return;
      }
      if(rec > 0)check = 1;
    }
    fprintf(stdout,"Here 3\n");
    fprintf(stdout,"Client monster name: %s\n",netstats.stagename);
    NetInfoToSaveInfo(&PlayerStats[1],&netstats);
    fprintf(stdout,"Client monster name: %s\n",PlayerStats[1].stagename);
    /*Send Monster Draw Info to client*/
    fprintf(stdout,"My monster name: %s\n",PlayerStats[0].stagename);
    SaveInfoToNetInfo(&PlayerStats[0],&netstats);
    fprintf(stdout,"My monster name: %s\n",PlayerStats[0].stagename);
    Send(&netstats,sizeof(NetStatInfo));
    fprintf(stdout,"Here 4\n");

    /*set up Player 0 like normal*/
    Combatant[0].Combatant = Player;
    Combatant[0].Type = CT_Player;
    Combatant[0].AttackCooldown = 20;
    Combatant[0].DefenseCooldown = 20;
    Combatant[0].ChosenAttack = -1;
    Combatant[0].ChosenDefense = -1;
    Player->combatindex = 0;
    Player->update = CombatUpdate;
    Player->think = CombatThink;
    Player->s.x = 96;
    SetCameraTarget(239,0, 2,5);/*the 'other' is always on the right*/
    CombatMode = 1;
    CombatOver = 0;

    /*set up player 1 to thinkNet*/
    Combatant[1].Combatant = SpawnEnounterMonster(PlayerStats[1].stagename,0,0);
    fprintf(stdout,"Got here last\n");
    Combatant[1].Type = CT_Net;
    Combatant[1].AttackCooldown = 20;
    Combatant[1].DefenseCooldown = 20;
    Combatant[1].ChosenAttack = -1;
    Combatant[1].ChosenDefense = -1;
    averagespeed = (PlayerStats[Combatant[1].Combatant->playernum].speed +  PlayerStats[Combatant[0].Combatant->playernum].speed)/2;
    if(averagespeed != 0)
    {
      Combatant[1].realtivespeed = PlayerStats[Combatant[1].Combatant->playernum].speed / averagespeed;
      Combatant[0].realtivespeed = PlayerStats[Combatant[0].Combatant->playernum].speed / averagespeed;
    }
    if(Combatant[0].realtivespeed <= 0)Combatant[0].realtivespeed = 1;
    if(Combatant[1].realtivespeed <= 0)Combatant[1].realtivespeed = 1;

  }
  else
  {
    fprintf(stdout,"Here 1\n");
    IsServer = 0;
    
    /*exchange version numbers, if out of sync, give error messages and return to game*/
    sprintf(message,"v %i",MajorVersion);
    Send(message,sizeof(message));
    
    fprintf(stdout,"Here 2\n");
    sprintf(message,". %i",MinorVersion);
    Send(message,sizeof(message));
    fprintf(stdout,"Here 3\n");
    /*Send Monster Info to host*/
    SaveInfoToNetInfo(&PlayerStats[0],&netstats);
    Send(&netstats,sizeof(NetStatInfo));
    fprintf(stdout,"Here 4\n");
    /*Get Monster Draw Info from host*/
    check = 0;
    while(!check)
    {
      rec = PassiveReceive(&netstats,sizeof(NetStatInfo));
      if(rec == -1)
      {
        NewMessage("Connection Lost",IndexColor(Red));
        CombatEnd();
        return;
      }
      if(rec > 0)check = 1;
    }
    NetInfoToSaveInfo(&PlayerStats[1],&netstats);
    fprintf(stdout,"server monster's stage: %s\n",PlayerStats[1].stagename);
    fprintf(stdout,"server health : %i / %i", (int)PlayerStats[1].health, PlayerStats[1].healthmax);
    /*Send Monster Draw Info to client*/
    fprintf(stdout,"Here 5\n");
    /*set up player 1 to thinkLocalNet*/
    Combatant[0].Combatant = Player;
    Combatant[0].Type = CT_Client;
    Combatant[0].AttackCooldown = 20;
    Combatant[0].DefenseCooldown = 20;
    Combatant[0].ChosenAttack = -1;
    Combatant[0].ChosenDefense = -1;
    Player->combatindex = 0;
    Player->update = NULL;
    Player->think = NULL; /*the dumby think that won't actually do anything.*/
    Player->s.x = 96;
    SetCameraTarget(239,0, 2,5);
    CombatMode = 1;
    CombatOver = 0;

    /*set up player 0 to thinkNet*/
    Combatant[1].Combatant = SpawnEnounterMonster(PlayerStats[1].stagename,0,0);
    fprintf(stdout,"Got here last\n");
    Combatant[1].Combatant->update = NULL;/*we don't actually do anything if we're the client*/
    Combatant[1].Combatant->think = NULL;
    Combatant[1].Type = CT_Server;
    Combatant[1].AttackCooldown = 20;
    Combatant[1].DefenseCooldown = 20;
    Combatant[1].ChosenAttack = -1;
    Combatant[1].ChosenDefense = -1;
    averagespeed = (PlayerStats[Combatant[1].Combatant->playernum].speed +  PlayerStats[Combatant[0].Combatant->playernum].speed)/2;
    if(averagespeed != 0)
    {
      Combatant[1].realtivespeed = PlayerStats[Combatant[1].Combatant->playernum].speed / averagespeed;
      Combatant[0].realtivespeed = PlayerStats[Combatant[0].Combatant->playernum].speed / averagespeed;
    }
    if(Combatant[0].realtivespeed <= 0)Combatant[0].realtivespeed = 1;
    if(Combatant[1].realtivespeed <= 0)Combatant[1].realtivespeed = 1;

  }
  SetupCombatMenu();
}

void SendCameraFocus(Sint32 tx,Sint32 ty,Uint32 framestotarget,Uint32 framestohold)
{
  char message[80];
  sprintf(message,"camerafocus");
  Send(message,sizeof(message));
  /*auto swap sides for the other side*/
  tx -= 120;
  tx *= -1;
  tx += 120;
  Send(&tx,sizeof(Sint32));
  Send(&ty,sizeof(Sint32));
  Send(&framestotarget,sizeof(Uint32));
  Send(&framestohold,sizeof(Uint32));
}

void ReveiceCameraFocus()
{
  Sint32 tx,ty;
  Uint32 target,hold;
  HardReceive(&tx,sizeof(Sint32));
  HardReceive(&ty,sizeof(Sint32));
  HardReceive(&target,sizeof(Uint32));
  HardReceive(&hold,sizeof(Uint32));
  SetCameraTarget(tx,ty, target,hold);
}

void SendFrameData()
{
  char message[80];
  framedata[1].Frame = Player->frame;
  framedata[1].State = Player->state;
  framedata[0].Frame = Combatant[1].Combatant->frame;
  framedata[0].State = Combatant[1].Combatant->state;
  sprintf(message,"framedata");
  Send(message,sizeof(message));
  Send(framedata,sizeof(FrameData) * 2);
}

void ReceiveFrameData()
{
  HardReceive(framedata,sizeof(FrameData) * 2);
  Combatant[1].Combatant->frame = framedata[1].Frame;
  Combatant[1].Combatant->state = framedata[1].State;
  Combatant[0].Combatant->frame = framedata[0].Frame;
  Combatant[0].Combatant->state = framedata[0].State;
}

void SendNetCombatBegin()
{
  char message[80];
  sprintf(message,"netcombatbegin");
  Send(message,strlen(message) + 1);
}

void UpdateCombatMode()
{
  int rec;
  char buffer[80];
  NetStatInfo netstats;
  if(CombatOver)
  {
    CombatOver++;
    if(CombatOver >= 200)
    {
      if(Player->state == CS_Happy)Player->state = MS_Happy;
      CombatEnd();
    }
    return;
  }
  /*poll for net activity and update accordingly*/
  if(Netmode)
  {
    if(IsServer)
    {
      SendFrameData();
    }
    do
    {
      rec = PassiveReceive(buffer,sizeof(char) * 80);
      if(rec == -1)
      {
        CombatEnd();
        return;
      }
      /*parse info*/
      if(strncmp(buffer,"textmessage",80)==0)
      {
        ReceiveMessage();
        fprintf(stdout,"text message received\n");
        continue;
      }
      if(strncmp(buffer,"camerafocus",80)==0)
      {
        ReveiceCameraFocus();
        fprintf(stdout,"camera focus received\n");
        continue;
      }
      if(strncmp(buffer,"netattack",80)==0)
      {
        DoNetAttack();
        fprintf(stdout,"attack command received\n");
        continue;
      }
      if(strncmp(buffer,"endcombat",80)==0)
      {
        Netmode = 0;
        CombatEnd();
        fprintf(stdout,"combat over received\n");
        return;
      }
      if(strncmp(buffer,"framedata",80)==0)
      {
        ReceiveFrameData();
        fprintf(stdout,"frame dater received\n");
        continue;
      }
      if(strncmp(buffer,"yourattackcooldown",80)==0)
      {
        HardReceive(&Combatant[0].AttackCooldown,sizeof(Uint32));
        fprintf(stdout,"attack cooldown received\n");
        continue;
      }
      if(strncmp(buffer,"yourdefensecooldown",80)==0)
      {
        HardReceive(&Combatant[0].DefenseCooldown,sizeof(Uint32));
        fprintf(stdout,"defense cooldown received\n");
        continue;
      }
      if(strncmp(buffer,"myattackset",80)==0)
      {
        HardReceive(&Combatant[1].ChosenAttack,sizeof(Uint32));
        fprintf(stdout,"attack chosen received\n");
        continue;
      }
      if(strncmp(buffer,"mydefenseset",80)==0)
      {
        HardReceive(&Combatant[1].ChosenDefense,sizeof(Uint32));
        fprintf(stdout,"defense chosen received\n");
        continue;
      }
      if(strncmp(buffer,"yourstats",80)==0)
      {   /*meaning MY stats updated here*/
        /*First we receive the converted data structure and then convert back*/
        HardReceive(&netstats,sizeof(NetStatInfo));
        NetInfoToSaveInfo(&PlayerStats[0],&netstats);
        fprintf(stdout,"my stats received\n");
        continue;
      }
      if(strncmp(buffer,"mystats",80)==0)
      {   /*meaning THEIR stats updated here*/
        HardReceive(&netstats,sizeof(NetStatInfo));
        NetInfoToSaveInfo(&PlayerStats[1],&netstats);
        fprintf(stdout,"your stats received\n");
        continue;
      }
      if(strncmp(buffer,"netcombatbegin",80)==0)
      {
        DoCombatViaNet();
        fprintf(stdout,"combat begin\n");
        return;
      }
    }while(rec);
  }
}


/*
  Called when combat is over to reset everything back to normal
*/

void CombatEnd()
{
  char message[80];
  Player->update = MonsterUpdate;
  Player->think = MonsterThink;
  FreeEntity(Combatant[1].Combatant);
  Player->s.x = 240;
  Camera.x = 120;
  CombatMode = 0;
  SetupGameMenu();
  if(Netmode)
  {
    sprintf(message,"endcombat");
    Send(message,sizeof(message));
    Netmode = 0;
  }
  /*send signal to other signifying end of combat*/
  
}

void SaveInfoToNetInfo(SaveInfo *stats,NetStatInfo *netstats)
{
  strncpy(netstats->name,stats->name,80);                    /*monster's nickname*/
  strncpy(netstats->stagename,stats->stagename,80);
  netstats->stageid = stats->stageid;                      /*the unique number for the stage*/
  netstats->Aspect = stats->Aspect;
  netstats->Species = stats->Species;
  netstats->good = stats->good;
  netstats->evil = stats->evil;
  memcpy(netstats->Element,stats->Element,sizeof(float) * 5);                    /*elements*/
  memcpy(netstats->attributes,stats->attributes,sizeof(int) * 6);                    /*attr*/
  memcpy(netstats->attributetrains,stats->attributetrains,sizeof(int) * 6);  /*attr*/
  netstats->numdefenses = stats->numdefenses;
  netstats->numattacks = stats->numattacks;
  memcpy(netstats->attacks,stats->attacks,sizeof(char) * 8 * 80);  /*attacks*/
  memcpy(netstats->defenses,stats->defenses,sizeof(char) * 8 * 80);  /*attacks*/
  netstats->nummovements = stats->nummovements;
  memcpy(netstats->Effects,stats->Effects,sizeof(int) * 10);  /*status effects*/
  netstats->health = stats->health;
  netstats->healthmax = stats->healthmax;
  netstats->stamina = stats->stamina;
  netstats->staminamax = stats->staminamax;
  netstats->speed = stats->speed;
  netstats->fullness = stats->fullness;
  netstats->fatigue = stats->fatigue;
  memcpy(netstats->inventory,stats->inventory,sizeof(int) * MAXITEMS);  /*items*/
}

void NetInfoToSaveInfo(SaveInfo *netstats,NetStatInfo *stats)
{                     /*saved on rewriting by just swapping the names of the variables*/
  strncpy(netstats->name,stats->name,80);                    /*monster's nickname*/
  strncpy(netstats->stagename,stats->stagename,80);
  netstats->stageid = stats->stageid;                      /*the unique number for the stage*/
  netstats->Aspect = stats->Aspect;
  netstats->Species = stats->Species;
  netstats->good = stats->good;
  netstats->evil = stats->evil;
  memcpy(netstats->Element,stats->Element,sizeof(float) * 5);                    /*elements*/
  memcpy(netstats->attributes,stats->attributes,sizeof(int) * 6);                    /*attr*/
  memcpy(netstats->attributetrains,stats->attributetrains,sizeof(int) * 6);  /*attr*/
  netstats->numdefenses = stats->numdefenses;
  netstats->numattacks = stats->numattacks;
  memcpy(netstats->attacks,stats->attacks,sizeof(char) * 8 * 80);  /*attacks*/
  memcpy(netstats->defenses,stats->defenses,sizeof(char) * 8 * 80);  /*attacks*/
  netstats->nummovements = stats->nummovements;
  memcpy(netstats->Effects,stats->Effects,sizeof(int) * 10);  /*status effects*/
  netstats->health = stats->health;
  netstats->healthmax = stats->healthmax;
  netstats->stamina = stats->stamina;
  netstats->staminamax = stats->staminamax;
  netstats->speed = stats->speed;
  netstats->fullness = stats->fullness;
  netstats->fatigue = stats->fatigue;
  memcpy(netstats->inventory,stats->inventory,sizeof(int) * MAXITEMS);  /*items*/
}

void SendStatUpdate(Entity *self)
{
  char message[80];
  NetStatInfo netstats;
  if(!self->playernum)/*my stats*/
  {
    sprintf(message,"mystats");
    Send(message,sizeof(message));
    SaveInfoToNetInfo(&PlayerStats[self->playernum],&netstats);
    /*First we convert our info to a small enough data structure and then transmit*/
    Send(&netstats,sizeof(NetStatInfo));
  }
  else
  {
    sprintf(message,"yourstats");
    Send(message,sizeof(message));
    SaveInfoToNetInfo(&PlayerStats[self->playernum],&netstats);
    /*First we convert our info to a small enough data structure and then transmit*/
    Send(&netstats,sizeof(NetStatInfo));
  }
}

void SendAttackCooldown(Entity *self)
{
  char message[80];
  sprintf(message,"yourattackcooldown");
  Send(message,sizeof(message));
  Send(&Combatant[self->combatindex].AttackCooldown,sizeof(Uint32));
}

void SendDefenseCooldown(Entity *self)
{
  char message[80];
  sprintf(message,"yourdefensecooldown");
  Send(message,sizeof(message));
  Send(&Combatant[self->combatindex].DefenseCooldown,sizeof(Uint32));
}

void SendAttackUpdate(Entity *self)
{
  char message[80];
  sprintf(message,"myattackset");
  Send(message,sizeof(message));
  Send(&Combatant[self->combatindex].ChosenAttack,sizeof(Uint32));
}

void SendDefenseUpdate(Entity *self)
{
  char message[80];
  sprintf(message,"mydefenseset");
  Send(message,sizeof(message));
  Send(&Combatant[self->combatindex].ChosenDefense,sizeof(Uint32));
}


void CombatThink(Entity *self)
{
  Skill_Info *skill;
  if(CombatOver)return;
  if(self->state == CS_Normal)
  {
    if(Combatant[self->combatindex].AttackCooldown <= 0)
    {
      if(Combatant[self->combatindex].ChosenAttack == -2)
      {
        NewMessage("Recovering Stamina",IndexColor(Cyan));
        Combatant[self->combatindex].AttackCooldown = (PlayerStats[self->playernum].staminamax * 0.05);
        PlayerStats[self->playernum].fatigue += 2;
        PlayerStats[self->playernum].stamina += (PlayerStats[self->playernum].staminamax * 0.05);
        if(PlayerStats[self->playernum].stamina > PlayerStats[self->playernum].staminamax) PlayerStats[self->playernum].stamina = PlayerStats[self->playernum].staminamax;
        Combatant[self->combatindex].ChosenAttack = -1;
        /*Send Update to Client - done*/
        if(Netmode)
        {
          SendMessage("Recovering Stamina",IndexColor(Cyan));
          SendStatUpdate(self);
          SendAttackCooldown(self);
        }
      }
      if(Combatant[self->combatindex].ChosenAttack > -1)
      {
        fprintf(stdout,"attack chosen : %i\n",Combatant[self->combatindex].ChosenAttack);
        skill = GetSkillInfoByName(PlayerStats[self->playernum].attacks[Combatant[self->combatindex].ChosenAttack]);
        if(skill == NULL)
        {
          fprintf(stdout,"skill not found\n");
          return;
        }
        if(PlayerStats[self->playernum].stamina < skill->staminacost)
        {
          if(self->combatindex == 0)
          {
            NewMessage("Not enough Stamina!",IndexColor(Cyan));
          }
          else if(Netmode)SendMessage("Not enough Stamina!",IndexColor(Cyan));
          Combatant[self->combatindex].ChosenAttack = -1;
          return;
        }
        if(PlayerStats[self->playernum].health >= skill->healthcost)
        {
          PlayerStats[self->playernum].health -= skill->healthcost;
          PlayerStats[self->playernum].stamina -= skill->staminacost;
        }
        else
        {
          if(self->combatindex == 0)
            NewMessage("Not enough Health!",IndexColor(Magenta));
          else if(Netmode)SendMessage("Not enough Health!",IndexColor(Cyan));
          Combatant[self->combatindex].ChosenAttack = -1;
          return;
        }
        PlayerStats[self->playernum].fullness -= 0.2;
        if(Netmode)SendStatUpdate(self);
        self->state = CS_Attack1 + skill->animation;
        self->frame = 8 + (skill->animation * 2);
        self->fcount = 4;
        SetCameraTarget((self->combatindex * 239),0, 2,1);
        if(Netmode)SendCameraFocus((self->combatindex * 239),0, 2,1);
      }
    }
  }
}

void CombatNetThink(Entity *self)
{
  
}


void CombatAIThink(Entity *self)
{
  if(CombatOver)return;
  if(self->decisiondelay <= 0)
  {
    if(PlayerStats[self->playernum].health < (PlayerStats[self->playernum].healthmax / 4))
    {
      if(crandom() > 0.3)
      {
        ThePlayerStats.wins++;
        self->shown = 0;
        NewMessage("It Ran away!",IndexColor(Cyan));
        SetupCombatMenu();
        CombatOver = 1;
        Player->state = CS_Happy;
        SetCameraTarget(239,0, 2,5);
        return;
      }
    }
    self->decisiondelay = 100 - (PlayerStats[self->playernum].stage * 15);
    Combatant[self->combatindex].ChosenAttack = (random() * (PlayerStats[self->playernum].numattacks - 1));
    if(Combatant[self->combatindex].ChosenAttack < -1)Combatant[self->combatindex].ChosenAttack = -1;
    if(PlayerStats[self->playernum].stamina < (PlayerStats[self->playernum].staminamax / 8))Combatant[self->combatindex].ChosenAttack = -2;
    if(PlayerStats[self->playernum].numdefenses > 0)
      Combatant[self->combatindex].ChosenDefense = (random() * (PlayerStats[self->playernum].numdefenses - 1));
  }else self->decisiondelay--;
  CombatThink(self);
}

void CombatUpdate(Entity *self)
{
  Skill_Info *skill;
  if(Combatant[self->combatindex].AttackCooldown > 0)
  {
    Combatant[self->combatindex].AttackCooldown -= Combatant[self->combatindex].realtivespeed;
  }
  if(Combatant[self->combatindex].DefenseCooldown > 0)
  {
    Combatant[self->combatindex].DefenseCooldown -= Combatant[self->combatindex].realtivespeed;
  }
  if((Netmode)&&(self->combatindex))/*if this is the client*/
  {
    SendAttackCooldown(self);
    SendDefenseCooldown(self);
  }
      /*animation handling*/
  switch(self->state)
  {
    case CS_Happy:
      if(self->fcount <= 0)
      {
        self->frame++;
        self->fcount = 4;
        self->state = MS_Normal;
      }
      else self->fcount--;
      break;
    case CS_Normal: /*for now , use normal idle*/
      if(self->fcount <= 0)
      {
        self->fcount = 2;
        if((random()*2) > 0.2)self->frame+= self->frdir;
        if(self->frame == 0)
        {
          self->frdir = 1;
        }
        if(self->frame == 2)
        {
          self->frdir = -1;
        }
        if((self->frame > 2)||(self->frame < 0))self->frame = 1;
      }
      else self->fcount--;
      break;
    case CS_Attack1:
    case CS_Attack2:
    case CS_Attack3:
      if(self->fcount <= 0)
      {
        self->frame++;
        self->fcount = 4;
        self->state = CS_Normal;
        /* Call Attack Function*/
        skill = GetSkillInfoByName(PlayerStats[self->playernum].attacks[Combatant[self->combatindex].ChosenAttack]);
        Combatant[self->combatindex].AttackCooldown = skill->cooldown;
        SkillUsed(&PlayerStats[self->playernum],skill->uname);
        DoCombat(self,Combatant[!self->combatindex].Combatant);
        Combatant[self->combatindex].ChosenAttack = -1;
        NewMessage("DONE",IndexColor(LightRed));
      }
      else self->fcount--;
      break;
    case CS_Pain:
        self->frame = 14;
        self->fcount = 4;
        self->state = CS_Normal;
      break;
    case CS_Dead:
      self->frame = 15;
      break;
    default:
      self->state = CS_Normal;
  }
}

int  SkillRoll()
{
  int total = 0;
  int roll = 0;
  do
  {
    roll = (int)(random() * 10);
    total += roll;
  }while (roll == 10);
  if(roll == 1)total -= (int)(random() * 10);
  return total;
}

/*
  Combat animation
*/

void SendMessage(char message[80],Uint32 color)
{
  char buffer[80];
  sprintf(buffer,"textmessage");
  Send(buffer,sizeof(buffer));
  Send(message,sizeof(message));
  Send(&color,sizeof(Uint32));
}

void ReceiveMessage()
{
  char message[80];
  Uint32 color;
  HardReceive(message,sizeof(message));
  HardReceive(&color,sizeof(Uint32));
  NewMessage(message,color);
}

void DoCombatViaNet()
{
  int done = 0;
  int frames = 0;
  char buffer[80];
  SetupCombatMenu();/*close any open windows for the attack*/
  while(!done)
  {
    ResetBuffer();
    SDL_PumpEvents();
    Draw_ALL();
    UpdateCamera();
    /*send frame data*/
    if(Netmode)
    {
      PassiveReceive(buffer,sizeof(char) * 80);
      if(strncmp(buffer,"framedata",80)==0)
        SendFrameData();
    }
    NextFrame();
    /*update attack animation*/
    if(frames++ >= 15)done = 1;
  }
  /*Display Attack Message*/
  /*spawn particles from the net*/
  /*do animation loop*/
  /*update stats information*/
}

void DoNetAttack()
{
  Skill_Info *a_skill;
  char message[80];
  Sint32 sx,pdir;
  HardReceive(message,sizeof(message));
  HardReceive(&sx,sizeof(Sint32));
  HardReceive(&pdir,sizeof(Sint32));
  a_skill = GetSkillInfoByName(message);
  if(pdir == -1)
  SpawnSwapFlipSpriteParticle(a_skill->sprite,a_skill->sw,a_skill->sh,8,a_skill->colorswaps[0], a_skill->colorswaps[1], a_skill->colorswaps[2],sx,160,(16 * pdir),0,1,0,15);
  else   SpawnSwapSpriteParticle(a_skill->sprite,a_skill->sw,a_skill->sh,8,a_skill->colorswaps[0], a_skill->colorswaps[1], a_skill->colorswaps[2],sx,160,(16 * pdir),0,15);
}

void SendNetAttack(char attack[80],Sint32 sx,Sint32 pdir)
{
  char message[80];
  sprintf(message,"netattack");
  Send(message,sizeof(message));
  Send(attack,sizeof(attack));
  Send(&sx,sizeof(Sint32));
  Send(&pdir,sizeof(Sint32));
}


void DoCombat(Entity *attacker,Entity *defender)
{
  int i;
  char message[80];
  int hitscore,dodgescore;
  int damage,soak;
  int roll;
  int done = 0;
  int frames = 0;
  int prim,sec;
  int bonus;
  int crit = 0;
  int pdir = 1;
  int attackindex;
  float holydmg;
  int elemental = 0;
  float elements[5] = {0,0,0,0,0};
  float delements[5] = {0,0,0,0,0};
  Skill_Info *a_skill = NULL;
  Skill_Info *d_skill = NULL;
  SetupCombatMenu();/*close any open windows for the attack*/
  SetCameraTarget((defender->combatindex * 239),0, 10,5);
  if(Netmode)SendCameraFocus((defender->combatindex * 239),0, 10,5);
  if(attacker->combatindex == 0)pdir = 1;
  else pdir = -1;
  attackindex = Combatant[attacker->combatindex].ChosenAttack;
  /*get attack skill*/
  a_skill = GetSkillInfoByName(PlayerStats[attacker->playernum].attacks[attackindex]);
  NewMessage(a_skill->skillname,IndexColor(LightYellow));
  if(Netmode)SendMessage(a_skill->skillname,IndexColor(LightYellow));
  if(strlen(a_skill->sprite) > 0)
  {
    if(pdir == -1)
    {
      SpawnSwapFlipSpriteParticle(a_skill->sprite,a_skill->sw,a_skill->sh,8,a_skill->colorswaps[0], a_skill->colorswaps[1], a_skill->colorswaps[2],attacker->s.x + (64 * pdir),160,(16 * pdir),0,1,0,15);
      /*if net, send net info for particle*/
      if(Netmode)SendNetAttack(a_skill->uname,defender->s.x + (64 * (pdir * -1)),pdir * -1);
    }
    else 
    {
      SpawnSwapSpriteParticle(a_skill->sprite,a_skill->sw,a_skill->sh,8,a_skill->colorswaps[0], a_skill->colorswaps[1], a_skill->colorswaps[2],attacker->s.x + (64 * pdir),160,(16 * pdir),0,15);
      /*if net send net info for particle*/
      if(Netmode)SendNetAttack(a_skill->uname,defender->s.x + (64 * (pdir * -1)),pdir * -1);
    }
  }
  if(Netmode)SendNetCombatBegin();
  while(!done)
  {
    ResetBuffer();
    SDL_PumpEvents();
    Draw_ALL();
    UpdateCamera();
    /*send frame data*/
    if(Netmode)SendFrameData();
    NextFrame();
    /*update attack animation*/
    if(frames++ >= 15)done = 1;
  }
  /*get defend skill if we have one set*/
  if(Combatant[defender->combatindex].ChosenDefense != -1)
    d_skill = GetSkillInfoByName(PlayerStats[defender->playernum].defenses[Combatant[defender->combatindex].ChosenDefense]);
  /*calculate 'to hit'*/

  if(attrLetToIndex(a_skill->hitbonus) == -1)bonus = 0;
  else bonus = PlayerStats[attacker->playernum].attributes[attrLetToIndex(a_skill->hitbonus)];
  if(attrLetToIndex(a_skill->primary) == -1)prim = 0;
  else prim = PlayerStats[attacker->playernum].attributes[attrLetToIndex(a_skill->primary)];

  roll = SkillRoll();
  if(roll > 10)
  {
    AttributeUsed(defender,attrLetToIndex(a_skill->primary), 1);
    crit = 1;
  }
  hitscore = ((prim + bonus) * a_skill->hitrate) + (random() * 10);
  if(PlayerStats[attacker->playernum].fullness <= 0)
  {
    hitscore *= 0.75;
    NewMessage("Starving!",IndexColor(Violet));
    if(Netmode)SendMessage("Starving!",IndexColor(Violet));
  }
  else if(PlayerStats[attacker->playernum].fullness < PlayerStats[attacker->playernum].stage * 2)
  {
    hitscore *= 0.75;
    NewMessage("Very Hungry!",IndexColor(Violet));
    if(Netmode)SendMessage("Very Hungry!",IndexColor(Violet));
  }
  /*calculate 'to dodge'*/
  if((d_skill != NULL)&&(strncmp(d_skill->function,"Dodge",80) == 0))
  {
    if((PlayerStats[defender->playernum].stamina < d_skill->staminacost) || (PlayerStats[defender->playernum].health < d_skill->healthcost))
    {
      roll = SkillRoll();
      if(roll > 10)AttributeUsed(defender,AT_Agility, 1);
      dodgescore = PlayerStats[defender->playernum].attributes[AT_Agility] + roll;
      NewMessage("To tired to Defend",IndexColor(LightViolet));
      if(Netmode)SendMessage("To tired to Defend",IndexColor(LightViolet));
    }
    else
    {
      NewMessage(d_skill->skillname,IndexColor(LightViolet));
      if(Netmode)SendMessage(d_skill->skillname,IndexColor(LightViolet));
      PlayerStats[defender->playernum].stamina -= d_skill->staminacost;
      PlayerStats[defender->playernum].health -= d_skill->healthcost;
      if(attrLetToIndex(d_skill->primary) == -1)prim = 0;
      else prim = PlayerStats[defender->playernum].attributes[attrLetToIndex(d_skill->primary)];
      if(attrLetToIndex(d_skill->secondary) == -1)sec = 0;
      else sec = PlayerStats[defender->playernum].attributes[attrLetToIndex(d_skill->secondary)];
      dodgescore = GetPower(d_skill->effect,prim, sec);
      Combatant[defender->combatindex].DefenseCooldown = d_skill->cooldown;
      SkillUsed(&PlayerStats[defender->playernum],d_skill->uname);
    }
    Combatant[defender->combatindex].ChosenDefense = -1;
  }
  else 
  {
    roll = SkillRoll();
    dodgescore = PlayerStats[defender->playernum].attributes[AT_Agility] + roll;
    if(roll > 10)AttributeUsed(defender,AT_Agility, 1);
  }
  if(hitscore >= dodgescore)
  { /*HIT*/
    NewMessage("HIT!",IndexColor(LightRed));
    if(Netmode)SendMessage("HIT!",IndexColor(LightRed));
    AttributeUsed(defender,AT_Agility, 1);
    /*calculating damage*/
    if(attrLetToIndex(a_skill->primary) == -1)prim = 0;
    else prim = PlayerStats[attacker->playernum].attributes[attrLetToIndex(a_skill->primary)];
    if(attrLetToIndex(a_skill->secondary) == -1)sec = 0;
    else sec = PlayerStats[attacker->playernum].attributes[attrLetToIndex(a_skill->secondary)];
    damage = GetPower(a_skill->effect,prim, sec);
    for(i = 0; i < 5;i ++)
    {
      if(a_skill->elements[i] > 0)
      {
        elements[i] = a_skill->elements[i] + PlayerStats[attacker->playernum].Element[i];
        PlayerStats[defender->playernum].Element[i] += (elements[i] * 0.001);
        delements[i] = PlayerStats[defender->playernum].Element[i] + PlayerStats[defender->playernum].attributes[AT_Resistance];
        elemental = 1;
      }
    }
    if(crit)
    {
      NewMessage("Critical Hit!",IndexColor(LightRed));
      if(Netmode)SendMessage("Critical Hit!",IndexColor(LightRed));
      damage += (damage * 0.5);
    }
    damage += ElementalWeight(elements,delements);
    
    holydmg = 0;
    
    if(a_skill->good)holydmg = (PlayerStats[attacker->playernum].good + a_skill->good);
    if((d_skill != NULL)&&(d_skill->good))holydmg -= (PlayerStats[defender->playernum].good + d_skill->good);
    
    if(a_skill->evil)holydmg += (PlayerStats[attacker->playernum].evil + a_skill->evil);
    if((d_skill != NULL)&&(d_skill->evil))holydmg -= (PlayerStats[defender->playernum].evil + d_skill->evil);
    
    damage += holydmg;
    
    if(PlayerStats[attacker->playernum].fullness <= 0)
    {
      damage *= 0.75;
    }
    else if(PlayerStats[attacker->playernum].fullness < PlayerStats[attacker->playernum].stage * 2)
    {
      damage *= 0.75;
    }
    
    soak = GetPower(PowerLevelName(PlayerStats[defender->playernum].stage),PlayerStats[defender->playernum].attributes[AT_Endurance], 0);
    if(elemental)AttributeUsed(defender,AT_Resistance, 2);
    damage -= soak;
    if(damage <= 0)damage = 1;
    sprintf(message,"%i Damage!",damage);
    NewMessage(message,IndexColor(White));
    if(Netmode)SendMessage(message,IndexColor(White));
    PlayerStats[defender->playernum].health -= damage;
    defender->state = CS_Pain;
    if(PlayerStats[defender->playernum].health <= 0)
    {
      NewMessage("KO",IndexColor(LightViolet));
      if(Netmode)SendMessage("KO",IndexColor(LightViolet));
      defender->state = CS_Dead;
      SetCameraTarget((defender->combatindex * 239),0,5,15);
      if(Netmode)SendCameraFocus((defender->combatindex * 239),0,5,15);
      CombatOver = 1;
      PlayerStats[defender->playernum].loses++;
      PlayerStats[attacker->playernum].wins++;
      attacker->state = CS_Happy;
      attacker->frame = 3;
    }
  }
  else
  {/*MISS*/
    NewMessage("MISS!",IndexColor(White));
    if(Netmode)SendMessage("MISS!",IndexColor(White));
    /*check for counter attack*/
    AttributeUsed(defender,AT_Agility, 2);
  }
  if(Netmode)
  {
    SendAttackCooldown(Combatant[1].Combatant);
    SendDefenseCooldown(Combatant[1].Combatant);
    SendStatUpdate(Combatant[1].Combatant);
    SendStatUpdate(Combatant[0].Combatant);
  }
}


/*

  Combat Window Sections

*/

void DrawCombatMenu()
{
  int percent;
  char message[80];
  DrawWindow(1,30,240);
  DrawWindow(1,-60,-50);
  DrawWindow(1,120,-50);
  DrawSprite(attributes,screen,112,4,6);
  DrawSprite(attributes,screen,112,4 + 20,7);
    /*player stats*/
  sprintf(message,"%i / %i",(int)PlayerStats[Player->playernum].health,PlayerStats[Player->playernum].healthmax );
  DrawText(message,screen,11,5,IndexColor(DarkGrey),F_Medium);
  DrawText(message,screen,10,4,IndexColor(Green),F_Medium);
  sprintf(message,"%i / %i",(int)PlayerStats[Player->playernum].stamina,PlayerStats[Player->playernum].staminamax );
  DrawText(message,screen,11,5 + 20,IndexColor(DarkGrey),F_Medium);
  DrawText(message,screen,10,4 + 20,IndexColor(Green),F_Medium);
  /*monster stats*/
  sprintf(message,"%i / %i",(int)PlayerStats[Combatant[1].Combatant->playernum].health,PlayerStats[Combatant[1].Combatant->playernum].healthmax );
  DrawText(message,screen,136,5,IndexColor(DarkGrey),F_Medium);
  DrawText(message,screen,135,4,IndexColor(Green),F_Medium);
  sprintf(message,"%i / %i",(int)PlayerStats[Combatant[1].Combatant->playernum].stamina,PlayerStats[Combatant[1].Combatant->playernum].staminamax );
  DrawText(message,screen,136,5 + 20,IndexColor(DarkGrey),F_Medium);
  DrawText(message,screen,135,4 + 20,IndexColor(Green),F_Medium);
  
  if(Combatant[0].AttackCooldown > 0)
  {
    DrawFilledRect(5,240, 20, 80, IndexColor(DarkRed),screen);
    percent = ((100 - Combatant[0].AttackCooldown)/100)*80;
    if(percent > 100)
      percent = 100;
    DrawFilledRect(5,240 + (80 - percent), 20, percent, IndexColor(Red),screen);
  }
  else DrawFilledRect(5,240, 20, 80, IndexColor(LightRed),screen);
  if(Combatant[0].DefenseCooldown > 0)
  {
    DrawFilledRect(215,240, 20, 80, IndexColor(DarkBlue),screen);
    percent = ((100 - Combatant[0].DefenseCooldown)/100)*80;
    if(percent > 100)
      percent = 100;
    DrawFilledRect(215,240 + (80 - percent), 20, percent, IndexColor(Blue),screen);
  }
  else DrawFilledRect(215,240, 20,80, IndexColor(LightBlue),screen);
}

void DrawListMenu()
{
  DrawCombatMenu();
  DrawWindow(2,30,50);
}

void UpdateCombatMenu(int id)
{
  char message[80];
  if(CombatOver)return;
  if(id == BI_CloseWin)
  {
    SetupCombatMenu();
    return;
  }
  if(id >= CB_ListStart)
  {
    switch(hud.subwindowinfo)
    {
      case 1: /*attacks*/
        Combatant[0].ChosenAttack = id - CB_ListStart;
        /*update server with this info*/
        sprintf(message,"Attack %i Set!",Combatant[0].ChosenAttack);
        NewMessage(message,IndexColor(LightViolet));
        if(Netmode)
        {
          SendAttackUpdate(Combatant[0].Combatant);
        }
        SetupCombatMenu();
        return;
      case 2: /*defenses*/
        Combatant[0].ChosenDefense = id - CB_ListStart;
        NewMessage("Defense Set!",IndexColor(LightViolet));
        if(Netmode)SendDefenseUpdate(Combatant[0].Combatant);
        /*update server with this info*/
        SetupCombatMenu();
        return;
    }
  }
  if(!hud.subwindowinfo)
  {
    switch(id)
    {
      /*case:CB_Attacks, CB_Recover, CB_Defense, CB_Items, CB_Info, CB_Flee*/
      case CB_Attacks:
        SetupAttackMenu();
        break;
      case CB_Defense:
        SetupDefenseMenu();
        break;
      case CB_Recover:
        Combatant[0].ChosenAttack = -2;
        NewMessage("Recover Set!",IndexColor(LightViolet));
        /*update server with this info*/
        if(Netmode)SendAttackUpdate(Combatant[0].Combatant);
        break;
      case CB_Items:
      case CB_Info:
        NewMessage("Coming Soon!",IndexColor(Violet));
        break;
      case CB_Flee:
        ThePlayerStats.loses++;
        /*update server with this info*/
        CombatEnd();
        break;
    }
  }
}

void SetupCombatMenu()
{
  hud.numbuttons = 6;
  hud.buttonfocus = 0;
  hud.subwindow = 1;
  hud.subwindowinfo = 0;
  SetButton(&hud.buttons[0],CB_Attacks, -1,"Attacks",NULL,NULL,NULL,NULL,40,250,80,16,1,0);
  SetButton(&hud.buttons[1],CB_Defense, -1,"Defense",NULL,NULL,NULL,NULL,40,270,80,16,1,0);
  SetButton(&hud.buttons[2],CB_Recover, -1,"Recover",NULL,NULL,NULL,NULL,40,290,80,16,1,0);
  SetButton(&hud.buttons[3],CB_Items, -1,"Items",NULL,NULL,NULL,NULL,130,250,80,16,1,0);
  SetButton(&hud.buttons[4],CB_Info, -1,"Info",NULL,NULL,NULL,NULL,130,270,80,16,1,0);
  SetButton(&hud.buttons[5],CB_Flee, -1,"Flee",NULL,NULL,NULL,NULL,130,290,80,16,1,0);
  hud.windowdraw = DrawCombatMenu;
  hud.windowupdate = UpdateCombatMenu;

}

void SetupAttackMenu()
{
  int i;
  hud.numbuttons = 6 + ThePlayerStats.numattacks + 1;
  if(ThePlayerStats.numattacks <= 0)
  {
    NewMessage("No Offensive Skills",IndexColor(Red));
    NewMessage("Run Bitch!",IndexColor(Red));
    return;
  }
  for(i = 0; i < ThePlayerStats.numattacks;i++)
  {
    SetButton(&hud.buttons[6 + i],CB_ListStart + i, -1, ThePlayerStats.attacks[i],NULL,NULL,NULL,NULL,60,70 + (i * 20),80,16,1,0);
  }
  SetButton(&hud.buttons[6 + ThePlayerStats.numattacks],BI_CloseWin, -1," ",buttonarrows[0],buttonarrows[0], buttonarrows[1], buttonarrows[2],172,58,16,16,1,5);
  hud.windowdraw = DrawListMenu;
  hud.subwindowinfo = 1;
}

void SetupDefenseMenu()
{
  int i;
  int numdefenses = ThePlayerStats.numdefenses - ThePlayerStats.nummovements;
  if(numdefenses <= 0)
  {
    NewMessage("No Defensive Skills",IndexColor(Red));
    return;
  }
  hud.numbuttons = 6 + ThePlayerStats.numdefenses - ThePlayerStats.nummovements + 1;
  for(i = 0; i < ThePlayerStats.numdefenses - ThePlayerStats.nummovements;i++)
  {
    SetButton(&hud.buttons[6 + i],CB_ListStart + i, -1, ThePlayerStats.defenses[i],NULL,NULL,NULL,NULL,60,70 + (i * 20),80,16,1,0);
  }
  SetButton(&hud.buttons[6 + numdefenses],BI_CloseWin, -1," ",buttonarrows[0],buttonarrows[0], buttonarrows[1], buttonarrows[2],172,58,16,16,1,5);
  hud.windowdraw = DrawListMenu;
  hud.subwindowinfo = 2;
}

/*eol@eof*/

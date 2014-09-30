#include "events.h"
#include "monster.h"
#include "skills.h"
#include "particle.h"

extern Entity *Player;
extern StageInfo StageList[MAXSTAGES];
int EventMode = 0;
int EventDelay;
void (*GoEvent)() = NULL;
void (*EventExit)() = NULL;
char EventSubString[80];
int eventcolor;
/*sets an event in motions: Spawns particles, sets positions and states of ents, etc.*/

void StartEvent(Uint8 time);

/*
  Specific events
*/

void Blanket()
{
  StartEvent(80);
  SpawnSpriteParticle("images/egg_blanket.png",96,96,1,240,Player->s.y - 1,0,0,80);
}

void UpdateTalking()
{
  if(crandom() > 0.5)
  {
    NewMessage("Blah Blah Blah",IndexColor(LightGrey));
  }
  else if(crandom() > 0.5)
  {
    NewMessage("Yada Yada Yada",IndexColor(LightGrey));
  }
  else NewMessage("  ",IndexColor(LightGrey));
}

void Talking()
{
  StartEvent(60);
  GoEvent = UpdateTalking;
}


void Meditate()
{
  if(crandom() > 0.9)
  {
    NewMessage("      OHHHMMMM",IndexColor(LightGrey));
  }
  else if(crandom() > 0.9)
  {
    NewMessage("      YUUUYUUU",IndexColor(LightGrey));
  }
  else NewMessage("  ",IndexColor(LightGrey));
}

void Meditating()
{
  StartEvent(60);
  Player->frame = 7;
  GoEvent = Meditate;
}

void Bath()
{
  if(crandom() > 0.9)
  {
    SpawnBloodSpray(IndexColor(eventcolor),0,100 + (crandom() * 5),30 + crandom(),-5,150,50);
    NewMessage("Splash!",IndexColor(eventcolor));
  }
}

void Bathing(int color)
{
  StartEvent(60);
  eventcolor = color;
  Player->s.x = 0;
  GoEvent = Bath;
}


void UpdateRockfall()
{
  if(EventDelay == 30)
  {
    SpawnSpriteParticle("images/boulder.png",64,64,1,240,100,crandom() * 5,3.333,30);
    Player->frame = 14;
  }
}

void Rockfall()
{
  StartEvent(60);
  SpawnSpriteParticle("images/boulder.png",64,64,1,240,0,0,3.333,30);
  GoEvent = UpdateRockfall;
}


void UpdateBreeze()
{
  if(crandom() > 0)
    SpawnBloodSpray(IndexColor(Silver),480,150 + (crandom() * 5),-30 + crandom(),-5,15,50);
  else
    SpawnBloodSpray(IndexColor(Cyan),480,150 + (crandom() * 5),-30 + crandom(),-5,15,50);
}

void Breeze()
{
  StartEvent(60);
  GoEvent = UpdateBreeze;
}


void UpdateSparks()
{
  SpawnBloodSpray(IndexColor(Cyan),240 + (crandom() * 20),150+ (crandom() * 10),crandom() * 10,-1,10,(int)(5 + crandom()));
  SpawnBloodSpray(IndexColor(White),240 + (crandom() * 20),150+ (crandom() * 10),crandom() * 10,-1,10,(int)(5 + crandom()));
}

void Sparks()
{
  StartEvent(60);
  GoEvent = UpdateSparks;
}

void Heating()/*For egg care*/
{
  Player->frame = 3;
  StartEvent(60);
  SpawnFire(IndexColor(LightOrange),240,200,60);
}

void SunShine()
{
  SpawnBloodSpray(IndexColor(White),240,-50,0,5,25,100);
}

void Sunning(int sprawl)
{
  StartEvent(100);
  if(sprawl)Player->frame = 15;
  SpawnBloodSpray(IndexColor(White),240,-50,0,5,25,100);
  GoEvent = SunShine;
}

void Fed()
{
  if(EventDelay == 10)
  {
    Player->frame++;
    NewMessage("Yum!",IndexColor(Blue));
    /*play munchung sound*/
  }
}

void Feeding()
{
  StartEvent(60);
  Player->frame = 3;
  SpawnSpriteParticle("images/particle_ham.png",64,64,1,Player->s.x,-50,0,5,50);
  GoEvent = Fed;
}


/*Jumping*/

void Jump()
{
  Player->s.y += Player->v.y;
  Player->v.y += 0.5;
}

void Jumping()
{
  StartEvent(60);
  Player->frame = 3;
  Player->v.y = -8;
  GoEvent = Jump;
}

void Run()
{
  Player->s.x += Player->v.x;
}

void Running()
{
  StartEvent(60);
  Player->frame = 8;
  Player->v.x = 8;
  GoEvent = Run;
}

void Climb()
{
  Player->s.y -= 3;
}

void Climbing()
{
  StartEvent(60);
  Player->frame = 8;
  Player->s.x = 360;
  GoEvent = Climb;
}


void GoAttack()
{
  Skill_Info *skill;
  if(EventDelay == 10)
  {
    Player->frame++;
    skill = GetSkillInfoByName(EventSubString);
    if((skill != NULL) && (strlen(skill->sprite) > 0))
      SpawnSwapSpriteParticle(skill->sprite,skill->sw,skill->sh,8,skill->colorswaps[0], skill->colorswaps[1], skill->colorswaps[2],Player->s.x + 64,160,8,0,15);
  }
}

int AttackAnim(char uname[80])
{
  Skill_Info *skill;
  skill = GetSkillInfoByName(uname);
  if(skill == NULL)return 0;
  if(skill->skilltype > 0)return 0;
  StartEvent(60);
  strncpy(EventSubString,uname,80);
  Player->frame = 8 + (skill->animation * 2);
  GoEvent = GoAttack;
  return 1;/*attack caught*/
}

/*

  Generic Event Control Functions

*/

void StartEvent(Uint8 time)
{
  EventMode = 1;
  EventDelay = time;
  Player->state = MS_Event;
}

void UpdateEvent()
{
  EventDelay--;
  if(EventDelay <= 0)
  {
    EndEvent();
    return;
  }
  if(GoEvent != NULL)GoEvent();
}

void EndEvent()
{
  EventMode = 0;
  Player->state = MS_Normal;
  Player->s.x = 240;
  Player->s.y = 200;
  GoEvent = NULL;
  if(EventExit != NULL)EventExit();
  EventExit = NULL;
}

void DoEvent(char name[80],void (*exitevent)())
{
  EventExit = exitevent;
  if(strncmp(name,"bathing",80) == 0)
  {
    Bathing(LightBlue);
    return;
  }
  if(strncmp(name,"meditating",80) == 0)
  {
    Meditating();
    return;
  }
  if(strncmp(name,"climbing",80) == 0)
  {
    Climbing();
    return;
  }
  if(strncmp(name,"jumping",80) == 0)
  {
    Jumping();
    return;
  }
  if(strncmp(name,"running",80) == 0)
  {
    Running();
    return;
  }
  if(strncmp(name,"rockfall",80) == 0)
  {
    Rockfall();
    return;
  }
  if(strncmp(name,"sunning",80) == 0)
  {
    Sunning(1);
    return;
  }
  if(strncmp(name,"eggsunning",80) == 0)
  {
    Sunning(0);
    return;
  }
  if(strncmp(name,"sparks",80) == 0)
  {
    Sparks();
    return;
  }
  if(strncmp(name,"breeze",80) == 0)
  {
    Breeze();
    return;
  }
  if(strncmp(name,"blanket",80) == 0)
  {
    Blanket();
    return;
  }
  if(strncmp(name,"warming",80) == 0)
  {
    Heating();
    return;
  }
  if(strncmp(name,"whispering",80) == 0)
  {
    Talking();
    return;
  }
  if(strncmp(name,"feeding",80) == 0)
  {
    Feeding();
    return;
  }
  if(AttackAnim(name))return;
}







/*eol@eof*/

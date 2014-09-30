#include "skills.h"
#include "monster.h"
#include "hud.h"
#include "events.h"
#include "area.h"

Skill_Info SkillList[MAXSKILLS];
int NumSkills = 0;
extern HUDInfo hud;
extern Sprite *buttonsprites[3];
extern Sprite *mediumwindowsprite;
extern SDL_Surface *screen; /*pointer to the draw buffer*/
extern Sprite *buttonarrows[3];
extern SaveInfo PlayerStats[MAXPLAYERS];
extern Entity *Player;
extern int keymoved;


void DrawListMenuSkill()
{
  DrawWindow(2,30,50);
}
/*

  Skill conflict choice window

*/

void DrawConflictWindow()
{
  Skill_Info *skill;
  DrawWindow(2,-60,50);
  DrawWindow(2,120,50);
  if(hud.subwindowinfo != -1)
  {
    DrawWindow(1,30,-60);
    skill =GetSkillInfoByName(hud.buttons[hud.subwindowinfo].text);
    if(skill != NULL)
    {
      DrawTextCentered(skill->effect,screen,120,8,IndexColor(DarkGrey),F_Large);
      DrawTextCentered(skill->effect,screen,119,7,IndexColor(LightYellow),F_Large);
    }
    if(hud.subwindowinfo < ThePlayerStats.numattacks)
    {/*current skill*/
      DrawFilledRect(20,60 + (hud.subwindowinfo * 24),74,20, IndexColor(LightYellow),screen);
    }
    else
    {/*new skill*/
      DrawFilledRect(146,60 + ((hud.subwindowinfo - ThePlayerStats.numattacks) * 24),74,20, IndexColor(LightYellow),screen);
    }
  }
}

void UpdateConflictWindow(int id)
{
  char temp[80];
  if(id == BI_CloseWin)
  {
    ResolveDefSkillConflict();
    return;
  }
  if(id >= BI_ListStart)
  {
    if(hud.subwindowinfo == -1)
    {
      hud.subwindowinfo = id - BI_ListStart;
    }
    else if((id - BI_ListStart) < ThePlayerStats.numattacks)
    {
      if(hud.subwindowinfo < ThePlayerStats.numattacks)
      {/*select different skill*/
        if(hud.subwindowinfo == (id - BI_ListStart))
        {/*double select, de-selects*/
          hud.subwindowinfo = -1;
          return;
        }
        hud.subwindowinfo = (id - BI_ListStart);
        return;
      }
      else
      {
        strncpy(temp,ThePlayerStats.attacks[(id - BI_ListStart)],80);
        strncpy(ThePlayerStats.attacks[(id - BI_ListStart)],hud.buttons[hud.subwindowinfo].text,80);
        strncpy(hud.buttons[hud.subwindowinfo].text,temp,80);
        strncpy(hud.buttons[(id - BI_ListStart)].text,ThePlayerStats.attacks[(id - BI_ListStart)], 80);
        hud.subwindowinfo = -1;
      }
    }
    else
    {
      if(hud.subwindowinfo >= ThePlayerStats.numattacks)
      {/*select different skill*/
        if(hud.subwindowinfo == (id - BI_ListStart))
        {/*double select, de-selects*/
          hud.subwindowinfo = -1;
          return;
        }
        hud.subwindowinfo = (id - BI_ListStart);
        return;
      }
      else
      {
        strncpy(temp,ThePlayerStats.attacks[hud.subwindowinfo],80);
        strncpy(ThePlayerStats.attacks[hud.subwindowinfo],hud.buttons[(id - BI_ListStart)].text,80);
        strncpy(hud.buttons[(id - BI_ListStart)].text,temp,80);
        strncpy(hud.buttons[hud.subwindowinfo].text,ThePlayerStats.attacks[hud.subwindowinfo],80);
        hud.subwindowinfo = -1;
      }
    }
  }
}

void ResolveSkillConflict()
{
  int i;
  StageInfo *stage;
  stage = GetStageInfoByName(ThePlayerStats.stagename);
  hud.numbuttons = ThePlayerStats.numattacks + stage->numattacks + 1;
  hud.buttonfocus = 0;
  hud.subwindow = 1;
  hud.subwindowinfo = -1;
  sprintf(hud.windowheader,"Learn New Skill");
  for(i = 0; i < ThePlayerStats.numattacks;i++)
  {
    SetButton(&hud.buttons[i],BI_ListStart + i, -1,ThePlayerStats.attacks[i] ,NULL,NULL,NULL,NULL,20,60 + (i * 24),74,20,1,0);
  }
  for(i = 0; i < stage->numattacks;i++)
  {
    SetButton(&hud.buttons[ThePlayerStats.numattacks + i],BI_ListStart + ThePlayerStats.numattacks + i, -1,stage->attacks[i] ,NULL,NULL,NULL,NULL,146,60 + (i * 24),74,20,1,0);
  }
  SetButton(&hud.buttons[ThePlayerStats.numattacks + stage->numattacks],BI_CloseWin, -1,"Done",buttonsprites[2],buttonsprites[0],buttonsprites[1], buttonsprites[2],164,251,74,64,1,0);
  hud.windowdraw = DrawConflictWindow;
  hud.windowupdate = UpdateConflictWindow;
}

/*

  Defense / movement Skill conflict choice window

*/

void DrawDefConflictWindow()
{
  Skill_Info *skill;
  DrawWindow(2,-60,50);
  DrawWindow(2,120,50);
  if(hud.subwindowinfo != -1)
  {
    DrawWindow(1,30,-60);
    skill =GetSkillInfoByName(hud.buttons[hud.subwindowinfo].text);
    if(skill != NULL)
    {
      DrawTextCentered(skill->effect,screen,120,8,IndexColor(DarkGrey),F_Large);
      DrawTextCentered(skill->effect,screen,119,7,IndexColor(LightYellow),F_Large);
    }
    if(hud.subwindowinfo < ThePlayerStats.numdefenses)
    {/*current skill*/
      DrawFilledRect(20,60 + (hud.subwindowinfo * 24),74,20, IndexColor(LightYellow),screen);
    }
    else
    {/*new skill*/
      DrawFilledRect(146,60 + ((hud.subwindowinfo - ThePlayerStats.numdefenses) * 24),74,20, IndexColor(LightYellow),screen);
    }
  }
}

void UpdateDefConflictWindow(int id)
{
  char temp[80];
  if(id == BI_CloseWin)
  {
    ReorganizeSkills(&ThePlayerStats);
    SetupGameMenu();
    TextDialog("Nickname:",ThePlayerStats.name,NULL);
    return;
  }
  if(id >= BI_ListStart)
  {
    if(hud.subwindowinfo == -1)
    {
      hud.subwindowinfo = id - BI_ListStart;
    }
    else if((id - BI_ListStart) < ThePlayerStats.numdefenses)
    {
      if(hud.subwindowinfo < ThePlayerStats.numdefenses)
      {/*select different skill*/
        if(hud.subwindowinfo == (id - BI_ListStart))
        {/*double select, de-selects*/
          hud.subwindowinfo = -1;
          return;
        }
        hud.subwindowinfo = (id - BI_ListStart);
        return;
      }
      else
      {
        strncpy(temp,ThePlayerStats.defenses[(id - BI_ListStart)],80);
        strncpy(ThePlayerStats.defenses[(id - BI_ListStart)],hud.buttons[hud.subwindowinfo].text,80);
        strncpy(hud.buttons[hud.subwindowinfo].text,temp,80);
        strncpy(hud.buttons[(id - BI_ListStart)].text,ThePlayerStats.defenses[(id - BI_ListStart)], 80);
        hud.subwindowinfo = -1;
      }
    }
    else
    {
      if(hud.subwindowinfo >= ThePlayerStats.numdefenses)
      {/*select different skill*/
        if(hud.subwindowinfo == (id - BI_ListStart))
        {/*double select, de-selects*/
          hud.subwindowinfo = -1;
          return;
        }
        hud.subwindowinfo = (id - BI_ListStart);
        return;
      }
      else
      {
        strncpy(temp,ThePlayerStats.defenses[hud.subwindowinfo],80);
        strncpy(ThePlayerStats.defenses[hud.subwindowinfo],hud.buttons[(id - BI_ListStart)].text,80);
        strncpy(hud.buttons[(id - BI_ListStart)].text,temp,80);
        strncpy(hud.buttons[hud.subwindowinfo].text,ThePlayerStats.defenses[hud.subwindowinfo],80);
        hud.subwindowinfo = -1;
      }
    }
  }
}

void ResolveDefSkillConflict()
{
  int i;
  StageInfo *stage;
  stage = GetStageInfoByName(ThePlayerStats.stagename);
  hud.numbuttons = ThePlayerStats.numdefenses + stage->numdefenses + 1;
  hud.buttonfocus = 0;
  hud.subwindow = 1;
  hud.subwindowinfo = -1;
  sprintf(hud.windowheader,"Learn New Skill");
  for(i = 0; i < ThePlayerStats.numdefenses;i++)
  {
    SetButton(&hud.buttons[i],BI_ListStart + i, -1,ThePlayerStats.defenses[i] ,NULL,NULL,NULL,NULL,20,60 + (i * 24),74,20,1,0);
  }
  for(i = 0; i < stage->numdefenses;i++)
  {
    SetButton(&hud.buttons[ThePlayerStats.numdefenses + i],BI_ListStart + ThePlayerStats.numdefenses + i, -1,stage->defenses[i] ,NULL,NULL,NULL,NULL,146,60 + (i * 24),74,20,1,0);
  }
  SetButton(&hud.buttons[ThePlayerStats.numdefenses + stage->numdefenses],BI_CloseWin, -1,"Done",buttonsprites[2],buttonsprites[0],buttonsprites[1], buttonsprites[2],164,251,74,64,1,0);
  hud.windowdraw = DrawDefConflictWindow;
  hud.windowupdate = UpdateDefConflictWindow;
}
/*

  Training Stuff

*/

void SetupSkillTrainMenu()
{
  int i;
  if(ThePlayerStats.numattacks <= 0)
  {
    NewMessage("No Offensive Skills",IndexColor(Red));
    SetupGameMenu();
    return;
  }
  hud.numbuttons = ThePlayerStats.numattacks + 1;
  for(i = 0; i < ThePlayerStats.numattacks;i++)
  {
    SetButton(&hud.buttons[i],BI_ListStart + i, -1, ThePlayerStats.attacks[i],NULL,NULL,NULL,NULL,60,70 + (i * 20),80,16,1,0);
  }
  SetButton(&hud.buttons[ThePlayerStats.numattacks],BI_CloseWin, -1," ",buttonarrows[0],buttonarrows[0], buttonarrows[1], buttonarrows[2],172,58,16,16,1,5);
  hud.windowdraw = DrawListMenuSkill;
  hud.subwindowinfo = 1;
}

/*
  Training Windows
*/

void DrawTrainTypeWindow()
{
  DrawSprite(mediumwindowsprite,screen,30,60,0);
  DrawTextCentered(hud.windowheader,screen,120,68,IndexColor(DarkGrey),F_Large);
  DrawTextCentered(hud.windowheader,screen,119,67,IndexColor(Green),F_Large);
}

void UpdateTrainTypeWindow(int pressID)
{
  Skill_Info *skill;
  int roll;
  if(pressID == -1)return;
  switch(pressID)
  {
    case BI_Yes:
      SetupSkillTrainMenu();
      keymoved = 2;
      return;
    case BI_No:
      SetupGameMenu();
      SetupActionTrainMenu();
      keymoved = 2;
      break;
    case BI_CloseWin:
      SetupGameMenu();
      keymoved = 2;
      break;
  }
  if(pressID >= BI_ListStart)
  {
    switch(hud.subwindowinfo)
    {
      case 1:
        skill = GetSkillInfoByName(ThePlayerStats.attacks[pressID - BI_ListStart]);
        if(skill == NULL)
        {
          NewMessage("Invalid Skill",IndexColor(Red));
          keymoved = 2;
          SetupGameMenu();
          return;
        }
        if((skill->staminacost > ThePlayerStats.stamina)||(skill->healthcost > ThePlayerStats.health))
        {
          NewMessage("Too Tired Now",IndexColor(Violet));
          keymoved = 2;
          SetupGameMenu();
          return;
        }
        roll = SkillRoll();
        ThePlayerStats.fullness -= 1;
        ThePlayerStats.stamina -= skill->staminacost;
        ThePlayerStats.health -= skill->healthcost;
        if(roll < 0)
        {
          NewMessage("Oops, Messed up!",IndexColor(LightViolet));
        }
        else if(roll > 10)
        {
          SkillUsed(&ThePlayerStats,skill->uname);
          SkillUsed(&ThePlayerStats,skill->uname);
          NewMessage("Critical!",IndexColor(White));
          NewMessage(skill->skillname,IndexColor(White));
        }
        else
        {
          SkillUsed(&ThePlayerStats,skill->uname);
          NewMessage(skill->skillname,IndexColor(LightYellow));
        }
        DoEvent(ThePlayerStats.attacks[pressID - BI_ListStart],NULL);
        SetupGameMenu();
        keymoved = 2;
        break;
      case 2:
        GoTrainArea(pressID - BI_ListStart);
        SetupGameMenu();
        break;
    }
  }
}


void SetupTrainWindow()
{
  hud.numbuttons = 2;
  hud.buttonfocus = 0;
  hud.subwindow = 1;
  strncpy(hud.windowheader,"How to Train?",80);
  SetButton(&hud.buttons[0],BI_Yes, -1,"Skills",buttonsprites[2],buttonsprites[0],buttonsprites[1], buttonsprites[2],44,90,74,64,1,0);
  SetButton(&hud.buttons[1],BI_No, -1,"Action",buttonsprites[2],buttonsprites[0],buttonsprites[1], buttonsprites[2],122,90,74,64,1,0);
  hud.windowdraw = DrawTrainTypeWindow;
  hud.windowupdate = UpdateTrainTypeWindow;
}

char *PowerLevelName(int level)
{
  switch(level)
  {
    case 0:
      return "Minimal";
    case 1:
      return "Light";
    case 2:
      return "Medium";
    case 3:
      return "Heavy";
    case 4:
      return "Great";
    case 5:
      return "Massive";
    default:
      return "None";

  }
  return NULL;
}

/*EP_None, EP_Minimal, EP_Light, EP_Medium, EP_Heavy, EP_Great, EP_Massive*/
int GetPower(char powertype[80],int primary,int secondary)
{
  if(strncmp(powertype,"None",80)==0)
  {
    return 0;
  }
  if(strncmp(powertype,"Minimal",80)==0)
  {
    return ((primary * random()) + (secondary * (random() * 0.6)));
  }
  if(strncmp(powertype,"Light",80)==0)
  {
    return ((primary * (random() * 2)) + (secondary * random()));
  }
  if(strncmp(powertype,"Medium",80)==0)
  {
    return ((primary * (random() * 4)) + (secondary * (random() * 2)));
  }
  if(strncmp(powertype,"Heavy",80)==0)
  {
    return ((primary * (random() * 8)) + (secondary * (random() * 4)));
  }
  if(strncmp(powertype,"Great",80)==0)
  {
    return ((primary * (random() * 16)) + (secondary * (random() * 8)));
  }
  if(strncmp(powertype,"Massive",80)==0)
  {
    return ((primary * (random() * 32)) + (secondary * (random() * 16)));
  }
  return -1;
}

Skill_Info *GetSkillInfoByName(char uname[80])
{
  int i;
  for(i = 0;i < NumSkills;i++)
  {
    if(strncmp(uname,SkillList[i].uname,80)==0)
    {
      return &SkillList[i];
    }
  }
  return NULL;
}

void LoadSkillInfoBinary();
void SaveSkillInfoBinary();

int attrLetToIndex(int l)
{
  switch(l)
  {
    case 0:
      return AT_Strength;
    case 1:
      return AT_Dexterity;
    case 2:
      return AT_Focus;
    case 3:
      return AT_Endurance;
    case 4:
      return AT_Agility;
    case 5:
      return AT_Resistance;
  }
  return -1;
}

void SaveSkillInfoText()
{
  int i,j;
  FILE *file;
  file = fopen("system/skilllist.cfg","w");
  if(file == NULL)
  {
    fprintf(stderr,"unable to open skill file!: %s\n",SDL_GetError());
    return;
  }
  fprintf(file,"# This file generated by game.  DO NOT EDIT!\n");
  for(i = 0;i < NumSkills;i++)
  {
    fprintf(file,"uname %s\n",SkillList[i].uname);
    fprintf(file,"displayname %s\n",SkillList[i].skillname);
    fprintf(file,"skilltype %i\n",SkillList[i].skilltype);
    fprintf(file,"animation %i\n",SkillList[i].animation);
    fprintf(file,"staminacost %i\n",SkillList[i].staminacost);
    fprintf(file,"healthcost %i\n",SkillList[i].healthcost);
    fprintf(file,"hitrate %f\n",SkillList[i].hitrate);
    fprintf(file,"hitbonus %i\n",SkillList[i].hitbonus);
    fprintf(file,"projectile %i\n",SkillList[i].projectile);
    fprintf(file,"sprite %s\n",SkillList[i].sprite);
    fprintf(file,"widthheight %i %i\n", SkillList[i].sw,SkillList[i].sh);
    fprintf(file,"colorswap %i %i %i\n", SkillList[i].colorswaps[0],SkillList[i].colorswaps[1],SkillList[i].colorswaps[2]);
    fprintf(file,"hitevent %s\n",SkillList[i].hitevent);
    fprintf(file,"hitfunction %s\n",SkillList[i].function);
    fprintf(file,"power %s\n",SkillList[i].effect);
    fprintf(file,"primary %i\n",SkillList[i].primary);
    fprintf(file,"secondary %i\n",SkillList[i].secondary);
    for(j = 0;j < 6;j++)
      fprintf(file,"trains %i %i\n",j,SkillList[i].trains[j]);
    for(j = 0;j < 5;j++)
      fprintf(file,"element %i %i\n",j,SkillList[i].elements[j]);
    fprintf(file,"light %i\n",SkillList[i].good);
    fprintf(file,"dark %i\n",SkillList[i].evil);
    fprintf(file,"effect %i\n",SkillList[i].statuseffect);
    fprintf(file,"cooldown %i\n",SkillList[i].cooldown);
    fprintf(file,"description %s\n",SkillList[i].description);
    fprintf(file,"end");
  }
  fclose(file);
}


void LoadSkillInfoText()
{
  int i;
  int temp;
  char buf[128];
  char *c;
  FILE *file;
  file = fopen("system/skilllist.cfg","r");
  if(file == NULL)
  {
    fprintf(stderr,"unable to open skill file!: %s\n",SDL_GetError());
    return;
  }
  memset(SkillList,-1,sizeof(Skill_Info) * MAXSKILLS);
  i = 0;
  while((fscanf(file, "%s", buf) != EOF)&&(i < MAXSKILLS))
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
    if(strcmp(buf,"uname") ==0)
    {
      fgetc(file);  /*clear the space before the word*/
      fgets(SkillList[i].uname, 80, file);
      c = strchr(SkillList[i].uname, '\n');
      if(c != NULL) *c = '\0';    /*replace trailing return with terminating character*/
      continue;
    }
    if(strcmp(buf,"displayname") ==0)
    {
      fgetc(file);  /*clear the space before the word*/
      fgets(SkillList[i].skillname, 80, file);
      c = strchr(SkillList[i].skillname, '\n');
      if(c != NULL) *c = '\0';    /*replace trailing return with terminating character*/
      continue;
    }
    if(strcmp(buf,"skilltype") ==0)
    {
      fscanf(file, "%i", &SkillList[i].skilltype);
      continue;
    }
    if(strcmp(buf,"animation") ==0)
    {
      fscanf(file, "%i", &SkillList[i].animation);
      continue;
    }
    if(strcmp(buf,"staminacost") ==0)
    {
      fscanf(file, "%i", &SkillList[i].staminacost);
      continue;
    }
    if(strcmp(buf,"healthcost") ==0)
    {
      fscanf(file, "%i", &SkillList[i].healthcost);
      continue;
    }
    if(strcmp(buf,"hitrate") ==0)
    {
      fscanf(file, "%f", &SkillList[i].hitrate);
      continue;
    }
    if(strcmp(buf,"projectile") ==0)
    {
      fscanf(file, "%i", &SkillList[i].projectile);
      continue;
    }
    if(strcmp(buf,"sprite") ==0)
    {
      fgetc(file);  /*clear the space before the word*/
      fscanf(file, "%s", SkillList[i].sprite);
      c = strchr(SkillList[i].sprite, '\n');
      if(c != NULL) *c = '\0';    /*replace trailing return with terminating character*/
      continue;
    }
    if(strcmp(buf,"widthheight") ==0)
    {
      fscanf(file, "%i %i", &SkillList[i].sw, &SkillList[i].sh);
      continue;
    }
    if(strcmp(buf,"colorswap") ==0)
    {
      fscanf(file, "%i %i %i", &SkillList[i].colorswaps[0], &SkillList[i].colorswaps[1], &SkillList[i].colorswaps[2]);
      continue;
    }
    if(strcmp(buf,"hitevent") ==0)
    {
      fgetc(file);  /*clear the space before the word*/
      fscanf(file, "%s", SkillList[i].hitevent);
      c = strchr(SkillList[i].hitevent, '\n');
      if(c != NULL) *c = '\0';    /*replace trailing return with terminating character*/
      continue;
    }
    if(strcmp(buf,"hitbonus") ==0)
    {
      fscanf(file, "%i", &SkillList[i].hitbonus);
      continue;
    }
    if(strcmp(buf,"hitfunction") ==0)
    {
      fgetc(file);  /*clear the space before the word*/
      fgets(SkillList[i].function, 80, file);
      c = strchr(SkillList[i].function, '\n');
      if(c != NULL) *c = '\0';    /*replace trailing return with terminating character*/
      continue;
    }
    if(strcmp(buf,"power") ==0)
    {
      fgetc(file);  /*clear the space before the word*/
      fscanf(file, "%s", SkillList[i].effect);
      c = strchr(SkillList[i].effect, '\n');
      if(c != NULL) *c = '\0';    /*replace trailing return with terminating character*/
      continue;
    }
    if(strcmp(buf,"primary") ==0)
    {
      fscanf(file, "%i", &SkillList[i].primary);
      continue;
    }
    if(strcmp(buf,"secondary") ==0)
    {
      fscanf(file, "%i", &SkillList[i].secondary);
      continue;
    }
    if(strcmp(buf,"trains") ==0)
    {
      fscanf(file, "%i", &temp);
      fscanf(file, "%i", &SkillList[i].trains[temp]);
      continue;
    }
    if(strcmp(buf,"element") ==0)
    {
      fscanf(file, "%i", &temp);
      fscanf(file, "%i", &SkillList[i].elements[temp]);
      continue;
    }
    if(strcmp(buf,"light") ==0)
    {
      fscanf(file, "%i", &SkillList[i].good);
      continue;
    }
    if(strcmp(buf,"dark") ==0)
    {
      fscanf(file, "%i", &SkillList[i].evil);
      continue;
    }
    if(strcmp(buf,"effect") ==0)
    {
      fscanf(file, "%i", &SkillList[i].statuseffect);
      continue;
    }
    if(strcmp(buf,"cooldown") ==0)
    {
      fscanf(file, "%i", &SkillList[i].cooldown);
      continue;
    }
    if(strcmp(buf,"description") ==0)
    {
      fgetc(file);  /*clear the space before the word*/
      fgets(SkillList[i].description, 120, file);
      c = strchr(SkillList[i].description, '\n');
      if(c != NULL) *c = '\0';    /*replace trailing return with terminating character*/
      continue;
    }
  }
  fclose(file);
  NumSkills = i;
}

/*eol@eof*/

#include "area.h"
#include "monster.h"
#include "hud.h"
#include "combat.h"
#include "events.h"
#include "skills.h"

extern SDL_Surface *background;
extern SDL_Surface *screen;
extern SDL_Rect Camera;
extern Entity *Player;
extern HUDInfo hud;
extern Sprite *buttonsprites[3];
extern Sprite *buttonarrows[3];
extern SaveInfo PlayerStats[MAXPLAYERS];
extern StageInfo StageList[MAXSTAGES];
extern int NumStages;
Area *CurrentArea;
Sprite *bgsprite;
Sprite *icons[8];
Area AreaList[MAXAREAS];
int NumAreas = 1;
/*
 *  Area Window Functions
*/

void UpdatePrenatalWindow(int id)
{
  char listitems[6][80] = {"warming",
    "sparks",
    "breeze",
    "whispering",
    "blanket",
    "eggsunning"};
  switch(id)
  {
    case BI_CloseWin:
      SetupGameMenu();
      return;
  }
  if(id >= BI_ListStart)
  {
    DoEvent(listitems[id - BI_ListStart],NULL);
    switch(id - BI_ListStart)
    {
      case 0:
        ThePlayerStats.Element[3] += 0.3;
        ThePlayerStats.attributes[AT_Strength]++;
        ThePlayerStats.AgeStep += 5;
        break;
      case 1:
        ThePlayerStats.Element[4] += 0.3;
        ThePlayerStats.attributes[AT_Dexterity]++;
        ThePlayerStats.AgeStep += 5;
        break;
      case 2:
        ThePlayerStats.Element[1] += 0.3;
        ThePlayerStats.attributes[AT_Agility]++;
        ThePlayerStats.AgeStep += 5;
        break;
      case 3:
        ThePlayerStats.Element[2] += 0.3;
        ThePlayerStats.attributes[AT_Focus]++;
        ThePlayerStats.AgeStep += 5;
        break;
      case 4:
        ThePlayerStats.Element[0] += 0.3;
        ThePlayerStats.attributes[AT_Endurance]++;
        ThePlayerStats.evil += 0.3;
        ThePlayerStats.AgeStep += 5;
        break;
      case 5:
        ThePlayerStats.Element[2] += 0.15;
        ThePlayerStats.Element[3] += 0.15;
        ThePlayerStats.attributes[AT_Resistance]++;
        ThePlayerStats.good += 0.3;
        ThePlayerStats.AgeStep += 5;
        break;
    }
    SetupGameMenu();
  }
}

void SetupPrenatalCareMenu()
{
  int i;
  char listitems[6][80] = {"Heat",
    "Sparks",
    "Cool Water",
    "Whisper",
    "Blanket",
    "Sunning"};
  hud.subwindow = 1;
  hud.numbuttons = 7;
  for(i = 0; i < 6;i++)
  {
    SetButton(&hud.buttons[i],BI_ListStart + i, -1, listitems[i], NULL,NULL,NULL,NULL,60,70 + (i * 20),80,16,1,0);
  }
  SetButton(&hud.buttons[6],BI_CloseWin, -1," ",buttonarrows[0],buttonarrows[0], buttonarrows[1], buttonarrows[2],172,58,16,16,1,5);
  hud.windowdraw = DrawListMenuSkill;
  hud.windowupdate = UpdatePrenatalWindow;
  hud.subwindowinfo = 1;
}




void SetupActionTrainMenu()
{
  int i;
  int trains;
  int hi;
  trains = GetTrainsFound();
  if(trains <= 0)
  {
    NewMessage("No Activities Found",IndexColor(LightRed));
    SetupGameMenu();
    return;
  }
  SetupGameMenu();
  hud.subwindow = 1;
  hud.numbuttons = trains + 1;
  hi = GetHistoryIndexByUName(CurrentArea->uname,&PlayerStats[Player->playernum]);
  for(i = 0; i < trains;i++)
  {
    SetButton(&hud.buttons[i],BI_ListStart + i, -1, ThePlayerStats.history[hi].trainsfound[i], NULL,NULL,NULL,NULL,60,70 + (i * 20),80,16,1,0);
  }
  SetButton(&hud.buttons[trains],BI_CloseWin, -1," ",buttonarrows[0],buttonarrows[0], buttonarrows[1], buttonarrows[2],172,58,16,16,1,5);
  hud.windowdraw = DrawListMenuSkill;
  hud.windowupdate = UpdateTrainTypeWindow;
  hud.subwindowinfo = 2;
}


void SearchArea()
{
  int food;
  int rval;
  int mi;
  char message[80];
  int combat = 0;
  if(Player->asleep)
  {
    NewMessage("Can't explore:",IndexColor(LightRed));
    NewMessage("Sleeping!",IndexColor(LightRed));
    return;
  }
  if(ThePlayerStats.stage <= S_Egg)
  {
    NewMessage("Can't explore:",IndexColor(LightRed));
    NewMessage("Its an Egg!",IndexColor(LightRed));
    return;
  }
  if(ThePlayerStats.stamina < CurrentArea->searchcost)
  {
    NewMessage("Too tired now!",IndexColor(LightRed));
    return;
  }
  ThePlayerStats.stamina -= CurrentArea->searchcost;
  rval = (int)(random() * (10 + CurrentArea->encounter_rate));
  switch(rval)
  {
    case 1:
      NewMessage("An Event!",IndexColor(LightViolet));
      break;
    case 5:
    case 3:
    case 6:
      if(FindExit())
      {
        NewMessage("Found New Area!",IndexColor(LightViolet));
        break;
      }
    case 2:
    case 7:
    case 8:
      food = (int)(random() * CurrentArea->foodavailable * 4) + 1;
      ThePlayerStats.foodsupply += food;
      sprintf(message,"You found %i Food!",food);
      NewMessage(message,IndexColor(LightOrange));
      if(ThePlayerStats.foodsupply > (ThePlayerStats.stage * 100))
      {
        ThePlayerStats.foodsupply = (ThePlayerStats.stage * 100);
        NewMessage("Can't carry anymore!",IndexColor(LightRed));
      }
      break;
    case 4:
      if(FindTrain())
      {
        NewMessage("A New Way to Train!",IndexColor(LightViolet));
        break;
      }
      else NewMessage("Woulda Been Train",IndexColor(LightViolet));
    default:
      mi = (int)(random() * (NumStages - 2)) + 1;
      fprintf(stdout,"Monster Index: %i / %i \n",mi,NumStages);
      CombatBeginEncounter(StageList[mi].name,CurrentArea->encounter_level);
      combat = 1;
      NewMessage("An Encounter!",IndexColor(LightViolet));
  }
  if(!combat)
  {
    SetupGameMenu();
    ThePlayerStats.fullness -= (CurrentArea->searchcost * 0.01);
    ThePlayerStats.fatigue += (CurrentArea->searchcost * 2);
  }
}


void GoTrainArea(int train)
{
  int hi;
  int roll;
  int actindex;
  Skill_Info *skill;
  int primary;
  hi = GetHistoryIndexByUName(CurrentArea->uname,&PlayerStats[Player->playernum]);
  skill = GetSkillInfoByName(PlayerStats[Player->playernum].history[hi].trainsfound[train]);
  if(skill == NULL)return;
  primary = attrLetToIndex(skill->primary);
  if(primary == -1)primary = 0;
  DoEvent(PlayerStats[Player->playernum].history[hi].trainsfound[train],NULL);
  actindex = GetTrainIndexByUName(PlayerStats[Player->playernum].history[hi].trainsfound[train]);
  if(CurrentArea->act_challenge[actindex] > (ThePlayerStats.attributes[primary] + 10))
  {
    roll = SkillRoll();
    ThePlayerStats.fullness -= (skill->staminacost * 0.5);
    ThePlayerStats.fatigue += (skill->staminacost * 2);
    ThePlayerStats.stamina -= skill->staminacost;
    ThePlayerStats.health -= skill->healthcost;
    if(roll < 0)
    {/*bogey*/
      NewMessage("Oh No, Failed!",IndexColor(LightRed));
      NewMessage("Injured in Attempt!",IndexColor(LightRed));
      ThePlayerStats.stamina -= CurrentArea->act_challenge[actindex];
      ThePlayerStats.health -= CurrentArea->act_challenge[actindex];
    }
    else if(roll > 10)
    {/*chance in hell*/
      SkillUsed(&ThePlayerStats,PlayerStats[Player->playernum].history[hi].trainsfound[train]);
      SkillUsed(&ThePlayerStats,PlayerStats[Player->playernum].history[hi].trainsfound[train]);
      SkillUsed(&ThePlayerStats,PlayerStats[Player->playernum].history[hi].trainsfound[train]);
      SkillUsed(&ThePlayerStats,PlayerStats[Player->playernum].history[hi].trainsfound[train]);
      NewMessage("Exellent Work!",IndexColor(White));
    }
    else
    {
      SkillUsed(&ThePlayerStats,PlayerStats[Player->playernum].history[hi].trainsfound[train]);
      SkillUsed(&ThePlayerStats,PlayerStats[Player->playernum].history[hi].trainsfound[train]);
      NewMessage("Hard Work!",IndexColor(LightYellow));
    }
  }
  else if(CurrentArea->act_challenge[actindex] > ThePlayerStats.attributes[primary])
  {
    SkillUsed(&ThePlayerStats,PlayerStats[Player->playernum].history[hi].trainsfound[train]);
    SkillUsed(&ThePlayerStats,PlayerStats[Player->playernum].history[hi].trainsfound[train]);
    if(CurrentArea->act_challenge[actindex] < (ThePlayerStats.attributes[primary] + (random() * 10)))
    {
      NewMessage("Great Workout!",IndexColor(LightYellow));
      SkillUsed(&ThePlayerStats,PlayerStats[Player->playernum].history[hi].trainsfound[train]);
    }
    else NewMessage("Good Workout",IndexColor(LightViolet));
  }
  else if((CurrentArea->act_challenge[actindex] + 10) > ThePlayerStats.attributes[primary])
  {
    NewMessage("Not very challenging",IndexColor(LightViolet));
    SkillUsed(&ThePlayerStats,PlayerStats[Player->playernum].history[hi].trainsfound[train]);
  }
  else NewMessage("This is useless",IndexColor(LightRed));
  ThePlayerStats.fullness -= (skill->staminacost * 0.5);
  ThePlayerStats.fatigue += (skill->staminacost * 2);
  ThePlayerStats.stamina -= skill->staminacost;
  ThePlayerStats.health -= skill->healthcost;
}

/*

  Area info functions

*/

int GetExitsFound()
{
  int historyindex;
  historyindex = GetHistoryIndexByUName(CurrentArea->uname,&PlayerStats[Player->playernum]);
  if(historyindex == -1)return 0;
  return PlayerStats[Player->playernum].history[historyindex].foundexitcount;
}

int GetTrainsFound()
{
  int historyindex;
  historyindex = GetHistoryIndexByUName(CurrentArea->uname,&PlayerStats[Player->playernum]);
  if(historyindex == -1)return 0;
  return PlayerStats[Player->playernum].history[historyindex].foundactivitycount;
}

int ExitFoundYet(char uname[80])
{
  int i;
  int hi;
  hi = GetHistoryIndexByUName(CurrentArea->uname,&PlayerStats[Player->playernum]);
  for(i = 0;i < PlayerStats[Player->playernum].history[hi].foundexitcount;i++)
  {
    if(strncmp(PlayerStats[Player->playernum].history[hi].exitsfound[i],uname,80) == 0)return 1;
  }
  return 0;
}

int TrainFoundYet(char uname[80])
{
  int i;
  int hi;
  hi = GetHistoryIndexByUName(CurrentArea->uname,&PlayerStats[Player->playernum]);
  for(i = 0;i < PlayerStats[Player->playernum].history[hi].foundactivitycount;i++)
  {
    if(strncmp(PlayerStats[Player->playernum].history[hi].trainsfound[i],uname,80) == 0)return 1;
  }
  return 0;
}

int FindExit()
{
  int i;
  for(i = 0;i < 8;i++)
  {
    if(GetAreaIndexByUName(CurrentArea->exits[i]) != -1)
    {   /*first make sure the string is a valid area...*/
      if(!ExitFoundYet(CurrentArea->exits[i]))
      {/*we found an exit we havn't used yet*/
        FindExitByUName(CurrentArea->exits[i]);
        return 1;
      }
    }
  }
  return 0;
}

int FindTrain()
{
  int i;
  for(i = 0;i < 5;i++)
  {
    if(strlen(CurrentArea->activities[i]) > 0)
    {   /*first make sure the string is a valid activity...*/
      if(!TrainFoundYet(CurrentArea->activities[i]))
      {/*we found an exit we havn't used yet*/
        FindTrainByUName(CurrentArea->activities[i]);
        return 1;
      }
    }
  }
  return 0;
}

char *GetDirectionName(int dir)
{
  switch(dir)
  {}
  return NULL;
}

void FindExitByUName(char uname[80])
{
  int historyindex;
  int exitnum;
  historyindex = GetHistoryIndexByUName(CurrentArea->uname,&PlayerStats[Player->playernum]);
  exitnum = PlayerStats[Player->playernum].history[historyindex].foundexitcount;
  strncpy(PlayerStats[Player->playernum].history[historyindex].exitsfound[exitnum],uname,80);
  PlayerStats[Player->playernum].history[historyindex].foundexitcount++;
}

void FindTrainByUName(char uname[80])
{
  int historyindex;
  int exitnum;
  historyindex = GetHistoryIndexByUName(CurrentArea->uname,&PlayerStats[Player->playernum]);
  exitnum = PlayerStats[Player->playernum].history[historyindex].foundactivitycount;
  strncpy(PlayerStats[Player->playernum].history[historyindex].trainsfound[exitnum],uname,80);
  PlayerStats[Player->playernum].history[historyindex].foundactivitycount++;
}

void GoMigrate()
{
  int hi;
  hi = GetHistoryIndexByUName(CurrentArea->uname,&PlayerStats[Player->playernum]);
  MigrateTo(PlayerStats[Player->playernum].history[hi].exitsfound[hud.subwindowinfo]);
  SetupGameMenu();
}

/*
 *  Area Selection Window sections
 */

void DrawAreaSelectWindow()
{
  int index;
  int hi;
  Area *nextarea;
  hi = GetHistoryIndexByUName(CurrentArea->uname,&PlayerStats[Player->playernum]);
  nextarea = GetAreaByUName(PlayerStats[Player->playernum].history[hi].exitsfound[hud.subwindowinfo]);
  DrawWindow(2,30,30);
  if((nextarea != NULL)&&(strlen(nextarea->displayname) > 0))
  {
    DrawTextCentered(nextarea->displayname,screen,120,56,IndexColor(DarkGrey),F_Large);
    DrawTextCentered(nextarea->displayname,screen,119,55,IndexColor(LightGreen),F_Large);
  }
  DrawTextCentered("Move to",screen,120,38,IndexColor(DarkGrey),F_Large);
  DrawTextCentered("Move to",screen,119,37,IndexColor(Green),F_Large);
  index = GetExitIndexByUName(PlayerStats[Player->playernum].history[hi].exitsfound[hud.subwindowinfo]);
  if(icons[index] != NULL)
    DrawSprite(icons[index],screen,120 - 32,80,0);
}


void UpdateAreaSelectWindow(int pressID)
{
  if(pressID == -1)return;
  switch(pressID)
  {
    case BI_CloseWin:
      SetupGameMenu();
      break;
    case BI_Next:
      hud.subwindowinfo++;
      if(hud.subwindowinfo >= GetExitsFound())
        hud.subwindowinfo = 0;
      break;
    case BI_Prev:
      hud.subwindowinfo--;
      if(hud.subwindowinfo < 0)
        hud.subwindowinfo = GetExitsFound() - 1;
      if(hud.subwindowinfo < 0)
        hud.subwindowinfo = 0;
      break;
    case BI_OK:
      /*if conditions met for migration AND*/
      YesNo("Really Migrate?",GoMigrate,SetupGameMenu);
      break;
  }
}

void AreaSelect()
{
  hud.numbuttons = 4;
  hud.buttonfocus = 0;
  hud.subwindow = 1;
  hud.subwindowinfo = 0;
  SetButton(&hud.buttons[0],BI_CloseWin, -1," ",buttonarrows[0],buttonarrows[0], buttonarrows[1], buttonarrows[2],172,38,16,16,1,5);
  SetButton(&hud.buttons[1],BI_Prev, -1," ",buttonarrows[0],buttonarrows[0], buttonarrows[1], buttonarrows[2],38,130,16,16,1,1);
  SetButton(&hud.buttons[2],BI_Next, -1," ",buttonarrows[0],buttonarrows[0], buttonarrows[1], buttonarrows[2],186,130,16,16,1,3);
  SetButton(&hud.buttons[3],BI_OK, -1,"GO",buttonsprites[2],buttonsprites[0],buttonsprites[1], buttonsprites[2],82,154,74,64,1,0);
  hud.windowdraw = DrawAreaSelectWindow;
  hud.windowupdate = UpdateAreaSelectWindow;

}

/*
 * Area maintenance functions
 */

void LoadArea(char filename[80])
{
  int i;
  int exitarea;
  Area *area;
  area = GetAreaByUName(filename);
  Camera.x = 120;
  if(area != NULL)
  {
    if(bgsprite != NULL)FreeSprite(bgsprite);
    bgsprite = LoadSprite(area->imagename,480,320);
    for(i = 0;i < 8;i++)
    {
      if(icons[i] != NULL)
      {
        FreeSprite(icons[i]);
        icons[i] = NULL;
      }
      exitarea = GetAreaIndexByUName(area->exits[i]);
      if(exitarea != -1)
      {
        icons[i] = LoadSprite(AreaList[exitarea].iconname,64,64);
      }
    }
  }
  if(bgsprite != NULL)
  {
    BlankScreen(background,0);
    SDL_BlitSurface(bgsprite->image,NULL,background,NULL);
  }
  CurrentArea = area;
}

void AddAreaToHistory(Area *area , SaveInfo *info)
{
  int i;
  for(i = 0;i < MAXAREAS;i++)
  {
    if(!info->history[i].inuse)break;
  }
  if(i >= MAXAREAS)return;
  strncpy(info->history[i].uname,area->uname,80);
  info->history[i].inuse = 1;
}

int GetHistoryIndexByUName(char uname[80],SaveInfo *info)
{
  int i;
  for(i = 0;i < MAXAREAS;i++)
  {
    if(strncmp(uname,info->history[i].uname,80)==0)
    {
      return i;
    }
  }
  return -1;

}

Area *GetAreaByUName(char uname[80])
{
  int i;
  for(i = 0;i < NumAreas;i++)
  {
    if(strncmp(uname,AreaList[i].uname,80)==0)
    {
      return &AreaList[i];
    }
  }
  return NULL;
}

int GetAreaIndexByUName(char uname[80])
{
  int i;
  for(i = 0;i < NumAreas;i++)
  {
    if(strncmp(uname,AreaList[i].uname,80)==0)
    {
      return i;
    }
  }
  return -1;
}


int GetTrainIndexByUName(char uname[80])
{
  int i;
  for(i = 0;i < 5;i++)
  {
    if(strncmp(uname,CurrentArea->activities[i],80)==0)
    {
      return i;
    }
  }
  return -1;/*not found*/
}

int GetFoundTrainByUname(char uname[80])
{
  int i;
  int hi;
  hi = GetHistoryIndexByUName(CurrentArea->uname,&PlayerStats[Player->playernum]);
  for(i = 0;i < 5;i++)
  {
    if(strncmp(uname,ThePlayerStats.history[hi].trainsfound[i],80)==0)
    {
      return i;
    }
  }
  return -1;/*not found*/
}

int GetExitIndexByUName(char uname[80])
{
  int i;
  for(i = 0;i < 8;i++)
  {
    if(strncmp(uname,CurrentArea->exits[i],80)==0)
    {
      return i;
    }
  }
  return -1;/*not found*/
}


void LoadAreasBinary();
void SaveAreasBinary();


void SaveAreasText()
{
  int i,j;
  FILE *file;
  file = fopen("system/arealist.cfg","w");
  if(file == NULL)
  {
    fprintf(stderr,"unable to open area file!: %s\n",SDL_GetError());
    return;
  }
  fprintf(file,"# This file generated by game.  DO NOT EDIT!\n");
  for(i = 0;i < NumAreas;i++)
  {
    fprintf(file,"uname %s\n",AreaList[i].uname);
    fprintf(file,"displayname %s\n",AreaList[i].displayname);
    fprintf(file,"bgimage %s\n",AreaList[i].imagename);
    fprintf(file,"iconimage %s\n",AreaList[i].iconname);
    fprintf(file,"bgmusic %s\n",AreaList[i].bgmusic);
    fprintf(file,"food %i\n",AreaList[i].foodavailable);
    for(j = 0;j < 5;j++)
      fprintf(file,"bgelement %i %i\n",j,AreaList[i].elements[j]);
    for(j = 0;j < 5;j++)
      fprintf(file,"activity %i %s %i\n",j,AreaList[i].activities[j],AreaList[i].act_challenge[j]);
    for(j = 0;j < 8;j++)
    {
      fprintf(file,"exit %i %s\n",j,AreaList[i].exits[j]);
      fprintf(file,"exitcondition %i %s\n",j,AreaList[i].exit_conditions[j]);
    }
    fprintf(file,"bgalignment %i\n",AreaList[i].alignment);
    for(j = 0;j < 10;j++)
      fprintf(file,"skill %i %s\n",j,AreaList[i].skills[j]);
    fprintf(file,"enclevel %i\n",AreaList[i].encounter_level);
    fprintf(file,"encrate %i\n",AreaList[i].encounter_rate);
    fprintf(file,"searcheffort %i\n",AreaList[i].searchcost);
    for(j = 0;j < 5;j++)
      fprintf(file,"species %i %i\n",j,AreaList[i].races_encountered[j]);
    for(j = 0;j < 5;j++)
      fprintf(file,"aspect %i %i\n",j,AreaList[i].aspects_encountered[j]);
    for(j = 0;j < 3;j++)
      fprintf(file,"alignment %i %i\n",j,AreaList[i].alignments_encountered[j]);
    fprintf(file,"end");
  }
  fclose(file);
}


void LoadAreasText()
{
  int i;
  int temp;
  char buf[128];
  char *c;
  FILE *file;
  file = fopen("system/arealist.cfg","r");
  if(file == NULL)
  {
    fprintf(stderr,"unable to open area file!: %s\n",SDL_GetError());
    return;
  }
  memset(AreaList,-1,sizeof(Area) * MAXAREAS);
  i = 0;
  while((fscanf(file, "%s", buf) != EOF)&&(i < MAXAREAS))
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
      fgets(AreaList[i].uname, 80, file);
      c = strchr(AreaList[i].uname, '\n');
      if(c != NULL) *c = '\0';    /*replace trailing return with terminating character*/
      continue;
    }
    if(strcmp(buf,"displayname") ==0)
    {
      fgetc(file);  /*clear the space before the word*/
      fgets(AreaList[i].displayname, 80, file);
      c = strchr(AreaList[i].displayname, '\n');
      if(c != NULL) *c = '\0';    /*replace trailing return with terminating character*/
      continue;
    }
    if(strcmp(buf,"iconimage") ==0)
    {
      fgetc(file);  /*clear the space before the word*/
      fgets(AreaList[i].iconname, 80, file);
      c = strchr(AreaList[i].iconname, '\n');
      if(c != NULL) *c = '\0';    /*replace trailing return with terminating character*/
      continue;
    }
    if(strcmp(buf,"bgimage") ==0)
    {
      fgetc(file);  /*clear the space before the word*/
      fgets(AreaList[i].imagename, 80, file);
      c = strchr(AreaList[i].imagename, '\n');
      if(c != NULL) *c = '\0';    /*replace trailing return with terminating character*/
      continue;
    }
    if(strcmp(buf,"bgmusic") ==0)
    {
      fgetc(file);  /*clear the space before the word*/
      fgets(AreaList[i].bgmusic, 80, file);
      c = strchr(AreaList[i].bgmusic, '\n');
      if(c != NULL) *c = '\0';    /*replace trailing return with terminating character*/
      continue;
    }
    if(strcmp(buf,"food") ==0)
    {
      fscanf(file, "%i", &AreaList[i].foodavailable);
      continue;
    }
    if(strcmp(buf,"bgelement") ==0)
    {
      fscanf(file, "%i", &temp);
      fgetc(file);  /*clear the space before the word*/
      fscanf(file, "%i", &AreaList[i].elements[temp]);
      continue;
    }
    if(strcmp(buf,"activity") ==0)
    {
      fscanf(file, "%i", &temp);
      fgetc(file);  /*clear the space before the word*/
      fscanf(file, "%s", AreaList[i].activities[temp]);
      c = strchr(AreaList[i].activities[temp], '\n');
      if(c != NULL) *c = '\0';    /*replace trailing return with terminating character*/
      fscanf(file, "%i", &AreaList[i].act_challenge[temp]);
      continue;
    }
    if(strcmp(buf,"exitcondition") ==0)
    {
      fscanf(file, "%i", &temp);
      fgetc(file);  /*clear the space before the word*/
      fscanf(file, "%s", AreaList[i].exit_conditions[temp]);
      c = strchr(AreaList[i].exit_conditions[temp], '\n');
      if(c != NULL) *c = '\0';    /*replace trailing return with terminating character*/
      continue;
    }
    if(strcmp(buf,"exit") ==0)
    {
      fscanf(file, "%i", &temp);
      fgetc(file);  /*clear the space before the word*/
      fscanf(file, "%s", AreaList[i].exits[temp]);
      c = strchr(AreaList[i].exits[temp], '\n');
      if(c != NULL) *c = '\0';    /*replace trailing return with terminating character*/
      continue;
    }
    if(strcmp(buf,"bgalignment") ==0)
    {
      fscanf(file, "%i", &AreaList[i].alignment);
      continue;
    }
    if(strcmp(buf,"skill") ==0)
    {
      fscanf(file, "%i", &temp);
      fgetc(file);  /*clear the space before the word*/
      fscanf(file, "%s", AreaList[i].skills[temp]);
      c = strchr(AreaList[i].skills[temp], '\n');
      if(c != NULL) *c = '\0';    /*replace trailing return with terminating character*/
      continue;
    }
    if(strcmp(buf,"enclevel") ==0)
    {
      fscanf(file, "%i", &AreaList[i].encounter_level);
      continue;
    }
    if(strcmp(buf,"encrate") ==0)
    {
      fscanf(file, "%i", &AreaList[i].encounter_rate);
      continue;
    }
    if(strcmp(buf,"searcheffort") ==0)
    {
      fscanf(file, "%i", &AreaList[i].searchcost);
      continue;
    }
    if(strcmp(buf,"species") ==0)
    {
      fscanf(file, "%i", &temp);
      fgetc(file);  /*clear the space before the word*/
      fscanf(file, "%i", &AreaList[i].races_encountered[temp]);
      continue;
    }
    if(strcmp(buf,"apect") ==0)
    {
      fscanf(file, "%i", &temp);
      fgetc(file);  /*clear the space before the word*/
      fscanf(file, "%i", &AreaList[i].aspects_encountered[temp]);
      continue;
    }
    if(strcmp(buf,"alignment") ==0)
    {
      fscanf(file, "%i", &temp);
      fgetc(file);  /*clear the space before the word*/
      fscanf(file, "%i", &AreaList[i].alignments_encountered[temp]);
      continue;
    }
  }
  fclose(file);
  NumAreas = i;
}




/*eol @ eof*/

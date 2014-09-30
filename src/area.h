#ifndef __AREA__
#define __AREA__
/*
  Area definitions
from the design doc:
Areas offer:
  (background image)
  random encounters based on surrounding
  Chance to learn new skills.
  different training exercises
  background elements
  random events that may shape alignment
  access to other areas under conditions...
    a skill type
    a growth level (min or max)
    a resistance level

*/

#include "graphics.h"

/*outside figure, when game is done, I will lower this number*/
#define MAXAREAS  512

enum Compass_Dir {CD_N,CD_S,CD_E,CD_W,CD_NE,CD_SE,CD_NW,CD_SW};

typedef struct
{
  char uname[80];                 /*the unique name to search by plains002*/
  char displayname[80];           /*the name to display in the area info window*/
  char iconname[80];              /*the name of the icon file to display*/
  char imagename[80];             /*the name of the sprite used for the background*/
  char bgmusic[80];               /*the name of the background music file to play*/
  int  foodavailable;             /*how much food is available when searching*/
  int  elements[5];               /*how much elemental influence the area has*/
  char activities[5][80];         /*activities by name*/
  int  act_challenge[5];          /*challenge for activities*/
  char exits[8][80];              /*exits to adjacent areas by name.*/
  char exit_conditions[8][80];    /*the condition needed to use an exit*/
  int  alignment;                 /*any background alignment influence*/
  char skills[10][80];            /*skills that may be learned in this area, if conditions met*/
  int  encounter_level;            /*level of monsters encountered*/
  int  races_encountered[5];      /*types of races met*/
  int  aspects_encountered[5];    /*types of monsters with set aspects met*/
  int  alignments_encountered[3]; /*alignments of mosnters encountered*/
  int  encounter_rate;            /*how often monsters are encountered  The higher the more often*/
  int  searchcost;                /*how much stamina is spent to search*/
}Area;

typedef struct
{
  int inuse;
  char uname[80];                 /*the unique name to search by plains002*/
  char exitsfound[8][80];         /*the names of the exits found in order found*/
  int foundexitcount;
  char trainsfound[5][80];        /*The names of the training activities found*/
  int foundactivitycount;
  int timespent;                  /*how much time has been spent here*/
}AreaHistory;

Area *GetAreaByUName(char uname[80]);
int  GetAreaIndexByUName(char uname[80]);

int  GetExitIndexByUName(char uname[80]);
int  GetExitsFound();
int  FindExit();
void FindExitByUName(char uname[80]);
char *GetDirectionName(int dir);

int  GetTrainIndexByUName(char uname[80]);
void FindTrainByUName(char uname[80]);
int  GetTrainsFound();
int  GetFoundTrainByUname(char uname[80]);
int  FindTrain();
void GoTrainArea(int train);
void SetupPrenatalCareMenu();

void SearchArea();


void AreaSelect();      /*window option*/
void SetupActionTrainMenu();

void LoadArea(char filename[80]);  /*sets up the area based on the current player stats and the area of the name specified.*/
void LoadAreasText();
void SaveAreasText();
void LoadAreasBinary();
void SaveAreasBinary();



#endif

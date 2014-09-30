#ifndef __HUD__
#define __HUD__
/*
Functions for handling the HUD and parsing commands
*/

#include "entity.h"

enum Button_ID  {BI_NewGame, BI_LoadGame, BI_Quit, BI_Care, BI_Status, BI_World, BI_Feed, BI_Train, BI_Rest, BI_Skills, BI_Stats, BI_Stage, BI_Search, BI_Move, BI_Link, BI_CloseWin, BI_Next,BI_Prev, BI_Select, BI_Yes, BI_No, BI_OK, BI_ListStart = 100,BI_Letter = 1000};
            /*BI_Letter must remain the last in the list*/

#define HUDBUTTONS 40


typedef struct
{
  Sprite *sprite;
  Uint32 buttons;
  Uint32 oldbuttons;
  Uint32 state;
  Uint32 shown;
  Uint32 frame;
  Uint16  x, y;
  Uint16 ox,oy;
}Mouse_T;

typedef struct
{
  Sprite *button;
  Sprite *buttons[3];
  int buttonID;
  SDL_Rect box;
  char text[80];
  int state;
  int frame;
  int hotkey;
  int shown;
}Button;

typedef struct
{
  Button buttons[HUDBUTTONS];
  int numbuttons;
  int buttonfocus;
  int state;
  int subwindow;
  int subwindowinfo;
  int readyreturn;
  int submenu;
  Sprite *image;
  char windowheader[80];
  char description[120];
  char *editstring;
  void (*windowdraw)();
  void (*windowupdate)(int pressID);
  void (*optionrun[10])();
}HUDInfo;

void LoadHUD();
void KillHUD();
void DrawHUD();
void UpdateHUD();
void SetButton(Button *button,int buttonID,int hotkey, char *text,Sprite *sprite,Sprite *sprite1,Sprite *sprite2,Sprite *sprite3,int x,int y,int w,int h,int shown,int frame);

void SetupMainMenu();
void SetupGameMenu();
void DrawListMenuSkill();

void DrawStatusBarHoriz(int stat,int range,int FG,int BG,int x, int y, int w, int h);

/*
 returns a pointer to an integer that will be changed from -1 to either 1 or 0 when selected.
 Takes two function pointers that will be called for either the yes event or no event
*/
int *YesNo(char message[80],void (*yes)(),void (*no)());
int *TwoOptions(char message[80],char op1[80],char op2[80],void (*option1)(),void (*option2)());
int *TextDialog(char message[80],char *text,void (*option)());
int *IPDialog(char message[80],char *text,void (*option)());
void DrawWindow(int size,int x,int y);

void InfoWindow(char title[80],Sprite *image,int frame,char description[120]);

void InitMouse();
void DrawMouse();
void KillMouse();
Uint32 MouseIn(SDL_Rect rect); /*returns the button mask IF the mouse is in the Rectangle*/
int MouseOver(SDL_Rect rect);

#endif

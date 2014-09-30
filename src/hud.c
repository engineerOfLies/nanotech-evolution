#include "hud.h"
#include "monster.h"
#include "area.h"
#include "combat.h"
#include "skills.h"
#include "events.h"
#include "net.h"

enum Button_States {B_Normal,B_Press,B_Highlight};

extern Entity *Player;
extern SDL_Surface *screen;
extern SaveInfo PlayerStats[MAXPLAYERS];
extern int GameBegun;
extern Area *CurrentArea;
extern int EventMode;
int keymoved = 0;


Mouse_T Mouse;
HUDInfo hud;
Sprite *buttonsprites[3];
Sprite *buttonarrows[3];
Sprite *windowsprite;
Sprite *mediumwindowsprite;
Sprite *elements;
Sprite *attributes;

/*
  Button Maintenance
*/

void SetButton(Button *button,int buttonID,int hotkey, char *text,Sprite *sprite,Sprite *sprite1,Sprite *sprite2,Sprite *sprite3,int x,int y,int w,int h,int shown,int frame)
{
  strcpy(button->text,text);
  button->button = sprite;
  button->hotkey = hotkey;
  button->buttons[0] = sprite1;
  button->buttons[1] = sprite2;
  button->buttons[2] = sprite3;
  button->buttonID = buttonID;
  button->frame = frame;
  button->box.x = x;
  button->box.y = y;
  button->box.w = w;
  button->box.h = h;
  button->shown = shown;
}

void ModifyButton(Button *button,int state,int shown)
{
  button->button = buttonsprites[state];
  button->shown = shown;
}

void ResetButtons()
{
  int j;
  for(j = 0;j < HUDBUTTONS;j++)
  {
    ModifyButton(&hud.buttons[j],B_Normal,0);
  }
}

void ResetFocus(int button)
{
  int i;
  hud.buttonfocus = button;
  for(i = 0;i < HUDBUTTONS;i++)
  {
    if(button == i)
    {
      hud.buttons[i].button = hud.buttons[i].buttons[2];
    }
    else
    {           /*pressed buttons are not highlighted*/
      if((hud.buttons[i].shown)&&(hud.buttons[i].button != hud.buttons[i].buttons[1]))
        hud.buttons[i].button = hud.buttons[i].buttons[0];
    }
  }
}

/*

  Window Maintenance

*/

void DrawStatusBarHoriz(int stat,int range,int FG,int BG,int x, int y, int w, int h)
{
  float percent;
  DrawFilledRect(x,y, w, h, IndexColor(BG),screen);
  if((stat > 0)&&(range != 0))
  {
    percent = (stat * w) / range;
    DrawFilledRect(x,y, percent, h, IndexColor(FG),screen);
  }

}

void DrawWindow(int size,int x, int y)
{
  switch(size)
  {
    case 1:
      DrawSprite(mediumwindowsprite,screen,x,y,0);
      break;
    case 2:
      DrawSprite(windowsprite,screen,x,y,0);
      break;
  }
}

/*
 *    Text Windows
 */
 
void UpdateTextWindow(int pressID)
{
  if(pressID == -1)return;
  if(pressID == BI_OK)
  {
    hud.readyreturn = 1;
    if(hud.optionrun[0])hud.optionrun[0]();
    SetupGameMenu();
    keymoved = 2;
    return;
  }
  if(pressID == BI_Prev)
  {
    if(hud.subwindowinfo > 0)
    {
      hud.editstring[--hud.subwindowinfo] = '\0';
      keymoved = 2;
    }
  }
  if(pressID == BI_Next)
  {
    if(hud.subwindowinfo < 79)
    {
      hud.editstring[hud.subwindowinfo++] = ' ';
      hud.editstring[hud.subwindowinfo] = '\0';
      keymoved = 2;
    }
  }
  if((pressID >= BI_Letter) && (pressID < BI_Letter + 26))
  {
    hud.editstring[hud.subwindowinfo++] = pressID - BI_Letter + 'a';
    hud.editstring[hud.subwindowinfo] = '\0';
    keymoved = 2;
  }
  if((pressID >= BI_Letter + 26) && (pressID < BI_Letter + 36))
  {
    hud.editstring[hud.subwindowinfo++] = pressID - (BI_Letter + 26) + '0';
    hud.editstring[hud.subwindowinfo] = '\0';
    keymoved = 2;
  }
}


void DrawTextWindow()
{
  char message[80];
  DrawSprite(mediumwindowsprite,screen,30,200,0);
  DrawSprite(windowsprite,screen,30,20,0);
  DrawTextCentered(hud.windowheader,screen,120,32,IndexColor(DarkGrey),F_Large);
  DrawTextCentered(hud.windowheader,screen,119,31,IndexColor(Green),F_Large);
  if(hud.editstring[0] == '\0')
  {
    DrawText("_",screen,50,228,IndexColor(DarkGrey),F_Large);
    DrawText("_",screen,49,227,IndexColor(LightGreen),F_Large);
  }
  else
  {
    sprintf(message,"%s_",hud.editstring);
    DrawText(message,screen,50,228,IndexColor(DarkGrey),F_Large);
    DrawText(message,screen,49,227,IndexColor(LightGreen),F_Large);
  }
}

int *TextDialog(char message[80],char *text,void (*option)())
{
  int i;
  char letter[2];
  hud.numbuttons = 39;
  hud.buttonfocus = 0;
  hud.subwindow = 1;
  hud.subwindowinfo = strlen(text);
  hud.readyreturn = -1;
  hud.optionrun[0] = option;
  hud.editstring = text;
  strncpy(hud.windowheader,message,80);
  for(i = 0;i < 26;i++)
  {
    sprintf(letter,"%c",'a' + i);
    SetButton(&hud.buttons[i],BI_Letter + i,(SDLK_a + i),letter,NULL,NULL,NULL,NULL,48 + (24 *(i % 6)),56 + (24 *(i / 6)),16,16,1,0);
  }
  for(i = 26;i < 36;i++)
  {
    sprintf(letter,"%c",'0' + (i - 26));
    SetButton(&hud.buttons[i],BI_Letter + i,(SDLK_0 + (i - 26)),letter,NULL,NULL,NULL,NULL,48 + (24 *(i % 6)),56 + (24 *(i / 6)),16,16,1,0);
  }
  SetButton(&hud.buttons[36],BI_OK,SDLK_RETURN,"Done",NULL,NULL,NULL,NULL,122,240,74,64,1,0);
  SetButton(&hud.buttons[37],BI_Prev,SDLK_BACKSPACE," ",buttonarrows[0],buttonarrows[0], buttonarrows[1], buttonarrows[2],100,194,16,16,1,1);
  SetButton(&hud.buttons[38],BI_Next,SDLK_SPACE," ",buttonarrows[0],buttonarrows[0], buttonarrows[1], buttonarrows[2],124,194,16,16,1,3);
  hud.windowdraw = DrawTextWindow;
  hud.windowupdate = UpdateTextWindow;
  return &hud.readyreturn;
}

void CloseWin(int pressID)
{
  if(pressID == BI_CloseWin)
  {
    SetupGameMenu();
    keymoved = 2;
    return;
  }
}

void DrawInfoWin()
{
  int height;
  DrawSprite(windowsprite,screen,30,20,0);
  DrawTextCentered(hud.windowheader,screen,120,32,IndexColor(DarkGrey),F_Large);
  DrawTextCentered(hud.windowheader,screen,119,31,IndexColor(Green),F_Large);
  if(hud.image == NULL)height = 50;
  else
  {
    DrawSprite(hud.image,screen,120 - (hud.image->w /2), 50,hud.subwindowinfo);
    height = 60 + hud.image->h;
  }
  DrawTextBlock(hud.description,screen,38, height,IndexColor(LightBlue),F_Small,172);
}


void InfoWindow(char title[80],Sprite *image,int frame,char description[120])
{
  hud.numbuttons = 1;
  hud.buttonfocus = 0;
  hud.subwindow = 1;
  hud.subwindowinfo = frame;
  hud.image = image;
  strncpy(hud.windowheader,title,80);
  strncpy(hud.description,description,80);
  SetButton(&hud.buttons[0],BI_CloseWin, -1," ",buttonarrows[0],buttonarrows[0], buttonarrows[1], buttonarrows[2],182,18,16,16,1,5);
  hud.windowdraw = DrawInfoWin;
  hud.windowupdate = CloseWin;
}


/*
    IP Windows
*/

void UpdateIPWindow(int pressID)
{
  if(pressID == -1)return;
  if(pressID == BI_OK)
  {
    hud.readyreturn = 1;
    if(hud.optionrun[0])hud.optionrun[0]();
    keymoved = 2;
    return;
  }
  if(pressID == BI_Prev)
  {
    if(hud.subwindowinfo > 0)
    {
      hud.editstring[--hud.subwindowinfo] = '\0';
      keymoved = 2;
    }
  }
  if(hud.subwindowinfo < 15)
  {
    if(pressID == BI_Next)
    {
      if(hud.subwindowinfo < 15)
      {
        hud.editstring[hud.subwindowinfo++] = ' ';
        hud.editstring[hud.subwindowinfo] = '\0';
        keymoved = 2;
      }
    }
    if((pressID >= BI_Letter) && (pressID < BI_Letter + 10))
    {
      hud.editstring[hud.subwindowinfo++] = pressID - BI_Letter + '0';
      hud.editstring[hud.subwindowinfo] = '\0';
      keymoved = 2;
    }
    if(pressID == BI_Letter + 10)
    {
      hud.editstring[hud.subwindowinfo++] = '.';
      hud.editstring[hud.subwindowinfo] = '\0';
      keymoved = 2;
    }
  }
}


int *IPDialog(char message[80],char *text,void (*option)())
{
  int i;
  char letter[2];
  hud.numbuttons = 14;
  hud.buttonfocus = 0;
  hud.subwindow = 1;
  hud.subwindowinfo = strlen(text);
  hud.readyreturn = -1;
  hud.optionrun[0] = option;
  hud.editstring = text;
  strncpy(hud.windowheader,message,80);
  for(i = 0;i < 10;i++)
  {
    sprintf(letter,"%c",'0' + i);
    SetButton(&hud.buttons[i],BI_Letter + i,(SDLK_0 + i),letter,NULL,NULL,NULL,NULL,48 + (24 *(i % 6)),56 + (24 *(i / 6)),16,16,1,0);
  }
  i = 10;
  SetButton(&hud.buttons[i],BI_Letter + i,SDLK_PERIOD,".",NULL,NULL,NULL,NULL,48 + (24 *(i % 6)),56 + (24 *(i / 6)),16,16,1,0);
  SetButton(&hud.buttons[11],BI_OK,SDLK_RETURN,"Done",NULL,NULL,NULL,NULL,122,240,74,64,1,0);
  SetButton(&hud.buttons[12],BI_Prev,SDLK_BACKSPACE," ",buttonarrows[0],buttonarrows[0], buttonarrows[1], buttonarrows[2],100,194,16,16,1,1);
  SetButton(&hud.buttons[13],BI_Next,SDLK_SPACE," ",buttonarrows[0],buttonarrows[0], buttonarrows[1], buttonarrows[2],124,194,16,16,1,3);
  hud.windowdraw = DrawTextWindow;
  hud.windowupdate = UpdateIPWindow;
  return &hud.readyreturn;
}


/*
 *    Yes No Windows
*/

void DrawYesNoWindow()
{
  DrawSprite(mediumwindowsprite,screen,30,60,0);
  DrawTextCentered(hud.windowheader,screen,120,68,IndexColor(DarkGrey),F_Large);
  DrawTextCentered(hud.windowheader,screen,119,67,IndexColor(Green),F_Large);
}

void UpdateYesNoWindow(int pressID)
{
  if(pressID == -1)return;
  switch(pressID)
  {
    case BI_Yes:
      hud.readyreturn = 1;
      if(hud.optionrun[0])hud.optionrun[0]();
      else SetupGameMenu();
      keymoved = 2;
      break;
    case BI_No:
      hud.readyreturn = 0;
      if(hud.optionrun[1])hud.optionrun[1]();
      else SetupGameMenu();
      keymoved = 2;
      break;
    case BI_CloseWin:
      hud.readyreturn = 0;
      SetupGameMenu();
      keymoved = 2;
      break;
  }
}

int *YesNo(char message[80],void (*option1)(),void (*option2)())
{
  hud.numbuttons = 2;
  hud.buttonfocus = 0;
  hud.subwindow = 1;
  hud.readyreturn = -1;
  hud.optionrun[0] = option1;
  hud.optionrun[1] = option2;
  strncpy(hud.windowheader,message,80);
  SetButton(&hud.buttons[0],BI_Yes, -1,"Yes",buttonsprites[2],buttonsprites[0],buttonsprites[1], buttonsprites[2],44,90,74,64,1,0);
  SetButton(&hud.buttons[1],BI_No, -1,"No",buttonsprites[2],buttonsprites[0],buttonsprites[1], buttonsprites[2],122,90,74,64,1,0);
  hud.windowdraw = DrawYesNoWindow;
  hud.windowupdate = UpdateYesNoWindow;
  return &hud.readyreturn;
}

int *TwoOptions(char message[80],char op1[80],char op2[80],void (*option1)(),void (*option2)())
{
  hud.numbuttons = 3;
  hud.buttonfocus = 0;
  hud.subwindow = 1;
  hud.readyreturn = -1;
  hud.optionrun[0] = option1;
  hud.optionrun[1] = option2;
  strncpy(hud.windowheader,message,80);
  SetButton(&hud.buttons[0],BI_Yes, -1,op1,buttonsprites[2],buttonsprites[0],buttonsprites[1], buttonsprites[2],44,90,74,64,1,0);
  SetButton(&hud.buttons[1],BI_No, -1,op2,buttonsprites[2],buttonsprites[0],buttonsprites[1], buttonsprites[2],122,90,74,64,1,0);
  SetButton(&hud.buttons[2],BI_CloseWin, -1," ",buttonarrows[0],buttonarrows[0], buttonarrows[1], buttonarrows[2],182,68,16,16,1,5);
  hud.windowdraw = DrawYesNoWindow;
  hud.windowupdate = UpdateYesNoWindow;
  return &hud.readyreturn;
}


/*
 *  Stat windows
 */
void UpdateElementWindow(int pressID)
{
  if(pressID != -1)
  {
    switch(pressID)
    {
      case BI_CloseWin:
        SetupGameMenu();
        break;
      case BI_Next:
        hud.subwindowinfo = (hud.subwindowinfo + 1) % 5;
        break;
      case BI_Prev:
        hud.subwindowinfo--;
        if(hud.subwindowinfo < 0)hud.subwindowinfo = 4;
        break;
    }
  }
}

void DrawElementWindow()
{
  int i;
  int percent;
  char message[80];
  DrawSprite(windowsprite,screen,30,30,0);
  switch(hud.subwindowinfo)
  {
    case 0:
      DrawTextCentered("Condition",screen,120,38,IndexColor(DarkGrey),F_Large);
      DrawTextCentered("Condition",screen,119,37,IndexColor(LightGreen),F_Large);
      DrawTextCentered(ThePlayerStats.stagename,screen,120,60,IndexColor(DarkGrey),F_Large);
      DrawTextCentered(ThePlayerStats.stagename,screen,119,59,IndexColor(Green),F_Large);
      
      sprintf(message,"Age: %i",PlayerStats[Player->playernum].Age );
      DrawText(message,screen,61,80,IndexColor(DarkGrey),F_Medium);
      DrawText(message,screen,60,79,IndexColor(Green),F_Medium);
      
      DrawSprite(attributes,screen,60,96,6);
      sprintf(message,"%i / %i",(int)PlayerStats[Player->playernum].health,PlayerStats[Player->playernum].healthmax );
      DrawText(message,screen,79,97,IndexColor(DarkGrey),F_Medium);
      DrawText(message,screen,78,96,IndexColor(Green),F_Medium);
      DrawSprite(attributes,screen,60,97 + 24,7);
      sprintf(message,"%i / %i",(int)PlayerStats[Player->playernum].stamina,PlayerStats[Player->playernum].staminamax );
      DrawText(message,screen,79,97 + 24,IndexColor(DarkGrey),F_Medium);
      DrawText(message,screen,78,96 + 24,IndexColor(Green),F_Medium);
      DrawSprite(attributes,screen,60,96 + 48,8);
      sprintf(message,"%i / %i",(int)PlayerStats[Player->playernum].fullness,PlayerStats[Player->playernum].stage * 25 );
      DrawText(message,screen,79,97 + 48,IndexColor(DarkGrey),F_Medium);
      DrawText(message,screen,78,96 + 48,IndexColor(Green),F_Medium);
      sprintf(message,"Food: %i",PlayerStats[Player->playernum].foodsupply );
      DrawText(message,screen,61,96 + 48 + 20,IndexColor(DarkGrey),F_Medium);
      DrawText(message,screen,60,97 + 48 + 20,IndexColor(Green),F_Medium);
      break;
    case 1:
      DrawTextCentered("Attributes",screen,120,38,IndexColor(DarkGrey),F_Large);
      DrawTextCentered("Attributes",screen,119,37,IndexColor(Green),F_Large);
      for(i = 0;i < 6;i++)
      {
        DrawSprite(attributes,screen,60,66 + (24 * i),i);
        sprintf(message,"%i",PlayerStats[Player->playernum].attributes[i]);
        DrawText(message,screen,79,67 + (24 * i),IndexColor(DarkGrey),F_Medium);
        if(PlayerStats[Player->playernum].attributetrains[i] <= 0)
          DrawText(message,screen,78,66 + (24 * i),IndexColor(DarkGreen),F_Medium);
        else if(PlayerStats[Player->playernum].attributetrains[i] < 10)
          DrawText(message,screen,78,66 + (24 * i),IndexColor(Green),F_Medium);
        else DrawText(message,screen,78,66 + (24 * i),IndexColor(LightGreen),F_Medium);
      }
      break;
    case 2:
      DrawTextCentered("Elements",screen,120,38,IndexColor(DarkGrey),F_Large);
      DrawTextCentered("Elements",screen,119,37,IndexColor(Green),F_Large);
      for(i = 0;i < 5;i++)
      {
        DrawSprite(elements,screen,60,56 + (33 * i),i);
        sprintf(message,"%i",(int)PlayerStats[Player->playernum].Element[i]);
        DrawText(message,screen,79,57 + (33 * i),IndexColor(DarkGrey),F_Medium);
        DrawText(message,screen,78,56 + (33 * i),IndexColor(Green),F_Medium);
          DrawSprite(elements,screen,75,56 + (33 * i) + 16,i + 5);
      }
      DrawSprite(elements,screen,120,74,12);/*good*/
      if(ThePlayerStats.good)
      {
        percent = (ThePlayerStats.good * 0.001)*100;
        if(percent <= 0)percent = 1;
        DrawFilledRect(140,130 - (percent), 16, percent, IndexColor(LightYellow),screen);
      }
      DrawSprite(elements,screen,120,122,11);/*balance*/
      DrawSprite(elements,screen,120,170,10);/*evil*/
      if(ThePlayerStats.evil)
      {
        percent = (ThePlayerStats.evil * 0.001)*100;
        if(percent <= 0)percent = 1;
        DrawFilledRect(140,130, 16, percent, IndexColor(LightViolet),screen);
      }
      break;
    case 3:
      DrawTextCentered("Attacks",screen,120,38,IndexColor(DarkGrey),F_Large);
      DrawTextCentered("Attacks",screen,119,37,IndexColor(Green),F_Large);
      for(i = 0;i < PlayerStats[Player->playernum].numattacks;i++)
      {
        sprintf(message,"%s",PlayerStats[Player->playernum].attacks[i]);
        DrawText(message,screen,79,57 + (20 * i),IndexColor(DarkGrey),F_Medium);
        DrawText(message,screen,78,56 + (20 * i),IndexColor(Green),F_Medium);
      }
      break;
    case 4:
      DrawTextCentered("Other Skills",screen,120,38,IndexColor(DarkGrey),F_Large);
      DrawTextCentered("Other Skills",screen,119,37,IndexColor(Green),F_Large);
      for(i = 0;i < PlayerStats[Player->playernum].numdefenses;i++)
      {
        sprintf(message,"%s",PlayerStats[Player->playernum].defenses[i]);
        DrawText(message,screen,79,57 + (20 * i),IndexColor(DarkGrey),F_Medium);
        DrawText(message,screen,78,56 + (20 * i),IndexColor(Green),F_Medium);
      }
      break;
  }
}

void SetupElementWindow(int startwin)
{
  hud.numbuttons = 3;
  hud.buttonfocus = 2;
  hud.subwindow = 1;
  hud.subwindowinfo = startwin;
  SetButton(&hud.buttons[0],BI_CloseWin,-1," ",buttonarrows[0],buttonarrows[0], buttonarrows[1], buttonarrows[2],172,38,16,16,1,5);
  SetButton(&hud.buttons[1],BI_Prev,-1," ",buttonarrows[0],buttonarrows[0], buttonarrows[1], buttonarrows[2],38,130,16,16,1,1);
  SetButton(&hud.buttons[2],BI_Next,-1," ",buttonarrows[2],buttonarrows[0], buttonarrows[1], buttonarrows[2],186,130,16,16,1,3);
  hud.windowdraw = DrawElementWindow;
  hud.windowupdate = UpdateElementWindow;
}


/*

  HUD Maintenance

*/

void LoadHUD()
{
  buttonsprites[0] = LoadSprite("images/button_large.png",74,64);
  buttonsprites[1] = LoadSprite("images/button_large_pressed.png",74,64);
  buttonsprites[2] = LoadSprite("images/button_large_highlighted.png",74,64);
  windowsprite = LoadSprite("images/window_large.png",180,200);
  SDL_SetAlpha(windowsprite->image,SDL_SRCALPHA|SDL_RLEACCEL, 192);
  mediumwindowsprite = LoadSprite("images/window_medium.png",180,100);
  SDL_SetAlpha(mediumwindowsprite->image,SDL_SRCALPHA|SDL_RLEACCEL, 192);
  elements = LoadSprite("images/elements_tiny.png",16,16);
  attributes = LoadSprite("images/attributes_tiny.png",16,16);
  buttonarrows[0] = LoadSwappedSprite("images/arrows.png",16,16,Green,Green,Green);
  buttonarrows[1] = LoadSwappedSprite("images/arrows.png",16,16,DarkGreen,DarkGreen,LightGreen);
  buttonarrows[2] = LoadSwappedSprite("images/arrows.png",16,16,LightGreen,LightGreen,LightGreen);
}

void SetupGameMenu()
{
  int i;
  hud.subwindow = 0;
  hud.numbuttons = 12;
  hud.buttonfocus = 0;
  hud.subwindowinfo = 0;
  hud.readyreturn = -1;
  hud.submenu = 0;
  strncpy(hud.windowheader," ",80);
  hud.editstring = NULL;
  hud.windowdraw = NULL;
  hud.windowupdate = NULL;
  for(i = 0; i < 10; i++)
    hud.optionrun[i] = NULL;
  SetButton(&hud.buttons[0],BI_Care,-1,"Care",buttonsprites[2],buttonsprites[0],buttonsprites[1], buttonsprites[2],4,250,74,64,1,0);
  SetButton(&hud.buttons[1],BI_Status,-1,"Status",buttonsprites[0],buttonsprites[0],buttonsprites[1], buttonsprites[2],82,250,74,64,1,0);
  SetButton(&hud.buttons[2],BI_World,-1,"World",buttonsprites[0],buttonsprites[0],buttonsprites[1], buttonsprites[2],160,250,74,64,1,0);
  SetButton(&hud.buttons[3],BI_Feed,-1,"Feed",buttonsprites[0],buttonsprites[0],buttonsprites[1], buttonsprites[2],4,52,74,64,0,0);
  SetButton(&hud.buttons[4],BI_Train,-1,"Train",buttonsprites[0],buttonsprites[0],buttonsprites[1], buttonsprites[2],4,52 + 66,74,64,0,0);
  if(Player->asleep)
    SetButton(&hud.buttons[5],BI_Rest,-1,"Wake",buttonsprites[0],buttonsprites[0],buttonsprites[1], buttonsprites[2],4,52 + 132,74,64,0,0);
  else
    SetButton(&hud.buttons[5],BI_Rest,-1,"Rest",buttonsprites[0],buttonsprites[0],buttonsprites[1], buttonsprites[2],4,52 + 132,74,64,0,0);
  SetButton(&hud.buttons[6],BI_Skills,-1,"Skills",buttonsprites[0],buttonsprites[0],buttonsprites[1], buttonsprites[2],82,52,74,64,0,0);
  SetButton(&hud.buttons[7],BI_Stats,-1,"Stats",buttonsprites[0],buttonsprites[0],buttonsprites[1], buttonsprites[2],82,52 + 66,74,64,0,0);
  SetButton(&hud.buttons[8],BI_Stage,-1,"Stage",buttonsprites[0],buttonsprites[0],buttonsprites[1], buttonsprites[2],82,52 + 132,74,64,0,0);
  SetButton(&hud.buttons[9],BI_Search,-1,"Search",buttonsprites[0],buttonsprites[0],buttonsprites[1], buttonsprites[2],160,52,74,64,0,0);
  SetButton(&hud.buttons[10],BI_Move,-1,"Move",buttonsprites[0],buttonsprites[0],buttonsprites[1], buttonsprites[2],160,52 + 66,74,64,0,0);
  SetButton(&hud.buttons[11],BI_Link,-1,"Link",buttonsprites[0],buttonsprites[0],buttonsprites[1], buttonsprites[2],160,52 + 132,74,64,0,0);
}

void SetupMainMenu()
{
  hud.numbuttons = 3;
  hud.buttonfocus = 0;
  SetButton(&hud.buttons[0],BI_NewGame,-1,"New",buttonsprites[2],buttonsprites[0],buttonsprites[1], buttonsprites[2],4,152,74,64,1,0);
  SetButton(&hud.buttons[1],BI_LoadGame,-1,"Load",buttonsprites[0],buttonsprites[0],buttonsprites[1], buttonsprites[2],82,152,74,64,1,0);
  SetButton(&hud.buttons[2],BI_Quit,-1,"Quit",buttonsprites[0],buttonsprites[0],buttonsprites[1], buttonsprites[2],160,152,74,64,1,0);
}

void KillHUD()
{
  int i = 0;
  for(i = 0; i < 3;i++)
  {
    FreeSprite(buttonsprites[i]);
    buttonsprites[i] = NULL;
  }
}

void DrawHUD()
{
  int i;
  int color;
  if(hud.subwindow)
  {
    if(hud.windowdraw != NULL)
    {
      hud.windowdraw();
    }
  }
  else
  {
    if(Player != NULL)
    {
      if(strlen(ThePlayerStats.name) > 0)
      {
        DrawWindow(1,30,-60);
        DrawTextCentered(ThePlayerStats.name,screen,120,18,IndexColor(DarkGrey),F_Large);
        if((ThePlayerStats.good - ThePlayerStats.evil) > ((ThePlayerStats.good + ThePlayerStats.evil)/2))
          DrawTextCentered(ThePlayerStats.name,screen,119,17,IndexColor(LightYellow),F_Large);
        if((ThePlayerStats.evil - ThePlayerStats.good) > ((ThePlayerStats.good + ThePlayerStats.evil)/2))
          DrawTextCentered(ThePlayerStats.name,screen,119,17,IndexColor(Violet),F_Large);
        else DrawTextCentered(ThePlayerStats.name,screen,119,17,IndexColor(LightGreen),F_Large);
      }
      else DrawWindow(1,30,-70);
      DrawStatusBarHoriz(ThePlayerStats.health, ThePlayerStats.healthmax, LightRed, Red, 40, 2, 160, 4);
      DrawStatusBarHoriz(ThePlayerStats.stamina, ThePlayerStats.staminamax, Cyan, Blue, 40, 6, 160, 4);
      if(ThePlayerStats.fatigue < 1000)
        DrawStatusBarHoriz(ThePlayerStats.fatigue, 2000, LightGreen, DarkGreen, 40, 10, 160, 4);
      else DrawStatusBarHoriz(ThePlayerStats.fatigue, 2000, Green, DarkGreen, 40, 10, 160, 4);
      if(ThePlayerStats.stage > 0)
      {
        if(ThePlayerStats.fullness > (ThePlayerStats.stage * 25))
          DrawStatusBarHoriz(100, 100, Yellow, DarkBlue, 40, 14, 160, 4);
        else DrawStatusBarHoriz(ThePlayerStats.fullness, (ThePlayerStats.stage * 25), LightViolet, Violet, 40, 14, 160, 4);
      }
    }
  }
  for(i = 0;i < hud.numbuttons;i++)
  {
    if(hud.buttons[i].shown)
    {
      DrawSprite(hud.buttons[i].button,screen,hud.buttons[i].box.x,hud.buttons[i].box.y - hud.buttons[i].state, hud.buttons[i].frame);
      if(strlen(hud.buttons[i].text) > 0)
      {
        if(hud.buttonfocus == i)color = LightGreen;
        else color = Green;
        DrawTextCentered(hud.buttons[i].text,screen,1 + hud.buttons[i].box.x + (hud.buttons[i].box.w >> 1),hud.buttons[i].box.y + (hud.buttons[i].box.h >> 1) - 10 + 1,IndexColor(DarkGrey),F_Large);
        DrawTextCentered(hud.buttons[i].text,screen,hud.buttons[i].box.x + (hud.buttons[i].box.w >> 1),hud.buttons[i].box.y + (hud.buttons[i].box.h >> 1) - 10,IndexColor(color),F_Large);
      }
    }
  }
}

void UpdateHUD()
{
  int i;
  int temp;
  int mousemoved = 0;
  int keypressnow = 0;
  Uint8 *keys;
  SDLMod mod;
  keys = SDL_GetKeyState(NULL);
  mod = SDL_GetModState();  
  /*handle button focus*/
  if(EventMode)return;
  if(keymoved > 0)
  {
    keymoved--;
    return;
  }
  if((Mouse.x != Mouse.ox)&&(Mouse.y != Mouse.oy))mousemoved = 1;
  for(i = 0;i < HUDBUTTONS;i++)
  {
    if(hud.buttons[i].shown)
    {     /*only check mouse position if it has moved*/
      if((mousemoved)&&(MouseOver(hud.buttons[i].box)))
      {
        ResetFocus(i);
        break;
      }
    }
  }
  if(keymoved <= 0)
  {
    keypressnow = 1;
    if(hud.subwindow)
    {
      if(keys[SDLK_LEFT])
      {
        hud.buttonfocus--;
        if(hud.buttonfocus < 0)hud.buttonfocus = hud.numbuttons - 1;
        keymoved = 2;
        ResetFocus(hud.buttonfocus);

      }
      if(keys[SDLK_RIGHT])
      {
        hud.buttonfocus++;
        if(hud.buttonfocus >= hud.numbuttons)hud.buttonfocus = 0;
        keymoved = 2;
        ResetFocus(hud.buttonfocus);
      }
      if(keys[SDLK_UP])
      {
        hud.buttonfocus -= 6;
        if(hud.buttonfocus < 0)hud.buttonfocus = hud.numbuttons - 1;
        keymoved = 2;
        ResetFocus(hud.buttonfocus);
      }
      if(keys[SDLK_DOWN])
      {
        hud.buttonfocus += 6;
        if(hud.buttonfocus >= hud.numbuttons)hud.buttonfocus = 0;
        keymoved = 2;
        ResetFocus(hud.buttonfocus);
      }
    }
    else
    {
      if((keys[SDLK_LEFT])||(keys[SDLK_RIGHT]))
      {
        if(hud.buttonfocus < 3)
        {
          if((keys[SDLK_LEFT]))
          {
            hud.buttonfocus--;
            if(hud.buttonfocus < 0)hud.buttonfocus = 2;
          }
          if((keys[SDLK_RIGHT]))
          {
            hud.buttonfocus++;
            if(hud.buttonfocus > 2)hud.buttonfocus = 0;
          }
          ResetFocus(hud.buttonfocus);
          keymoved = 2;
        }
      }
      if((keys[SDLK_UP])||(keys[SDLK_DOWN]))
      {
        if(hud.buttonfocus >= 3)
        {
          if((keys[SDLK_UP]))
          {
            if((hud.buttonfocus % 3) == 0)hud.buttonfocus += 2;
            else hud.buttonfocus--;
          }
          if((keys[SDLK_DOWN]))
          {
            if((hud.buttonfocus %3 ) == 2)
            {
              ResetButtons();
              ModifyButton(&hud.buttons[0],B_Normal,1);
              ModifyButton(&hud.buttons[1],B_Normal,1);
              ModifyButton(&hud.buttons[2],B_Normal,1);
              hud.buttonfocus /= 3;
              hud.buttonfocus--;
            }
            else hud.buttonfocus++;
          }
          ResetFocus(hud.buttonfocus);
          keymoved = 2;
        }
      }
    }
  }
  /*check if anything was pressed*/
  for(i = 0;i < HUDBUTTONS;i++)
  {
    if(hud.buttons[i].shown)
    {
      if((MouseIn(hud.buttons[i].box) & SDL_BUTTON(1)) || ((keys[hud.buttons[i].hotkey])&&(!keymoved)) || ((hud.buttonfocus == i)&&(mod & KMOD_CTRL)&&(!keymoved)))
      {
        if(hud.subwindow)
        {
          if(hud.windowupdate != NULL)
          {
            hud.windowupdate(hud.buttons[i].buttonID);
            if(keymoved)return;
          }
        }
        else
        {
          keymoved = 2;
          switch(hud.buttons[i].buttonID)
          {
            case BI_NewGame:
              GameBegun = 1;
              Player = SpawnMonster(1,"egg",0);
              SetupGameMenu();
              return;
            case BI_Feed:
              if(ThePlayerStats.stage == 0)
              {
                NewMessage("Eggs can't eat!",IndexColor(Red));
              }
              else if(ThePlayerStats.health < 0)
              {
                NewMessage("Too injured to eat!",IndexColor(Red));
              }
              else
              {
                temp = ThePlayerStats.foodsupply;
                ThePlayerStats.foodsupply -= (ThePlayerStats.stage * 4);
                NewMessage("Feeding!",IndexColor(White));
                if(ThePlayerStats.foodsupply <= 0)
                {
                  ThePlayerStats.fullness += temp;
                  NewMessage("Out of Food!",IndexColor(Red));
                  ThePlayerStats.foodsupply = 0;
                }
                else
                {
                  ThePlayerStats.fullness += (ThePlayerStats.stage * 4);
                  DoEvent("feeding",NULL);
                }
              }
              SetupGameMenu();
              return;
            case BI_Rest:
              if(Player->asleep)
              {
                YesNo("Wake Monster?",WakeMonster,NULL);
              }
              else
              {
                Player->asleep = 1;
                SetupGameMenu();
                hud.buttonfocus = 0;
              }
              return;
            case BI_Stats:
              SetupElementWindow(1);
              break;
            case BI_Skills:
              SetupElementWindow(3);
              break;
            case BI_Stage:
              SetupElementWindow(0);
              break;
            case BI_Care:
              if(hud.submenu)
              {
                hud.submenu = 0;
                ResetButtons();
                ModifyButton(&hud.buttons[0],B_Highlight,1);
                ModifyButton(&hud.buttons[1],B_Normal,1);
                ModifyButton(&hud.buttons[2],B_Normal,1);
                hud.buttonfocus = 0;
              }
              else
              {
                hud.submenu = 1;
                ResetButtons();
                ModifyButton(&hud.buttons[0],B_Press,1);
                ModifyButton(&hud.buttons[3],B_Highlight,1);
                ModifyButton(&hud.buttons[4],B_Normal,1);
                ModifyButton(&hud.buttons[5],B_Normal,1);
                ModifyButton(&hud.buttons[1],B_Normal,1);
                ModifyButton(&hud.buttons[2],B_Normal,1);
                hud.buttonfocus = 3;
              }
              return;
            case BI_Status:
              if(hud.submenu)
              {
                hud.submenu = 0;
                ResetButtons();
                ModifyButton(&hud.buttons[1],B_Highlight,1);
                ModifyButton(&hud.buttons[0],B_Normal,1);
                ModifyButton(&hud.buttons[2],B_Normal,1);
                hud.buttonfocus = 1;
              }
              else
              {
                hud.submenu = 1;
                ResetButtons();
                ModifyButton(&hud.buttons[1],B_Press,1);
                ModifyButton(&hud.buttons[6],B_Highlight,1);
                ModifyButton(&hud.buttons[7],B_Normal,1);
                ModifyButton(&hud.buttons[8],B_Normal,1);
                ModifyButton(&hud.buttons[0],B_Normal,1);
                ModifyButton(&hud.buttons[2],B_Normal,1);
                hud.buttonfocus = 6;
              }
              return;
            case BI_LoadGame:
              Player = SpawnMonster(0,"egg",0);
              SetupGameMenu();
              GameBegun = 1;
              break;
            case BI_Quit:
              exit(0);
              return;
            case BI_World:
              if(hud.submenu)
              {
                hud.submenu = 0;
                ResetButtons();
                ModifyButton(&hud.buttons[2],B_Highlight,1);
                ModifyButton(&hud.buttons[1],B_Normal,1);
                ModifyButton(&hud.buttons[0],B_Normal,1);
                hud.buttonfocus = 2;
              }
              else
              {
                hud.submenu = 1;
                ResetButtons();
                ModifyButton(&hud.buttons[2],B_Press,1);
                ModifyButton(&hud.buttons[9],B_Highlight,1);
                ModifyButton(&hud.buttons[10],B_Normal,1);
                ModifyButton(&hud.buttons[11],B_Normal,1);
                ModifyButton(&hud.buttons[1],B_Normal,1);
                ModifyButton(&hud.buttons[0],B_Normal,1);
                hud.buttonfocus = 9;
              }
              return;
            case BI_Search:
              SearchArea();
              break;
            case BI_Move:
              if(Player->asleep)
              {
                NewMessage("Can't travel:",IndexColor(LightRed));
                NewMessage("Sleeping!",IndexColor(LightRed));
                SetupGameMenu();
                return;
              }
              if(ThePlayerStats.health < 0)
              {
                NewMessage("Too injured to travel!",IndexColor(Red));
                SetupGameMenu();
                return;
              }
              if(ThePlayerStats.stage <= S_Egg)
              {
                NewMessage("Can't travel:",IndexColor(LightRed));
                NewMessage("Its an Egg!",IndexColor(LightRed));
                SetupGameMenu();
                return;
              }
              if(GetExitsFound() > 0)
                AreaSelect();
              else
              {
                SetupGameMenu();
                NewMessage("No exits found!",IndexColor(LightRed));
                SetupGameMenu();
              }
              break;
            case BI_Train:
              if(Player->asleep)
              {
                NewMessage("Can't train:",IndexColor(LightRed));
                NewMessage("Sleeping!",IndexColor(LightRed));
                SetupGameMenu();
                return;
              }
              if(ThePlayerStats.health < 0)
              {
                NewMessage("Too injured to train!",IndexColor(Red));
                SetupGameMenu();
                return;
              }
              if(ThePlayerStats.stage <= S_Egg)
              {
                SetupPrenatalCareMenu();
              }
              else
              {
                SetupGameMenu();
                SetupTrainWindow();
              }
              return;
            case BI_Link:
              NewMessage("Very soon now...",IndexColor(LightOrange));
              /*TwoOptions("Net Battle","Host","IP",ServerBegin,SetupClient);*/
              return;
          }
        }
      }
    }
  }
}

/*
  Mouse Maintenance functions
*/

int MouseOver(SDL_Rect rect)
{
  if((Mouse.x >= rect.x)&&(Mouse.x < rect.x + rect.w)&&(Mouse.y >= rect.y)&&(Mouse.y < rect.y + rect.h))
    return 1;
  return 0;
}

Uint32 MouseIn(SDL_Rect rect)
{
  if((Mouse.buttons != 0)&&(Mouse.oldbuttons == 0))
  {
    if((Mouse.x >= rect.x)&&(Mouse.x < rect.x + rect.w)&&(Mouse.y >= rect.y)&&(Mouse.y < rect.y + rect.h))
      return Mouse.buttons;
  }
  return 0;
}

void InitMouse()
{
  Mouse.sprite=LoadSprite("images/mouse.png",16,16);
  Mouse.state = 0;
  Mouse.shown = 0;
  Mouse.frame = 0;
}

void KillMouse()
{
  FreeSprite(Mouse.sprite);
  Mouse.sprite = NULL;
}

void DrawMouse()
{
  int mx,my;
  Mouse.oldbuttons = Mouse.buttons;
  Mouse.buttons = SDL_GetMouseState(&mx,&my);
  if(Mouse.sprite != NULL)DrawSprite(Mouse.sprite,screen,mx,my,Mouse.frame);
  Mouse.frame = ((Mouse.frame + 1)%16)+ (16 * Mouse.state);
  Mouse.ox = Mouse.x;
  Mouse.oy = Mouse.y;
  Mouse.x = mx;
  Mouse.y = my;
}


/*eol@eof*/

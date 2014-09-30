#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "SDL.h"
#include "game.h"
#include "graphics.h"
#include "audio.h"
#include "area.h"
#include "monster.h"
#include "hud.h"
#include "skills.h"
#include "particle.h"
#include "combat.h"
#include "events.h"
#include "items.h"
#include "net.h"

extern SDL_Surface *screen;
extern SDL_Surface *bgimage;
extern SDL_Surface *background;
extern SDL_Rect Camera;
extern Entity *Player;
extern int CombatMode;
extern int EventMode;
extern int GAMESPEED;
extern int CanNetwork;

Sprite *icon;
const Uint32 MajorVersion = 0;
const Uint32 MinorVersion = 26;

CameraInfo CamInfo;

SDL_Joystick *joy;

int windowed = 1;
int GameBegun = 0;
int camdir = 1;

Sound *gamesounds[4];

void Init_All();
void Update_ALL();
int Think_ALL();
void Draw_ALL();
void DrawSplashScreen();
void GiveInfo();
void UpdateCamera();
void UpdateMapCamera();
void GoQuit();

int main(int argc, char *argv[])
{
  int done = 0;
  char serverIp[16];
  SDL_Event event;
  char string[40];
  SDLMod mod;
  int l = 1;
  Sprite *lightning = NULL;
  Mix_Chunk *soundtest = NULL;
  Uint8 *keys;
  string[0] = '\0';
  Init_All();
  GiveInfo();
  LoadStageInfoText();  /*Development loading*/
  LoadAreasText();      /*to be switched to binary before release*/
  LoadSkillInfoText();
  LoadItemListText();
  SetupMainMenu();
  memset(serverIp,0,sizeof(serverIp));
  fprintf(stdout,"size of the test structure: %i\n",sizeof(NetStatInfo));
  soundtest = Mix_LoadWAV("sounds/x_light.wav");
  lightning = LoadSprite("images/lightning_sprite.png",64,256);
  do
  {
    ResetBuffer();
    keys = SDL_GetKeyState(NULL);
    mod = SDL_GetModState();
    while(SDL_PollEvent(&event))
    {
      if((event.type == SDL_QUIT)||(keys[SDLK_ESCAPE] == 1))
      {
        if(GameBegun)
        {
          if(!CombatMode)
            YesNo("Save & Exit?",GoQuit,NULL);
          else NewMessage("Can't quit during combat!",IndexColor(LightRed));
        }
        else done = 1;
        break;
      }
      if(keys[SDLK_F2] == 1)
      {
        Mix_PlayChannel(-1,soundtest,0);
      }
    }
    SDL_PumpEvents();
    
    Draw_ALL();


    Update_ALL();
    Think_ALL();
    /*check some global input*/
    if(keys[SDLK_F5] == 1)
    {
      NewMessage("SuperSpeedMode",IndexColor(LightBlue));
      GAMESPEED = 10;
    }
    if(keys[SDLK_F6] == 1)
    {
      NewMessage("SpeedMode",IndexColor(LightBlue));
      GAMESPEED = 100;
    }
    if(keys[SDLK_F7] == 1)
    {
      NewMessage("NormalMode",IndexColor(LightBlue));
      GAMESPEED = 1000;
    }
    if(keys[SDLK_F12] == 1)
    {
      SDL_SaveBMP(screen,"screen0.bmp");
      NewMessage("Screen Shot Saved",IndexColor(LightBlue));
    }
    NextFrame();
  }while(!done);
  if(soundtest != NULL)Mix_FreeChunk(soundtest);
  exit(0);
  return 0;
}

void GoQuit()
{
  fprintf(stdout,"saving!\n");
  SaveGame(Player);
  fprintf(stdout,"saved!\n");
  exit(1);
}

/*
  Camera Section
*/
void SetCameraTarget(int tx,int ty, int framestotarget,int framestohold)
{
  CamInfo.Cameratarget.x = tx;
  CamInfo.Cameratarget.y = ty;
  CamInfo.Cameratargeted = 1;
  CamInfo.Cameradelay = framestotarget;
  CamInfo.Camerahold = framestohold;
}


void UpdateCamera()
{
  int range;
  int step;
  if(CamInfo.Cameratargeted)
  {
    if(CamInfo.Cameradelay > 0)
    {
      range = CamInfo.Cameratarget.x - Camera.x;
      step = range / CamInfo.Cameradelay--;
      Camera.x += step;
    }
    else if(CamInfo.Camerahold > 0)
    {
      CamInfo.Camerahold--;
    }
    else CamInfo.Cameratargeted = 0;
    return;
  }
  Camera.x += camdir;
  if(Camera.x > 239)
  {
    Camera.x = 239;
    camdir = -1;
  }
  if(Camera.x <= 0)
  {
    Camera.x = 0;
    camdir = 1;
  }
}

void CleanUpAll()
{ 
  KillMouse();
  KillHUD();
  FreeShadows();
  CloseSprites();
  ClearEntities();
  ClearSoundList();
  if(SDL_JoystickOpened(0))
    SDL_JoystickClose(joy);
  /*any other cleanup functions can be added here*/ 
}

void Init_All()
{
  Init_Graphics(windowed);
  InitSpriteList();
  icon = LoadSprite("images/icon_small.png",32,32);
  SDL_WM_SetCaption("Nanotech Evolution", NULL);
  SDL_WM_SetIcon(icon->image, NULL);
  LoadFonts();
  DrawSplashScreen();
  Init_Audio();
  InitSoundList();
  if(SDLNet_Init() == -1)CanNetwork = 0;
  else
  {
    CanNetwork = 1;
    atexit(SDLNet_Quit);
  }
  SDL_InitSubSystem(SDL_INIT_JOYSTICK);
  joy=SDL_JoystickOpen(0);
  InitEntityList();
  ResetAllParticles();
  InitMessages();
  InitMouse();
  LoadShadows();
  LoadHUD();
  atexit(CleanUpAll);
}

void GiveInfo()
{
  char message[10];
  sprintf(message,"v%i.%ia",MajorVersion,MinorVersion);
  NewMessage("Press Esc to Quit",IndexColor(White));
  NewMessage(message,IndexColor(White));
  NewMessage(" ",IndexColor(White));
  NewMessage(" ",IndexColor(White));
  NewMessage(" ",IndexColor(White));
  NewMessage(" ",IndexColor(White));
  NewMessage(" ",IndexColor(White));
  NewMessage(" ",IndexColor(White));
  NewMessage(" ",IndexColor(White));
}

void DrawSplashScreen()
{
  SDL_Surface *splash;
  Uint8 *keys;
  char message[10];
  sprintf(message,"v%i.%ia",MajorVersion,MinorVersion);
  splash = IMG_Load("images/splashscreen.png");
  if(splash != NULL)
  {
    SDL_BlitSurface(splash,NULL,background,NULL);
    SDL_BlitSurface(splash,NULL,screen,NULL);
    DrawText(message,screen,5,300,IndexColor(White),F_Medium);
    NextFrame();
  }
  SDL_FreeSurface(splash);
  do
  {
    SDL_PumpEvents();
    keys = SDL_GetKeyState(NULL);
  }
  while((SDL_GetTicks() < 2000)&&(keys[SDLK_SPACE] != 1));
}

/*calls all of the update functions for everything*/
void Update_ALL()
{
  if(CombatMode)
  {
    UpdateCombatMode();
    UpdateCamera();
  }
  if(EventMode)
  {
    UpdateEvent();
  }
  UpdateEntities();
  UpdateHUD();
}

/*calls all of the think function for everything*/
int Think_ALL()
{
  ThinkEntities();
  return 0;
}

/*calls all of the draw functions for everything*/
void Draw_ALL()
{
  DrawEntities();
  DrawAllParticles();
  DrawMessages();
  DrawHUD();
  DrawMouse();
}

/*the necessary blank line at the end of the file:*/


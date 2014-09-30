#include <stdlib.h>
#include <string.h>
#include "SDL_ttf.h"
#include "graphics.h"

extern SDL_Surface *screen;
TTF_Font *Font = NULL;
TTF_Font *SFont = NULL;
TTF_Font *LFont = NULL;

#define MAXMES    13

typedef struct MESSAGE_T
{
  char message[MAXMES][80];
  Uint32 fadeout[MAXMES];     /*this is when the test will fade out.*/
  Uint32 color[MAXMES]; 
  int last;
}Message;

Message messages;

void LoadFonts()
{
  LoadFont("fonts/font2.ttf",14,F_Small);
  LoadFont("fonts/font3.ttf",16,F_Medium);
  LoadFont("fonts/font3.ttf",18,F_Large);
}

void LoadFont(char filename[40],int ptsize,int type)
{
  if(TTF_Init()==0)
  {
    atexit(TTF_Quit);
  }
  else
  {
    fprintf(stderr,"Couldn't initialize Font System: %s\n", SDL_GetError());
    exit(1);
  }
  switch(type)
  {
    case F_Small:
      SFont = TTF_OpenFont(filename,ptsize);
      if(SFont == NULL)
      {
        fprintf(stderr,"Couldn't initialize Font: %s\n", SDL_GetError());
        exit(1);
      }
    break;
    case F_Medium:
      Font = TTF_OpenFont(filename,ptsize);
      if(Font == NULL)
      {
        fprintf(stderr,"Couldn't initialize Font: %s\n", SDL_GetError());
        exit(1);
      }
    case F_Large:
      LFont = TTF_OpenFont(filename,ptsize);
      if(LFont == NULL)
      {
        fprintf(stderr,"Couldn't initialize Font: %s\n", SDL_GetError());
        exit(1);
      }
    break;
  }
}

void InitMessages()
{
  int i;
  for(i = 0; i < MAXMES; i++)
  {
    strncpy(messages.message[i]," \0",80);
    messages.fadeout[i] = 0;
  }
}

void NewMessage(char *text,Uint32 color)
{
  int i,x = -1;
  Uint32 Now = SDL_GetTicks();
  Uint32 best = Now * 2;
  for(i = 0; i < MAXMES;i ++)
  {
    if(messages.fadeout[i] < Now)
    {
      x = i;
      break;
    }
  }
  if(x == -1) /*everything is in use, so lets hurry along the oldest one, shall we.*/
  {
    for(i = 0; i < MAXMES;i ++)
    {
      if(messages.fadeout[i] < best)
      {
        best = messages.fadeout[i];
        x = i;
      }
    }
  }
  if(x < 0)x = 0;
  strncpy(messages.message[x],text,80);
  messages.fadeout[x] = Now + 3500;
  messages.color[x] = color;
  messages.last = x;
}

void DrawMessages()
{
  int i,j;
  int t; /*target message*/
  Uint32  Now = SDL_GetTicks();
  j = MAXMES;
  for(i = MAXMES;i > 0;--i)
  {
    t = (messages.last + i)%MAXMES;
    if(messages.fadeout[t] > Now)
    {
      DrawText(messages.message[t],screen,11,11 + (16 * j),IndexColor(DarkGrey),F_Large);
      DrawText(messages.message[t],screen,10,10 + (16 * j--),messages.color[t],F_Large);
    }
  }
}

void CropStr(char *text,int length,int strl)
{
  int i;
  for(i = 0;i < strl - length;i++)
  {
    text[i] = text[i + length];
  }
  text[i] = '\0';/*null terminate in case its overwritten*/
}

void DrawTextBlock(char *thetext,SDL_Surface *surface,int sx, int sy,Uint32 color,int size,int width)
{
  char textline[120];
  char temptextline[120];
  char text[120];
  char word[80];
  SDL_Surface *fontpic = NULL;
  SDL_Surface *tempS = NULL;
  int drawheight = sy;
  SDL_Rect dst;
  int done = 0;
  int w,h;
  SDL_Color colortype,bgcolor;
  int lindex = 0;
  TTF_Font *font = NULL;
  if((thetext == NULL)||(thetext[0] == '\0'))
  {
    fprintf(stderr,"attempting to draw a NULL string\n");
    return;
  }
  switch(size)
  {
    case F_Small:
      if(SFont == NULL)return;
      font = SFont;
      break;
    case F_Medium:
      if(Font == NULL)return;
      font = Font;
      break;
    case F_Large:
      if(LFont == NULL)return;
      font = LFont;
      break;
  }
  SDL_GetRGBA(color, screen->format, &colortype.r, &colortype.g, &colortype.b, &colortype.unused);
  bgcolor.r = 0;
  bgcolor.g = 0;
  bgcolor.b = 0;
  bgcolor.unused = SDL_ALPHA_TRANSPARENT;
  strncpy(text,thetext,120);
  temptextline[0] = '\0';
  do
  {
    if(sscanf(text,"%s",word) == EOF)
    { /*we're out of words to draw, so draw whatever is left and return.*/
      tempS = TTF_RenderText_Blended(font, temptextline,colortype);/*render it*/
      fontpic = SDL_DisplayFormatAlpha(tempS);
      SDL_FreeSurface(tempS);
      dst.x = sx;
      dst.y = drawheight;
      SDL_SetColorKey(fontpic, SDL_SRCCOLORKEY , SDL_MapRGBA(screen->format, bgcolor.r,bgcolor.g,bgcolor.b,bgcolor.unused));
      SDL_BlitSurface(fontpic,NULL,surface,&dst);
      SDL_FreeSurface(fontpic);/*draw it*/
      return;
    }
    fprintf(stdout,"got %s from text\n",word);
    CropStr(text,strlen(word) + 1,120);
    strncpy(textline,temptextline,120);/*keep the last line that worked*/
    sprintf(temptextline,"%s %s",temptextline,word); /*add a word*/
    TTF_SizeText(font, temptextline, &w, &h); /*see how big it is now*/
    lindex += strlen(word);
    if(w > width)         /*see if we have gone over*/
    {
      tempS = TTF_RenderText_Blended(font, textline,colortype);/*render it*/
      fontpic = SDL_DisplayFormatAlpha(tempS);
      SDL_FreeSurface(tempS);
      dst.x = sx;
      dst.y = drawheight;
      SDL_SetColorKey(fontpic, SDL_SRCCOLORKEY , SDL_MapRGBA(screen->format, bgcolor.r,bgcolor.g,bgcolor.b,bgcolor.unused));
      SDL_BlitSurface(fontpic,NULL,surface,&dst);
      SDL_FreeSurface(fontpic);/*draw it*/
      /*draw the line and get ready for the next line*/
      drawheight += h;
      sprintf(temptextline,"%s",word); /*add a word*/
    }
  }while(!done);
}

void DrawTextCentered(char *text,SDL_Surface *surface,int sx,int sy,Uint32 color,int size)
{
  SDL_Surface *temp1 = NULL;
  SDL_Surface *fontpic = NULL;
  SDL_Color colortype,bgcolor;
  SDL_Rect dst;
  if((text == NULL)||(surface == NULL))return;
  SDL_GetRGBA(color, screen->format, &colortype.r, &colortype.g, &colortype.b, &colortype.unused);
  bgcolor.r = 0;
  bgcolor.g = 0;
  bgcolor.b = 0;
  bgcolor.unused = SDL_ALPHA_TRANSPARENT;
  switch(size)
  {
    case F_Small:
      if(SFont == NULL)return;
      temp1 = TTF_RenderText_Blended(SFont, text,colortype);
      break;
    case F_Medium:
      if(Font == NULL)return;
      temp1 = TTF_RenderText_Blended(Font, text,colortype);
      break;
    case F_Large:
      if(LFont == NULL)return;
      temp1 = TTF_RenderText_Blended(LFont, text,colortype);
      break;
  }
  fontpic = SDL_DisplayFormatAlpha(temp1);
  SDL_FreeSurface(temp1);
  dst.x = sx - (fontpic->w>>1);
  dst.y = sy;
  SDL_SetColorKey(fontpic, SDL_SRCCOLORKEY , SDL_MapRGBA(screen->format, bgcolor.r,bgcolor.g,bgcolor.b,bgcolor.unused));
  SDL_BlitSurface(fontpic,NULL,surface,&dst);
  SDL_FreeSurface(fontpic);
}


void DrawText(char *text,SDL_Surface *surface,int sx,int sy,Uint32 color,int size)
{
  SDL_Surface *temp1 = NULL;
  SDL_Surface *fontpic = NULL;
  SDL_Color colortype,bgcolor;
  SDL_Rect dst;
  if((text == NULL)||(surface == NULL))return;
  SDL_GetRGBA(color, screen->format, &colortype.r, &colortype.g, &colortype.b, &colortype.unused);
  bgcolor.r = 0;
  bgcolor.g = 0;
  bgcolor.b = 0;
  bgcolor.unused = SDL_ALPHA_TRANSPARENT;
  switch(size)
  {
    case F_Small:
      if(SFont == NULL)return;
      temp1 = TTF_RenderText_Blended(SFont, text,colortype);
    break;
    case F_Medium:
      if(Font == NULL)return;
      temp1 = TTF_RenderText_Blended(Font, text,colortype);
    break;
    case F_Large:
      if(LFont == NULL)return;
      temp1 = TTF_RenderText_Blended(LFont, text,colortype);
    break;
  }
  fontpic = SDL_DisplayFormatAlpha(temp1);
  SDL_FreeSurface(temp1);
  dst.x = sx;
  dst.y = sy;
  SDL_SetColorKey(fontpic, SDL_SRCCOLORKEY , SDL_MapRGBA(screen->format, bgcolor.r,bgcolor.g,bgcolor.b,bgcolor.unused));
  SDL_BlitSurface(fontpic,NULL,surface,&dst);
  SDL_FreeSurface(fontpic);
}



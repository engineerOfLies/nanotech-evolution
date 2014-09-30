#ifndef __EVENTS__
#define __EVENTS__

/*

  This section will be for events: Scripted sequences in the game.

*/
#define MAXFRAMES 32
/*
  This dater structure is designed to describe an scriptable event for the game.
  Intended to be read from a config file or set through an editor.
*/
/*an animation list will be referenced by the EventInfo*/
typedef struct
{
  char uname[80];   /*unique name of animation*/
  char sprites[80]; /*the sprite to use*/
  float vx,vy;      /*the direction of motion*/
  int timeout;      /*-1 does not time out until killed*/
}Animsprite;

typedef struct
{
  int timeindex;    /*index from start of event for this frame to take action*/
  char command[80]; /*what to do this key frame*/
  char options[80]; /*text paresed for commands*/
}KeyFrame;

typedef struct
{
  char uname[80];         /*unique name to search by*/
  char inDialog[4][120];  /*dialog to be spoken at onset of event*/
  int  inColor;           /*color of text*/
  int  px, py;            /*the position of the player during the event*/
  KeyFrame frames[MAXFRAMES];    /*the maximum number of keyframes for a single event*/
}EventInfo;

void DoEvent(char name[80],void (*exitevent)());
void EndEvent();
void UpdateEvent();

#endif

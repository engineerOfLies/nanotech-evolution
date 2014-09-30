#ifndef __GAME__
#define __GAME__

/*
 *    This file contains the structures and prototypes for the GAME itself
 */

#include "entity.h"

void Draw_ALL();
void SetCameraTarget(int tx,int ty, int framestotarget,int framestohold);
void UpdateCamera();

typedef struct
{
  Coord Cameratarget;
  int   Cameratargeted;
  int   Cameradelay;
  int   Camerahold;
}CameraInfo;




#endif

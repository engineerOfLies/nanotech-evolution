#ifndef __NET__
#define __NET__
/*

  data definitions for networking

*/


#include "SDL_net.h"


typedef struct ClientData_T
{
	int connected;
	int clientNum;
	int frame;
	int health;
	int facingright;
	int status;	
	int colliding;
	int ammo;
	int animation;
	int accessory;
	int weapon;
	int kills;
	int deaths;
	int shotsfired;
	int shotshit;
	int level;
	int hitenemy;
	int fire;
	int takedamage;
	SDL_Rect bbox;
}ClientData;


typedef struct client_s
{
  IPaddress ip;
  TCPsocket csd; /* Client socket descriptor */
  TCPsocket csdSend; /* Client socket for Sending  */
  int used;
}ClientInfo;

void initClientList();
void ServerBegin(); /*sets game up to be a server*/
void SetupClient();/*opens dialogs for connecting to server*/
void ClientBegin(char serverIp[16]);  /*sets game up to be the client*/
int Send(void *data,int len);         /*sends data after net begin*/
int PassiveReceive(void *data,int len);      /*received data after net begin, if active*/
int HardReceive(void *data,int len);      /*received data after net begin, wait till its done.*/
#endif
/*newline*/

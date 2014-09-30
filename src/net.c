#include "net.h"
#include "hud.h"
#include "combat.h"

#define MAXCLIENTS 32

int IsServer;
int CanNetwork;

char ServerIP[16];
ClientInfo Client;
int NumClients;
IPaddress *remoteIP;
TCPsocket sd; /*TCP Socket descriptor */
IPaddress ip;
SDLNet_SocketSet set;/*the set of socket(s) to listen on*/

void ServerBegin()
{
  int timeout = 4000;
  if(!CanNetwork)
  {
    NewMessage("No Network",IndexColor(LightRed));
    return;
  }
  SDLNet_ResolveHost(&ip, NULL, 2000);
  sd = SDLNet_TCP_Open(&ip);
  do
  {
    Client.csd = SDLNet_TCP_Accept(sd);
    if(Client.csd != NULL)remoteIP = SDLNet_TCP_GetPeerAddress(Client.csd);
    timeout--;
    SDL_Delay(1);
  }while((remoteIP == NULL)&&(timeout > 0));
  if(timeout <= 0)
  {
    NewMessage("no one connected!",IndexColor(LightViolet));
    YesNo("Try Again?",ServerBegin,NULL);
    SDLNet_TCP_Close(sd);
    SDLNet_TCP_Close(Client.csd);
    return;
  }
  set = SDLNet_AllocSocketSet(1);
  if(!set)
  {
    NewMessage("MAJOR NET ERROR!",IndexColor(LightRed));
    return;
  }
  SDLNet_TCP_AddSocket(set, Client.csd);
  CombatBeginNet(1);
}

int Send(void *data,int len)
{
  int sent;
  if(IsServer)
  {
    do
    {
      sent = SDLNet_TCP_Send(Client.csd, data, len);
      if(sent < len)fprintf(stdout,"sent %i / %i dater\n",sent,len);
    }while(sent != len);
  }
  else
  {
    /*sent = SDLNet_TCP_Send(Client.csd, data, len);*/
    do
    {
      sent = SDLNet_TCP_Send(sd, data, len);
      if(sent < len)fprintf(stdout,"sent %i / %i dater\n",sent,len);
    }while(sent != len);
    fprintf(stdout,"sent\n");
  }
  if(sent <= 0)
  {
    /*disconnect*/
    NewMessage("Disconnected!",IndexColor(Red));
    return -1;
  }
  return 1;
}

int HardReceive(void *data,int len)
{
  int recv;
  int ready;
  int timeout = 0;
  while(timeout < 1000)
  {
    ready = SDLNet_CheckSockets(set, 5);
    if(ready == -1)
    {
      NewMessage("System Network Error",IndexColor(LightRed));
      return -1;
    }
    if(ready > 0)
    {
      if(IsServer)
      {
        recv = SDLNet_TCP_Recv(Client.csd, data, len);
      }
      else
      {
        recv = SDLNet_TCP_Recv(sd, data, len);
      }
      if(recv <= 0)
      {
        NewMessage("Network Error",IndexColor(LightRed));
        return -1;
      }
      if(recv == len)return 1;
      else if (recv)fprintf(stdout,"received %i / %i",recv,len);
    }
    timeout++;
  }
  return 0;
}

int PassiveReceive(void *data,int len)
{
  int recv;
  int ready = SDLNet_CheckSockets(set, 5);
  if(ready == -1)
  {
    NewMessage("System Network Error",IndexColor(LightRed));
    return -1;
  }
  if(ready > 0)
  {
    if(IsServer)
    {
      recv = SDLNet_TCP_Recv(Client.csd, data, len);
    }
    else
    {
      recv = SDLNet_TCP_Recv(sd, data, len);
    }
    if(recv <= 0)
    {
      NewMessage("Network Error",IndexColor(LightRed));
      return -1;
    }
    if(recv > 0)return 1;
  }
  return 0;
}

void GoClient()
{
  /*check IP for validity*/
  ClientBegin(ServerIP);
}

void SetupClient()
{
  memset(ServerIP,0,sizeof(ServerIP));
  IPDialog("Enter IP",ServerIP,GoClient);
}

void ClientBegin(char serverIp[16])
{
  if(!CanNetwork)
  {
    NewMessage("No Network",IndexColor(LightRed));
    return;
  }
  if ((SDLNet_ResolveHost(&ip, serverIp, 2000) < 0)||(ip.host == INADDR_NONE))
  {
    fprintf(stderr,"unable to open IP address: %s",SDL_GetError());
    NewMessage("Bad IP Address",IndexColor(LightRed));
    return;
  }
  fprintf(stdout,"net got here!\n");
  sd = SDLNet_TCP_Open(&ip);
  set = SDLNet_AllocSocketSet(1);
  if(!set)
  {
    NewMessage("MAJOR NET ERROR!",IndexColor(LightRed));
    return;
  }
  SDLNet_TCP_AddSocket(set, sd);
  CombatBeginNet(0);
}



/*eol@eof*/

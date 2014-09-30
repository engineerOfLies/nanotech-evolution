#ifndef __ITEMS__
#define __ITEMS__

#define MAXLISTITEMS 128

typedef struct
{
  char uname[80];     /*search by name*/
  char dname[80];     /*display name*/
  char sname[80];     /*path to sprite*/
  int  swidth;        /*sprite frame width*/
  int  sheight;       /*sprite frame height*/
  int  colorswaps[3];    /*for the sprite*/
  int  sframe;        /*the frame to use for the sprite*/
  int  eframe;        /*the end frame for animation*/
  int  rarity;        /*how often this item will be found*/
  int  attributes[6]; /*modification to attributes*/
  int  health;        /*health modification*/
  int  stamina;       /*stamina modification*/
  int  elements[5];   /*depending on function*/
  int  good;
  int  evil;
  char function[80];  /*what its for...*/
  char finfo[80];     /*additional info for what its for.*/
  char description[120];    /*about the item.*/
  int index;
}Item;

Item *GetItemByName(char uname[80]);
void ItemInfoWindow();

void EquipItem(Item *item, int playernumber,int slot);
void LoadItemListBinary();
void LoadItemListText();
void SaveItemListBinary();
void SaveItemListText();

#endif

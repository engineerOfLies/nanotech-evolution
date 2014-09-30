#include "items.h"
#include "monster.h"
#include "hud.h"

Item ItemList[MAXLISTITEMS];
int NumItems = 0;
Sprite *ItemSprites[MAXITEMS];

Item *GetItemByName(char uname[80])
{
  int i;
  for(i = 0;i < NumItems;i++)
  {
    if(strncmp(uname,ItemList[i].uname,80)==0)
    {
      return &ItemList[i];
    }
  }
  return NULL;
}
/*since there are a lot of items and not often will they all be loaded, I will only load them up as needed, but won't be deleting them until I am done.*/
Sprite *GetItemSprite(int itemindex)
{
  Item *item;
  if(ItemSprites[itemindex] == NULL)
  {
    item = &ItemList[itemindex];
    ItemSprites[itemindex] = LoadSwappedSprite(item->sname,item->swidth,item->sheight, item->colorswaps[0], item->colorswaps[1], item->colorswaps[2]);
    return ItemSprites[itemindex];
  }
  else return ItemSprites[itemindex];
  return NULL;
}

void ItemInfoWindow()
{
  Item *item;
  item = GetItemByName("firecrystal1");
  InfoWindow(item->dname,GetItemSprite(item->index),8,item->description);
}

void EquipItem(Item *item, int playernumber,int slot);
void ClearItemList();
void LoadItemListBinary();
void SaveItemListBinary();

void SaveItemListText();

void LoadItemListText()
{
  int i;
  int temp;
  char buf[128];
  char *c;
  FILE *file;
  file = fopen("system/itemlist.cfg","r");
  if(file == NULL)
  {
    fprintf(stderr,"unable to open item file!: %s\n",SDL_GetError());
    return;
  }
  memset(ItemList,0,sizeof(Item) * MAXLISTITEMS);
  i = 0;
  while((fscanf(file, "%s", buf) != EOF)&&(i < MAXLISTITEMS))
  {
    if(strcmp(buf,"#") ==0)
    {
      fgets(buf, sizeof(buf), file);
      continue;/*ignore the rest of the line.*/
    }
    if(strcmp(buf,"end") ==0)
    {
      ItemList[i].index = i;
      i++;
      continue;
    }
    if(strcmp(buf,"uname") ==0)
    {
      fgetc(file);  /*clear the space before the word*/
      fgets(ItemList[i].uname, 80, file);
      c = strchr(ItemList[i].uname, '\n');
      if(c != NULL) *c = '\0';    /*replace trailing return with terminating character*/
      continue;
    }
    if(strcmp(buf,"sname") ==0)
    {
      fgetc(file);  /*clear the space before the word*/
      fgets(ItemList[i].sname, 80, file);
      c = strchr(ItemList[i].sname, '\n');
      if(c != NULL) *c = '\0';    /*replace trailing return with terminating character*/
      continue;
    }
    if(strcmp(buf,"dname") ==0)
    {
      fgetc(file);  /*clear the space before the word*/
      fgets(ItemList[i].dname, 80, file);
      c = strchr(ItemList[i].dname, '\n');
      if(c != NULL) *c = '\0';    /*replace trailing return with terminating character*/
      continue;
    }
    if(strcmp(buf,"swidth") ==0)
    {
      fscanf(file, "%i", &ItemList[i].swidth);
      continue;
    }
    if(strcmp(buf,"sheight") ==0)
    {
      fscanf(file, "%i", &ItemList[i].sheight);
      continue;
    }
    if(strcmp(buf,"rarity") ==0)
    {
      fscanf(file, "%i", &ItemList[i].rarity);
      continue;
    }
    if(strcmp(buf,"sframe") ==0)
    {
      fscanf(file, "%i", &ItemList[i].sframe);
      continue;
    }
    if(strcmp(buf,"eframe") ==0)
    {
      fscanf(file, "%i", &ItemList[i].eframe);
      continue;
    }
    if(strcmp(buf,"stamina") ==0)
    {
      fscanf(file, "%i", &ItemList[i].stamina);
      continue;
    }
    if(strcmp(buf,"health") ==0)
    {
      fscanf(file, "%i", &ItemList[i].health);
      continue;
    }
    if(strcmp(buf,"attribute") ==0)
    {
      fscanf(file, "%i", &temp);
      fscanf(file, "%i", &ItemList[i].attributes[temp]);
      continue;
    }
    if(strcmp(buf,"element") ==0)
    {
      fscanf(file, "%i", &temp);
      fscanf(file, "%i", &ItemList[i].elements[temp]);
      continue;
    }
    if(strcmp(buf,"light") ==0)
    {
      fscanf(file, "%i", &ItemList[i].good);
      continue;
    }
    if(strcmp(buf,"dark") ==0)
    {
      fscanf(file, "%i", &ItemList[i].evil);
      continue;
    }
    if(strcmp(buf,"function") ==0)
    {
      fgetc(file);  /*clear the space before the word*/
      fscanf(file, "%s", ItemList[i].function);
      c = strchr(ItemList[i].function, '\n');
      if(c != NULL) *c = '\0';    /*replace trailing return with terminating character*/
      continue;
    }
    if(strcmp(buf,"colorswap") ==0)
    {
      fscanf(file, "%i %i %i", &ItemList[i].colorswaps[0], &ItemList[i].colorswaps[1], &ItemList[i].colorswaps[2]);
      continue;
    }
    if(strcmp(buf,"info") ==0)
    {
      fgetc(file);  /*clear the space before the word*/
      fgets(ItemList[i].finfo, 80, file);
      c = strchr(ItemList[i].finfo, '\n');
      if(c != NULL) *c = '\0';    /*replace trailing return with terminating character*/
      continue;
    }
    if(strcmp(buf,"description") ==0)
    {
      fgetc(file);  /*clear the space before the word*/
      fgets(ItemList[i].description, 120, file);
      c = strchr(ItemList[i].description, '\n');
      if(c != NULL) *c = '\0';    /*replace trailing return with terminating character*/
      continue;
    }
  }
  fclose(file);
  NumItems = i;
}


/*blahh*/

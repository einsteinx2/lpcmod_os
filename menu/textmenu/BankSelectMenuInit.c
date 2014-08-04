#include "lpcmod_v1.h"
#include "TextMenu.h"


TEXTMENU *BankSelectMenuInit(void) {
	
	TEXTMENUITEM *itemPtr;
	TEXTMENU *menuPtr;
	
	menuPtr = (TEXTMENU*)malloc(sizeof(TEXTMENU));
	memset(menuPtr,0x00,sizeof(TEXTMENU));
	strcpy(menuPtr->szCaption, "Select Flash bank to flash");
	

	//Bank0 (512KB)
	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	strcpy(itemPtr->szCaption, "Bank0 (512KB)");
	itemPtr->functionPtr=DrawChildTextMenu;
	itemPtr->functionDataPtr = (void *)FlashMenuInit(switchBank(BNK512));
	TextMenuAddItem(menuPtr, itemPtr);

	//Bank1 (256KB)
	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	strcpy(itemPtr->szCaption, "Bank1 (256KB)");
	itemPtr->functionPtr=DrawChildTextMenu;
	itemPtr->functionDataPtr = (void *)FlashMenuInit(switchBank(BNK256));
	TextMenuAddItem(menuPtr, itemPtr);

	//Bank2 (OS)
	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	strcpy(itemPtr->szCaption, "Bank2 (OS)");
	itemPtr->functionPtr=DrawChildTextMenu;
	itemPtr->functionDataPtr = (void *)FlashMenuInit(switchBank(BNKOS));
	TextMenuAddItem(menuPtr, itemPtr);
	
	return menuPtr;
}

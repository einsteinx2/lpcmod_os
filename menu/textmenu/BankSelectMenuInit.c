#include "lpcmod_v1.h"
#include "TextMenu.h"
#include "include/boot.h"

TEXTMENU* BankSelectInit(void *bank);


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
	itemPtr->functionPtr=(void *)BankSelectInit;
	itemPtr->functionDataPtr = malloc(sizeof(char));
				*(char*)itemPtr->functionDataPtr = BNK512;
	TextMenuAddItem(menuPtr, itemPtr);

	//Bank1 (256KB)
	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	strcpy(itemPtr->szCaption, "Bank1 (256KB)");
	itemPtr->functionPtr=(void *)BankSelectInit;
	itemPtr->functionDataPtr = malloc(sizeof(char));
				*(char*)itemPtr->functionDataPtr = BNK256;
	TextMenuAddItem(menuPtr, itemPtr);

	//Bank2 (OS)
	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	strcpy(itemPtr->szCaption, "Bank2 (OS)");
	itemPtr->functionPtr=(void *)BankSelectInit;
	itemPtr->functionDataPtr = malloc(sizeof(char));
				*(char*)itemPtr->functionDataPtr = BNKOS;
	TextMenuAddItem(menuPtr, itemPtr);
	
	return menuPtr;
}

TEXTMENU* BankSelectInit(void *bank) {
	TEXTMENUITEM *itemPtr;
	TEXTMENU *menuPtr;

	menuPtr = (TEXTMENU*)malloc(sizeof(TEXTMENU));
	memset(menuPtr,0x00,sizeof(TEXTMENU));
	strcpy(menuPtr->szCaption, "Select Flash bank");

	switchBank(*(char *)bank);

	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	strcpy(itemPtr->szCaption, "Selected bank");
	itemPtr->functionPtr=DrawChildTextMenu;
	itemPtr->functionDataPtr = (void *)FlashMenuInit();
	TextMenuAddItem(menuPtr, itemPtr);

	return menuPtr;
}


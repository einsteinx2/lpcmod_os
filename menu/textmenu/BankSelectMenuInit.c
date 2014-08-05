#include "lpcmod_v1.h"
#include "TextMenu.h"

TEXTMENU* FlashMenuInit512(void);
TEXTMENU* FlashMenuInit256(void);
TEXTMENU* FlashMenuInitOS(void);

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
	itemPtr->functionDataPtr = (void *)FlashMenuInit512;
	TextMenuAddItem(menuPtr, itemPtr);

	//Bank1 (256KB)
	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	strcpy(itemPtr->szCaption, "Bank1 (256KB)");
	itemPtr->functionPtr=DrawChildTextMenu;
	itemPtr->functionDataPtr = (void *)FlashMenuInit256;
	TextMenuAddItem(menuPtr, itemPtr);

	//Bank2 (OS)
	itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
	memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
	strcpy(itemPtr->szCaption, "Bank2 (OS)");
	itemPtr->functionPtr=DrawChildTextMenu;
	itemPtr->functionDataPtr = (void *)FlashMenuInitOS;
	TextMenuAddItem(menuPtr, itemPtr);
	
	return menuPtr;
}

TEXTMENU* FlashMenuInit512() {
	switchBank(BNK512);
	return ((TEXTMENU*)FlashMenuInit());
}

TEXTMENU* FlashMenuInit256() {
	switchBank(BNK256);
	return ((TEXTMENU*)FlashMenuInit());
}

TEXTMENU* FlashMenuInitOS() {
	switchBank(BNKOS);
	return ((TEXTMENU*)FlashMenuInit());
}

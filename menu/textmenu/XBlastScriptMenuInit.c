#include "XBlastScriptMenuActions.h"
#include "boot.h"
#include "BootIde.h"
#include "TextMenu.h"
#include "memory_layout.h"
#include "BootFATX.h"
#include "lpcmod_v1.h"

TEXTMENU* RunScriptMenuInit(void);
TEXTMENU* SaveScriptMenuInit(void);


TEXTMENU* XBlastScriptMenuInit(void) {
    TEXTMENUITEM *itemPtr;
    TEXTMENU *menuPtr;

    menuPtr = (TEXTMENU*)malloc(sizeof(TEXTMENU));
    memset(menuPtr,0x00,sizeof(TEXTMENU));
    strcpy(menuPtr->szCaption, "XBlast scripts");

    //Run Script.
    itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption, "Run script from HDD");
    itemPtr->functionPtr= DrawChildTextMenu;
    itemPtr->functionDataPtr = (void *)RunScriptMenuInit();
    TextMenuAddItem(menuPtr, itemPtr);

    //Save script to flash.
    if(cromwell_config==CROMWELL || (fHasHardware == SYSCON_ID_V1 || fHasHardware == SYSCON_ID_XT)){
        itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
        memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
        strcpy(itemPtr->szCaption, "Save script to flash");
        itemPtr->functionPtr= DrawChildTextMenu;
        itemPtr->functionDataPtr = (void *)SaveScriptMenuInit();
        TextMenuAddItem(menuPtr, itemPtr);
    }

    //Load script from flash.
    if(LPCmodSettings.firstScript.ScripMagicNumber == 0xFAF1){
        itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
        memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
        strcpy(itemPtr->szCaption, "Run script from flash");
        itemPtr->functionPtr= loadScriptFromFlash;
        TextMenuAddItem(menuPtr, itemPtr);
    }

    //No need to show this item if there's no way to persist setting.
    if(fHasHardware == SYSCON_ID_V1 || fHasHardware == SYSCON_ID_XT || cromwell_config==CROMWELL){
        itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
        memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
        strcpy(itemPtr->szCaption,"Enable Boot script : ");
        sprintf(itemPtr->szParameter, "%s", LPCmodSettings.OSsettings.runBootScript? "Yes" : "No");
        itemPtr->functionPtr= toggleRunBootScript;
        itemPtr->functionDataPtr= itemPtr->szParameter;
        itemPtr->functionLeftPtr=toggleRunBootScript;
        itemPtr->functionLeftDataPtr = itemPtr->szParameter;
        itemPtr->functionRightPtr=toggleRunBootScript;
        itemPtr->functionRightDataPtr = itemPtr->szParameter;
        TextMenuAddItem(menuPtr, itemPtr);
    }

    itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
    strcpy(itemPtr->szCaption,"Enable Bank script : ");
    sprintf(itemPtr->szParameter, "%s", LPCmodSettings.OSsettings.runBankScript? "Yes" : "No");
    itemPtr->functionPtr= toggleRunBankScript;
    itemPtr->functionDataPtr= itemPtr->szParameter;
    itemPtr->functionLeftPtr=toggleRunBankScript;
    itemPtr->functionLeftDataPtr = itemPtr->szParameter;
    itemPtr->functionRightPtr=toggleRunBankScript;
    itemPtr->functionRightDataPtr = itemPtr->szParameter;
    TextMenuAddItem(menuPtr, itemPtr);

    return menuPtr;

}

TEXTMENU* RunScriptMenuInit(void) {
    TEXTMENUITEM *itemPtr;
    TEXTMENU *menuPtr;
    FATXFILEINFO fileinfo;
    FATXPartition *partition;

    char *fnames[4096]; //Because Each dir can have up to 4096 files when not in root of partition.
    short n=0, i=0;
    int bioses=0;
    int res;
    int dcluster;
    char *path="\\XBlast\\scripts\\";      //And we're not in root.
    char fullPath[25];
    char *fullPathptr = fullPath;
    for(i = 0; i < 4096; i++)   //Not really useful but good practice.
        fnames[i] = NULL;
    memset(fullPath, 0, 20);

    // Generate the menu title.
    strcpy(fullPath, "'C:");
    fullPathptr += 3;
    strcpy(fullPathptr, path);
    fullPathptr += strlen(path);
    strcpy(fullPathptr, "'");
    fullPathptr = NULL;

    //Only supports BIOS file fetch from Master HDD.
    partition = OpenFATXPartition(0, SECTOR_SYSTEM, SYSTEM_SIZE);

    menuPtr = (TEXTMENU*)malloc(sizeof(TEXTMENU));
    memset(menuPtr,0,sizeof(TEXTMENU));

    strcpy(menuPtr->szCaption, fullPath);

    if(partition != NULL) {
        dcluster = FATXFindDir(partition, FATX_ROOT_FAT_CLUSTER, "XBlast");
        if((dcluster != -1) && (dcluster != 1)) {
            dcluster = FATXFindDir(partition, dcluster, "scripts");
        }
        if((dcluster != -1) && (dcluster != 1)) {
            n = FATXListDir(partition, dcluster, &fnames[0], 4096, path);
            for (i=0; i<n; i++) {
                // Check the file.
                res = FATXFindFile(partition, fnames[i], FATX_ROOT_FAT_CLUSTER, &fileinfo);

                if((res) && (fileinfo.fileSize)) {
                    // If it's a (readable) file - i.e. not a directory.
                    // AND it's filesize is at least 1 byte.
                    itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
                    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
                    sprintf(itemPtr->szCaption,fnames[i]+strlen(path));
                    itemPtr->functionPtr = loadRunScript;
                    itemPtr->functionDataPtr = fnames[i];       //allocating char* pointer contained in char **fnames so char **fnames can be destroyed
                    itemPtr->functionDataPtrMemAlloc = true;    //at function return but we'll still get the pointer to the allocated memory location
                    TextMenuAddItem(menuPtr, itemPtr);
                    bioses++;
                }
            }
            if(n < 1) {
                // If there were no directories and no files.
                itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
                memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
                sprintf(itemPtr->szCaption,"No files in %s.", fullPath);
                itemPtr->functionPtr = NULL;
                TextMenuAddItem(menuPtr, itemPtr);
            } else if(bioses==0) {
                // If there were directories, but no files.
                itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
                memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
                sprintf(itemPtr->szCaption,"No script in %s.", fullPath);
                itemPtr->functionPtr = NULL;
                TextMenuAddItem(menuPtr, itemPtr);
            }
        } else {
            // If C:\BIOS doesnt exist.
            itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
            memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
            sprintf(itemPtr->szCaption,"%s does not exist.", fullPath);
            itemPtr->functionPtr = NULL;
            TextMenuAddItem(menuPtr, itemPtr);
        }
        CloseFATXPartition(partition);
    } else {
        // If the partition couldn't be opened at all.
        itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
        memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
        sprintf(itemPtr->szCaption,"Error reading C:\\ partition.");
        itemPtr->functionPtr = NULL;
        TextMenuAddItem(menuPtr, itemPtr);
    }
    return menuPtr;
}

TEXTMENU* SaveScriptMenuInit(void) {
    TEXTMENUITEM *itemPtr;
    TEXTMENU *menuPtr;
    FATXFILEINFO fileinfo;
    FATXPartition *partition;

    char *fnames[4096]; //Because Each dir can have up to 4096 files when not in root of partition.
    short n=0, i=0;
    int bioses=0;
    int res;
    int dcluster;
    char *path="\\XBlast\\scripts\\";      //And we're not in root.
    char fullPath[25];
    char *fullPathptr = fullPath;
    for(i = 0; i < 4096; i++)   //Not really useful but good practice.
        fnames[i] = NULL;
    memset(fullPath, 0, 20);

    // Generate the menu title.
    strcpy(fullPath, "'C:");
    fullPathptr += 3;
    strcpy(fullPathptr, path);
    fullPathptr += strlen(path);
    strcpy(fullPathptr, "'");
    fullPathptr = NULL;

    //Only supports BIOS file fetch from Master HDD.
    partition = OpenFATXPartition(0, SECTOR_SYSTEM, SYSTEM_SIZE);

    menuPtr = (TEXTMENU*)malloc(sizeof(TEXTMENU));
    memset(menuPtr,0,sizeof(TEXTMENU));

    strcpy(menuPtr->szCaption, fullPath);

    if(partition != NULL) {
        dcluster = FATXFindDir(partition, FATX_ROOT_FAT_CLUSTER, "XBlast");
        if((dcluster != -1) && (dcluster != 1)) {
            dcluster = FATXFindDir(partition, dcluster, "scripts");
        }
        if((dcluster != -1) && (dcluster != 1)) {
            n = FATXListDir(partition, dcluster, &fnames[0], 4096, path);
            for (i=0; i<n; i++) {
                // Check the file.
                res = FATXFindFile(partition, fnames[i], FATX_ROOT_FAT_CLUSTER, &fileinfo);

                if((res) && (fileinfo.fileSize)) {
                    // If it's a (readable) file - i.e. not a directory.
                    // AND it's filesize is at least 1 byte.
                    itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
                    memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
                    sprintf(itemPtr->szCaption,fnames[i]+strlen(path));
                    itemPtr->functionPtr = saveScriptToFlash;
                    itemPtr->functionDataPtr = fnames[i];       //allocating char* pointer contained in char **fnames so char **fnames can be destroyed
                    itemPtr->functionDataPtrMemAlloc = true;    //at function return but we'll still get the pointer to the allocated memory location
                    TextMenuAddItem(menuPtr, itemPtr);
                    bioses++;
                }
            }
            if(n < 1) {
                // If there were no directories and no files.
                itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
                memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
                sprintf(itemPtr->szCaption,"No files in %s.", fullPath);
                itemPtr->functionPtr = NULL;
                TextMenuAddItem(menuPtr, itemPtr);
            } else if(bioses==0) {
                // If there were directories, but no files.
                itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
                memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
                sprintf(itemPtr->szCaption,"No script in %s.", fullPath);
                itemPtr->functionPtr = NULL;
                TextMenuAddItem(menuPtr, itemPtr);
            }
        } else {
            // If C:\BIOS doesnt exist.
            itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
            memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
            sprintf(itemPtr->szCaption,"%s does not exist.", fullPath);
            itemPtr->functionPtr = NULL;
            TextMenuAddItem(menuPtr, itemPtr);
        }
        CloseFATXPartition(partition);
    } else {
        // If the partition couldn't be opened at all.
        itemPtr = (TEXTMENUITEM*)malloc(sizeof(TEXTMENUITEM));
        memset(itemPtr,0x00,sizeof(TEXTMENUITEM));
        sprintf(itemPtr->szCaption,"Error reading C:\\ partition.");
        itemPtr->functionPtr = NULL;
        TextMenuAddItem(menuPtr, itemPtr);
    }
    return menuPtr;
}

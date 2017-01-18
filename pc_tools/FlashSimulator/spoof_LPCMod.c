/*
 * spoof_LPCMod.c
 *
 *  Created on: Dec 7, 2016
 *      Author: cromwelldev
 */

#include "lpcmod_v1.h"
#include <stdio.h>

static unsigned char currentBank = BNKOS;

const char* getBankName(unsigned char bank)
{
    switch(bank)
    {
    case BNKOS:
        return "OS Bank";
    case BNK256:
        return "256KB Bank";
    case BNK512:
        return "512KB Bank";
    case BNKFULLTSOP:
        return "Full TSOP Bank";
    case BNKTSOPSPLIT0:
        return "TSOP Split Bank0";
    case BNKTSOPSPLIT1:
        return "TSOP Split Bank1";
    default:
    case NOBNKID:

        break;
    }
    return "No Bank!";
}

void switchOSBank(unsigned char bank)
{
    printf("Switching from \"%s\" to \"%s\"\n", getBankName(currentBank), getBankName(bank));
    currentBank = bank;
}

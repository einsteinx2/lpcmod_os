/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "i2c.h"
#include "CDMenuActions.h"

void CDEject(void *whatever) {
    I2CTransmitWord(0x10, 0x0c00);
}

void CDInject(void *whatever) {
    I2CTransmitWord(0x10, 0x0c01);
}

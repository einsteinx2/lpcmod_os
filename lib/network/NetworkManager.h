/*
 * NetworkManager.h
 *
 *  Created on: May 18, 2018
 *      Author: cromwelldev
 */

#ifndef LIB_NETWORK_NETWORKMANAGER_H_
#define LIB_NETWORK_NETWORKMANAGER_H_

typedef enum
{
    NetworkState_Idle,
    NetworkState_Init,
    NetworkState_DHCPStart,
    NetworkState_Running,
    NetworkState_ShuttingDown,
    NetworkState_Cleanup
}NetworkState;

void NetworkManager_init(void);
void NetworkManager_update(void);

void NetworkManager_start(void);
void NetworkManager_stop(void);

NetworkState NetworkManager_getState(void);
unsigned char NetworkManager_getIP(char * sz_out);

#endif /* LIB_NETWORK_NETWORKMANAGER_H_ */

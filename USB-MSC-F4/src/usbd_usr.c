/**
 * @file    usbd_usr.c
 * @brief   User callback for USB library
 * @date    6 sty 2015
 * @author  Michal Ksiezopolski
 *
 *
 * @verbatim
 * Copyright (c) 2015 Michal Ksiezopolski.
 * All rights reserved. This program and the
 * accompanying materials are made available
 * under the terms of the GNU Public License
 * v3.0 which accompanies this distribution,
 * and is available at
 * http://www.gnu.org/licenses/gpl.html
 * @endverbatim
 */

#include <usbd_usr.h>
#include <stdio.h>


/**
 * @brief User callbacks for USB events
 */
USBD_Usr_cb_TypeDef USR_cb = {
  USBD_USR_Init,
  USBD_USR_DeviceReset,
  USBD_USR_DeviceConfigured,
  USBD_USR_DeviceSuspended,
  USBD_USR_DeviceResumed,
  USBD_USR_DeviceConnected,
  USBD_USR_DeviceDisconnected,    
};
/**
 *
 */
void USBD_USR_Init(void)
{

}

void USBD_USR_DeviceReset(uint8_t speed)
{

}


void USBD_USR_DeviceConfigured (void)
{

}

void USBD_USR_DeviceSuspended(void)
{

}

void USBD_USR_DeviceResumed(void)
{

}

void USBD_USR_DeviceConnected (void)
{

}

void USBD_USR_DeviceDisconnected (void)
{

}

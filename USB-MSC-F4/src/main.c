/**
 * @file    main.c
 * @brief   STM32F4 USB CDC test
 * @date    9 kwi 2014
 * @author  Michal Ksiezopolski
 *
 *
 * @verbatim
 * Copyright (c) 2014 Michal Ksiezopolski.
 * All rights reserved. This program and the
 * accompanying materials are made available
 * under the terms of the GNU Public License
 * v3.0 which accompanies this distribution,
 * and is available at
 * http://www.gnu.org/licenses/gpl.html
 * @endverbatim
 */


// USB includes
#include <usbd_usr.h>
#include <usbd_desc.h>
#include <usbd_msc_core.h>
#include "fat16.h"

__ALIGN_BEGIN USB_OTG_CORE_HANDLE USB_OTG_dev __ALIGN_END; ///< USB device handle


/**
 * @brief Main function
 * @return None
 */
int main(void)
{

  FAT16_Init();

  // Initialize USB device stack
  USBD_Init(&USB_OTG_dev,
            USB_OTG_FS_CORE_ID,
            &USR_desc,
            &USBD_MSC_cb,
            &USR_cb);

  while (1)
  {

  }

}

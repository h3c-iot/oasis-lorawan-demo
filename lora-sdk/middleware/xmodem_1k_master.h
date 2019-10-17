/*******************************************************************************
*                                                                              *
*                Copyright (C) 2019, New H3C Technologies Co., Ltd.            *
*  License: Revised BSD License, see LICENSE.TXT file included in the project  *
*                                                                              *
*------------------------------------------------------------------------------*
*                                                                              *
*                              xmodem_1k_master.h                              *
*                                                                              *
*  Project Code: oasis-sdk                                                     *
*   Module Name: Lora Module WSL305S                                           *
*  Date Created: 2019-10-28                                                    *
*        Author: hxb18134                                                      *
*   Description: adapt to wsl305s module firmeware update                      *
*                                                                              *
*------------------------------------------------------------------------------*
*  Modification History                                                        *
*  DATE        NAME             DESCRIPTION                                    *
*  ----------------------------------------------------------------------------*
*  YYYY-MM-DD                                                                  *
*                                                                              *
*******************************************************************************/


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef XMODEM_1K_MASTER_H
#define XMODEM_1K_MASTER_H

#define BOOT_ENTER          'U'
#define UPGRADE_START       '1'

#define SOH                 0x01
#define STX                 0x02
#define EOT                 0x04
#define ACK                 0x06
#define NAK                 0x15
#define CAN                 0x18
#define CTRLZ               0x1A

#define DLY_1S              1000
#define MAXRETRANS          25

#define TRANSMIT_XMODEM_1K

/* Exported functions ------------------------------------------------------- */
int xmodemReceive(unsigned char *dest, int destsz);
int xmodemTransmit(unsigned char *src, int srcsz);

#endif /* XMODEM_1K_MASTER_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

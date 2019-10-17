/*******************************************************************************
*                                                                              *
*                Copyright (C) 2019, New H3C Technologies Co., Ltd.            *
*  License: Revised BSD License, see LICENSE.TXT file included in the project  *
*                                                                              *
*------------------------------------------------------------------------------*
*                                                                              *
*                               crc16.h                                        *
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


#ifndef _CRC16_H_
#define _CRC16_H_

unsigned short crc16_ccitt(const void *buf, int len);

#endif /* _CRC16_H_ */

/*******************************************************************************
*                                                                              *
*                Copyright (C) 2019, New H3C Technologies Co., Ltd.            *
*  License: Revised BSD License, see LICENSE.TXT file included in the project  *
*                                                                              *
*------------------------------------------------------------------------------*
*                                                                              *
*                                  debug.h                                     *
*                                                                              *
*  Date Created: 2019-10-24                                                    *
*  Author: hxb18134                                                            *
*  Description:  uart debug                                                    *
*                                                                              *
*------------------------------------------------------------------------------*
*  Modification History                                                        *
*  DATE        NAME             DESCRIPTION                                    *
*  ----------------------------------------------------------------------------*
*  YYYY-MM-DD                                                                  *
*                                                                              *
*******************************************************************************/


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <string.h>
#include <stdarg.h>
#include <stdio.h>

//#define UART_DEBUG_LOG   

#ifdef   UART_DEBUG_LOG
#define  DEBUG_PRINT(fmt, ...)   printf(fmt, ##__VA_ARGS__)
#else
#define  DEBUG_PRINT(fmt, ...)    
#endif

#endif // __DEBUG_H__

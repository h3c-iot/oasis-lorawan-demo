/**
* @file        sdk-config.h
*
* @copyright   Copyright (C) 2019, New H3C Technologies Co., Ltd.            
*              License: Revised BSD License 
*              ,see LICENSE.TXT file included in the project.
*
* @brief       oasis type process
*                                                                                   
* @code        
*                          Author : yuxin
*
*                          Project Code: oasis-sdk
*
*                          Module Name: Oasis Protocol Extension                                                 
*                                                     
*                          Date Created: 2019-10-26  
*
*------------------------------------------------------------------------------*
*  Modification History                                                        *
*  DATE              NAME                  DESCRIPTION                         *
*------------------------------------------------------------------------------*
*  YYYY-MM-DD                                                                  *
*------------------------------------------------------------------------------*                                                                             
* @endcode   
*
* @defgroup  OASIS-SDK oasis SDK
*            this module define oasis extend protocol control
* @{
*/

#ifndef __SDK_CONFIG_H__
#define __SDK_CONFIG_H__

/** �˱ܼ�� */
#define OASIS_SDK_CSMACA                 0    /* 0:close, 1:open */

/** ����� */
#define OASIS_SDK_KEEPALIVE              1    /* 0:close, 1:open */

/** ���������� */
#define OASIS_SDK_KEEPALIVE_INTERVAL     (OASIS_SDK_KEEPALIVE & 1)    /* 0:close, 1:open */

/** ���ܵ��� */
#define OASIS_SDK_INTELIGENCE_DATARATE   1    /* 0:close, 1:open */

/** ����Ӧ���ʿ��� */
#define OASIS_SDK_ADR                    0    /* 0:close, 1:open */

/** ��·��� */
#define OASIS_SDK_LINK_CHECK             0    /* 0:close, 1:open */

/** ͬ������*/
#define OASIS_SDK_SYNC_CONFIG            1    /* 0:close, 1:open */

#endif

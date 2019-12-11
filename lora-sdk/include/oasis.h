/**
* @file        oasis.h
*
* @copyright   Copyright (C) 2019, New H3C Technologies Co., Ltd.            
*              License: Revised BSD License 
*              ,see LICENSE.TXT file included in the project.
*
* @brief       oasis protocol extension
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
* @defgroup  OASIS oasis protocol extend layer
*            this module is oasis protocol
* @{
*/

#ifndef __OASIS_H__
#define __OASIS_H__


/**
 * 用于保存处理绿洲扩展协议上层函数指针结构体
 */
typedef struct tagOasisHandler
{
    /**
     * 处理下行数据回调函数
     */    
    void (*pfOasis_ResetModule)( void );
}OASIS_HANDLER_S; 


void OASIS_SendConfigSYNCReq(void);
void OASIS_StopAliveTimer(void);
void OASIS_StartAliveTimer(void);
void OASIS_SendPullDataReq(void);
void OASIS_Init(OASIS_HANDLER_S *pstTypeHandler);

#endif /*__OASIS_H__*/


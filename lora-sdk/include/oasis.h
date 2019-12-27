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

/**
 * @brief 发送配置同步请求 
 */
void OASIS_SendConfigSYNCReq(void);

/**
 * @brief 停止保活报文定时器  
 */
void OASIS_StopAliveTimer(void);

/**
 * @brief 启动保活报文定时器   
 */
void OASIS_StartAliveTimer(void);

/**
 * @brief   用于数据同步的报文
 *          ,开放给外部使用的
 *          ,使用confirm的方式
 */
void OASIS_SendPullDataReq(void);

/**
 * @brief   绿洲扩展协议初始化
 *
 */
void OASIS_Init(OASIS_HANDLER_S *pstTypeHandler);

#endif /*__OASIS_H__*/


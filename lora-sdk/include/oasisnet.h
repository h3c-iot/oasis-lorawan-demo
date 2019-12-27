/**
* @file        oasisnet.h
*
* @copyright   Copyright (C) 2019, New H3C Technologies Co., Ltd.            
*              License: Revised BSD License 
*              ,see LICENSE.TXT file included in the project.
*
* @brief       oasis protocol extension
*                                                                                   
* @code        
*                          Author : wangzhen
*
*                          Project Code: oasis-sdk
*
*                          Module Name: Oasis network                                                 
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
* @defgroup  OASISNET oasis network  layer
*            this module is oasis network 
* @{
*/

#ifndef __OASISNET_H__
#define __OASISNET_H__

/**
 * 网络层队列地址数据类型
 */
#define OASIS_NETPKT_HANDLER          uint32_t

/**
 * 网络层队列无效地址
 */
#define OASIS_NETPKT_HANDLER_INVALID  (OASIS_NETPKT_HANDLER)0


/**
 * 报文类型
 */
typedef enum tagOASISPktsType
{
    /** 
      * 不需要确认类型 
      */
    OASIS_NETPKT_UNCONFIRMED,
    /** 
      * 需要确认类型 
      */
    OASIS_NETPKT_CONFIRMED,
}OASIS_NETPKT_TYPE_E;

/**
 * 用于保存处理mlme状态和事件的上层函数指针结构体
 */
typedef struct tagOASISNETHandler
{
    /**
     * 获取温度回调函数
     */    
    float ( *pfOasisNet_GetTemperatureLevel)(void);
    /**
     * 获取电量回调函数
     */
    uint8_t ( *pfOasisNet_BoardGetBatteryLevel)( void );
    
    void ( *pfOasisNet_JOINProcess )( bool bJoined);
        
}OASISNET_HANDLER_S;


/**
 * 用于保存处理接收下行数据和确认的上层函数指针结构体
 */
typedef struct tagOasisNetPktHandler
{
    /**
     * 处理下行数据回调函数
     */    
    void ( *pfOasis_ProcFRMPktReceive )( char *pcData, uint8_t ucDataLen);
    /**
     * 下行确认处理结果回调函数
     */
    void ( *pfOasis_ProcMcpsConfirm )( OASIS_NETPKT_HANDLER hPktHandler, uint8_t ucResult);
}OASISNET_PKT_HANDLER_S;


/**
 * @brief  发送接口
 *
 * @param  [输入] enConfirmType 发送数据的类型
 *         ,enConfirmType = OASIS_NETPKT_CONFIRMED 表示发送需要确认的报文
 *         ,enConfirmType = OASIS_NETPKT_UNCONFIRMED 表示发送不需要确认的报文.
 *
 * @param  [输入] unsigned char 类型的端口
 *
 * @param  [输入] char* 类型的发送数据
 *
 * @param  [输入] unsigned char 类型的发送数据长度
 *
 * @retval Not NULL 表示发送报文的Handler
 *
 * @retval NULL 表示发送失败，缓存队列满了
 */
OASIS_NETPKT_HANDLER OASISNET_SendPkt(OASIS_NETPKT_TYPE_E enConfirmType, uint8_t ucFPort, char * pData, uint8_t ucDataSize);


/**
 * @brief   Fport端口回调注册入口
 *
 * @param   [输入] unsigned char 类型的注册端口
 *          
 * @param   [输入] OASISNET_PKT_HANDLER_S* 类型的上层回调函数指针结构体
 *
 * @retval ERROR_SUCCESS 表示注册成功
 *
 * @retval ERROR_FAILED 表示注册失败
 */
uint8_t OASISNET_RegisterFRMPktProc(uint8_t ucFPort, OASISNET_PKT_HANDLER_S *pstPKtHandler);

/**
 * @brief   拉取数据请求
 *
 * @param   [输入] OASIS_NETPKT_TYPE_E 类型变量
 *               OASIS_NETPKT_UNCONFIRMED 表示非确认
 *               OASIS_NETPKT_CONFIRMED 表示需要确认
 */
void OASISNET_SendPullDataReq(OASIS_NETPKT_TYPE_E enPktType);

/**
 * @brief   网络层初始化
 *
 */
void OASISNET_Init(OASISNET_HANDLER_S *pstNetHandler, uint8_t ucCacheCnt);

/**
 * @brief   入网处理流程
 *
 */
 uint8_t OASISNET_Join(void);

/**
 * @brief   网络层运行函数
 *
 */
void OASISNET_Run(void);

/**
* @brief   设置智能调速开关.
*
* @param   [输入] bool类型的变量
*          ,true表示打开智能调速
*          ,false表示关闭智能调速
*/
void OASISNET_SetIntelligentDR(bool bOn);

/**
 * @brief   设置ADR开关
 *
 * @param   [输入] bool类型变量 true表示打开
 *                  ,false表示关闭
 */
uint8_t OASISNET_SetADR(bool bADROn);

/**
 * @brief   链路检查
 *
 */
void OASISNET_DoLinkCheck(void);



#endif /*__OASISNET_H__*/


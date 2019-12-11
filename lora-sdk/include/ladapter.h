 /**
 * @file        ladapter.h
 *
 * @copyright   Copyright (C) 2019, New H3C Technologies Co., Ltd.            
 *              License: Revised BSD License 
 *              ,see LICENSE.TXT file included in the project.
 *
 * @brief       lora adapt to uart or spi
 *                                                                                   
 * @code        
 *                          Author : wangzhen
 *
 *                          Project Code: oasis-sdk
 *
 *                          Module Name: lora adapt
 *
 *                          Date Created: 2019-10-26                             
 *                                                                                                                                  *
 *------------------------------------------------------------------------------*
 *  Modification History                                                        *
 *  DATE              NAME                  DESCRIPTION                         *
 *------------------------------------------------------------------------------*
 *  YYYY-MM-DD                                                                  *
 *------------------------------------------------------------------------------*                                                                             
 * @endcode   
 *
 * @defgroup  LADAPTER LoRa Adapt layer
 *            this module adapt to spi or uart.
 * @{
 */

#ifndef __LADAPTER_H__
#define __LADAPTER_H__

#include <stdbool.h>


/**
 * 入网方式
 */
typedef enum tagLAdapterNetworkType
{
    /** 
      * 空中入网 
      */
    LADAPTER_NWKTYPE_OTAA,
    /** 
      * 手动入网 
      */
    LADAPTER_NWKTYPE_ABP,
    /** 
      * 无效入网方式 
      */
    LADAPTER_NWKTYPE_MAX
}LADAPTER_NWKTYPE_E;
    
/**
 * 数据速率
 */
typedef enum tagLAdapterDR
{
    /**
     * CN470: SF12
     */
    LADAPTER_DR_0,
    /**
     * CN470: SF11
     */
    LADAPTER_DR_1,
    /**
     * CN470: SF10
     */
    LADAPTER_DR_2,
    /**
     * CN470: SF9
     */
    LADAPTER_DR_3,
    /**
     * CN470: SF8
     */
    LADAPTER_DR_4,
    /**
     * CN470: SF7
     */
    LADAPTER_DR_5,

    LADAPTER_DR_6,

    LADAPTER_DR_7,
}LADAPTER_DR_E;

/**
 * 发射功率
 */
typedef enum tagLAdapterPowerLevel
{
    /*
    * CN470: 17dbm 
    */
    LADAPTER_PWRLEVEL_0,
    /*
    * CN470: 15dbm 
    */
    LADAPTER_PWRLEVEL_1,
    /*
    * CN470: 13dbm 
    */
    LADAPTER_PWRLEVEL_2,
    /*
    * CN470: 11dbm 
    */
    LADAPTER_PWRLEVEL_3,
    /*
    * CN470: 9dbm 
    */
    LADAPTER_PWRLEVEL_4,
    /*
    * CN470: 7dbm 
    */
    LADAPTER_PWRLEVEL_5,
    /*
    * CN470: 5dbm 
    */
    LADAPTER_PWRLEVEL_6,
    /*
    * CN470: 3dbm 
    */
    LADAPTER_PWRLEVEL_7,
}LADAPTER_PWRLEVEL_E;

/**
 * 工作模式
 */
typedef enum tagLAdaterWorkMode
{
    /**
     * ClassA模式
     */
    LADAPTER_CLASS_A,
    /**
     * ClassB模式
     */
    LADAPTER_CLASS_B,
    /**
     * ClassC模式
     */
    LADAPTER_CLASS_C,
}LADAPTER_CLASS_E;
    
/**
 * 用于保存处理接收下行数据和确认的上层函数指针结构体
 */
typedef struct tagLAdapterPktHandler
{
    /**
     * 处理下行数据回调函数
     */    
    void ( *pfLAdapter_ProcFRMPktReceive )( uint8_t ucFPort, char *pcData, uint8_t ucDataLen, uint16_t usRSSI, uint8_t ucSNR);
    /**
     * 下行确认处理结果回调函数
     */
    void ( *pfLAdapter_ProcMcpsConfirm )( uint8_t ucResult);
}LADAPTER_PKTHANDLER_S;

/**
 * 用于保存处理mlme状态和事件的上层函数指针结构体
 */
typedef struct tagLAdapterHandler
{
    /**
     * 入网结果处理回调函数
     */
    void ( *pfLAdapter_JoinServer )(uint8_t ucJoinResult);
    /**
     * 处理接收Fpending标记回调函数
     */
    void ( *pfLAdapter_ReceiveFPending )(bool bFPending);
    /**
     * 获取温度回调函数
     */    
    float ( *pfLAdapter_GetTemperatureLevel)(void);
    /**
     * 获取电量回调函数
     */
    uint8_t ( *pfLAdapter_BoardGetBatteryLevel)( void );
    /**
     * 处理处于MacProcss状态回调函数
     */
    void ( *pfLAdapter_OnMacProcessNotify)( void );
}LADAPTER_HANDLER_S;

/**
 * @brief  适配层初始化
 *
 * @param  [输入] LADAPTER_HANDLER_S* Mac处理函数回调结构体
 *
 */
void LADAPTER_Init(LADAPTER_HANDLER_S *pHander);

/**
 * @brief  入网命令处理流程
 *
 * @retval ERROR_SUCCESS 表示入网命令处理失败
 *
 * @retval ERROR_FAILED 表示入网命令处理成功
 */
uint8_t LADAPTER_Join(void);

/**
 * @brief  适配层运行入口    
 */
void LADAPTER_Running(void);

uint8_t LADAPTER_Send(bool bConfirm, uint8_t ucFPort, char *pcData, uint8_t ucDataSize);


/**
 * @brief  报文处理回调注册入口
 *
 * @param  [输入] LADAPTER_PKTHANDLER_S收发包处理回调函数句柄
 *
 */
void LADAPTER_RegisterFRMPktProc(LADAPTER_PKTHANDLER_S *pstPKtHandler);



/**
 * @brief  获取当前是否入网
 *
 * @retval true 表示已入网
 *
 * @retval false 表示未入网
 */
bool LADAPTER_IsNetworkJoined(void);


/**
 * @brief  设置入网发射信道组的发射频率
 *
 * @param  [输入] unsigned char 类型的信道组,取值范围 1 ~ 12.
 *
 * @par 示例:ucChngGrp = 1表示设置入网发射频率470.3~471.7MHZ
 *
 * @retval LADAPTER_SUCCESS 表示设置成功
 *
 * @retval ERROR_FAILED 表示设置失败
 */
uint8_t LADAPTER_SetJoinChannelGroup(uint8_t ucChnlGrp);

/**
 * @brief  设置工作信道组
 *
 * @param  [输入] unsigned char 类型的信道组,取值范围 1 ~ 12.
 *
 * @par:示例:ucChngGrp = 1表示设置入网发射频率470.3~471.7MHZ 
 *
 * @retval LADAPTER_SUCCESS 表示设置成功
 *
 * @retval ERROR_FAILED 表示设置失败
 */
uint8_t LADAPTER_SetWorkChannelGroup(uint8_t ucChnlGrp);


/**
 * @brief  设置入网方式
 *
 * @param  [输入] LADAPTER_NWKTYPE_E 类型的入网方式. 
 *
 * @retval LADAPTER_SUCCESS 表示设置成功
 *
 * @retval ERROR_FAILED 表示设置失败
 */
uint8_t LADAPTER_SetNetwork(LADAPTER_NWKTYPE_E enNetWork);

/**
 * @brief  设置发射功率
 *
 * @param  [输入] LADAPTER_PWRLEVEL_E 类型的发射功率索引.
 *
 * @retval LADAPTER_SUCCESS 表示设置成功
 *
 * @retval ERROR_FAILED 表示设置失败
 */
uint8_t LADAPTER_SetTxPower(LADAPTER_PWRLEVEL_E enPowerLevel);

/**
 * @brief  设置发射速率
 *
 * @param  [输入] LADAPTER_DR_E 类型的数据速率索引
 *
 * @retval LADAPTER_SUCCESS 表示设置成功
 *
 * @retval ERROR_FAILED 表示设置失败
 */
uint8_t LADAPTER_SetDR(LADAPTER_DR_E enDR);

/**
 * @brief  设置LoRa工作模式
 *
 * @param  [输入] bool 类型的变量
 *             ,bAdrON = true 表示打开ADR功能
 *             ,bAdrON = false 表示关闭ADR功能.
 *
 * @retval LADAPTER_SUCCESS 表示设置成功
 *
 * @retval ERROR_FAILED 表示设置失败
 */
uint8_t LADAPTER_SetADR(bool bAdrON);

/**
 * @brief  设置TxDone后到RX1接收窗口打开的延时
 *
 * @param  [输入] unsigned char 类型的延时时间,单位是秒, 取值范围1 ~ 15
 *
 * @retval LADAPTER_SUCCESS 表示设置成功
 *
 * @retval ERROR_FAILED 表示设置失败
 */
uint8_t LADAPTER_SetRX1Delay(uint8_t ucRx1Delay);

/**
 * @brief  设置LoRa工作模式
 *
 * @param  [输入] LADAPTER_CLASS_E 类型的工作模式
 *        ,enClass =  LADAPTER_CLASS_A 表示ClassA模式
 *        ,enClass =  LADAPTER_CLASS_B 表示ClassB模式
 *        ,enClass =  LADAPTER_CLASS_C 表示ClassC模式
 *
 * @retval LADAPTER_SUCCESS 表示设置成功
 *
 * @retval ERROR_FAILED 表示设置失败
 */
uint8_t LADAPTER_SetClassMode(LADAPTER_CLASS_E enClass);

/**
 * @brief  设置设备地址
 *
 * @param  [输入] unsigned int类型的网络ID. 
 *
 * @retval LADAPTER_SUCCESS 表示设置成功
 *
 * @retval ERROR_FAILED 表示设置失败
 */
uint8_t LADAPTER_SetDevAddr(uint32_t uiDevAddr);

/**
 * @brief  设置人工入网版本
 *
 * @param  [输入] unsigned int 类型的版本号      
 */
uint8_t LADAPTER_SetABPVersion(uint32_t uiVersion);

/**
 * @brief  设置根密钥
 * 
 * @param  [输入] unsigned char *类型的指针,设置的根密钥. 
 * 
 * @retval LADAPTER_SUCCESS 表示设置成功
 *
 * @retval ERROR_FAILED 表示设置失败
 */
uint8_t LADAPTER_SetAPPKey(uint8_t *pucAppKey);

/**
 * @brief  设置网络根密钥
 * 
 * @param  [输入] unsigned char *类型的指针,设置的网络根密钥. 
 * 
 * @retval LADAPTER_SUCCESS 表示设置成功
 * 
 * @retval ERROR_FAILED 表示设置失败
 */
uint8_t LADAPTER_SetNWKKey(uint8_t *pucNwkKey);

/**
 * @brief  设置转发网络会话完整性密钥
 * 
 * @param  [输入] unsigned char *类型的指针,设置的转发网络会话完整性密钥. 
 *
 * @retval LADAPTER_SUCCESS 表示设置成功
 *
 * @retval ERROR_FAILED 表示设置失败
 */
uint8_t LADAPTER_SetFNWKKey(uint8_t *pucFNwkKey);

/**
 * @brief  设置入网会话加密密钥
 *
 * @param  [输入] unsigned char *类型的指针,设置的入网加密密钥. 
 * 
 * @retval LADAPTER_SUCCESS 表示设置成功
 *
 * @retval ERROR_FAILED 表示设置失败
 */
uint8_t LADAPTER_SetSNWKKey(uint8_t *pucSNwkKey);

/**
 * @brief  设置网络会话加密密钥
 *
 * @param  [输入] unsigned char *类型的指针,设置的网络会话加密密钥.
 *
 * @retval LADAPTER_SUCCESS 表示设置成功
 *
 * @retval ERROR_FAILED 表示设置失败
 */
uint8_t LADAPTER_SetNWKSKey(uint8_t *pucNwkSKey);

/**
 * @brief  设置应用会话密钥
 *
 * @param  [输入] unsigned char *类型的指针,设置的应用会话密钥.
 *
 * @retval LADAPTER_SUCCESS 表示设置成功
 *
 * @retval ERROR_FAILED 表示设置失败
 */
uint8_t LADAPTER_SetAPPSKey(uint8_t *pucAppSKey);

/**
 * @brief  设置JoinEui也称AppEui
 *
 * @param  [输入] unsigned char *类型的指针,设置的JoinEui.
 *
 * @retval LADAPTER_SUCCESS 表示设置成功
 *
 * @retval ERROR_FAILED 表示设置失败
 */
uint8_t LADAPTER_SetJoinEUI(uint8_t *pucJoinEUI);

/**
 * @brief  设置DeviceEui
 *
 * @param  [输入] unsigned char *类型的指针,设置的DeviceEui.
 *
 * @retval LADAPTER_SUCCESS 表示设置成功
 *
 * @retval ERROR_FAILED 表示设置失败
 */
uint8_t LADAPTER_SetDevEUI(uint8_t *pucDevEUI);

/**
 * @brief  设置DeviceEui
 *
 * @param  [输入] unsigned char *类型的指针,设置的DeviceEui. 
 *
 * @retval LADAPTER_SUCCESS 表示设置成功
 *
 * @retval ERROR_FAILED 表示设置失败
 */
uint8_t LADAPTER_SetNetID(uint32_t uiNetID);


#endif /*__LADAPTER_H__*/


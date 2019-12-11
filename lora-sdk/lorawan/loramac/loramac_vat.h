/**
* @file        loramac_vat.h
*
* @copyright   Copyright (C) 2019, New H3C Technologies Co., Ltd.            
*              License: Revised BSD License 
*              ,see LICENSE.TXT file included in the project.
*
* @brief       lora mac virtual mac command
*                                                                                   
* @code        
*                          Author : wangzhen
*
*                          Project Code: oasis-sdk
*
*                          Module Name: Mac Virtual Command                                                  
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
* @defgroup  LORAMACVAT LoRa Mac Virtual Command layer
*            this module sets LoRa Mac parameters
* @{
*/

#ifndef __LORAMAC_VAT_H__
#define __LORAMAC_VAT_H__


/**
 * @brief   获取当前入网状态
 *
 * @retval true 表示已经入网
 *
 * @retval false 表示未入网
 */
bool LORAMAC_VAT_IsNetworkJoined(void);

/**
 * @brief   设置LoRaMac层LoRaWan手动入网的版本号.
 *
 * @param   [输入] unsigned int类型手动入网的版本号.
 *
 * @retval  LORAMAC_STATUS_OK 表示设置成功
 *
 * @retval  !LORAMAC_STATUS_OK 表示设置失败
 *
 */
LoRaMacStatus_t LORAMAC_VAT_SetABPVersion(uint32_t uiABPVersion);

/**
 * @brief   设置LoRaMac层入网方式.
 *
 * @param   [输入] ActivationType_t类型入网方式.
 *
 * @retval  LORAMAC_STATUS_OK 表示设置成功
 *
 * @retval  !LORAMAC_STATUS_OK 表示设置失败
 *
 */
LoRaMacStatus_t LORAMAC_VAT_SetNetwork(ActivationType_t enActionType);

/**
 * @brief   设置LoRaMac层网络ID
 *
 * @param   [输入] unsigned int类型网络ID
 *
 * @retval  LORAMAC_STATUS_OK 表示设置成功
 *
 * @retval  !LORAMAC_STATUS_OK 表示设置失败
 *
 */
LoRaMacStatus_t LORAMAC_VAT_SetNetID(uint32_t uiNetID);

/**
 * @brief   设置LoRaMac层设备地址
 *
 * @param   [输入] unsigned int类型设备地址
 *
 * @retval  LORAMAC_STATUS_OK 表示设置成功
 *
 * @retval  !LORAMAC_STATUS_OK 表示设置失败
 *
 */
LoRaMacStatus_t LORAMAC_VAT_SetDevAddr(uint32_t uiDevAddr);

/**
 * @brief   设置LoRaMac层根密钥.
 *
 * @param   [输入] unsigned char*类型的指针,设置的根密钥.
 *
 * @retval  LORAMAC_STATUS_OK 表示设置成功
 *
 * @retval  !LORAMAC_STATUS_OK 表示设置失败
 *
 */
LoRaMacStatus_t LORAMAC_VAT_SetAPPKey(uint8_t *pucAppKey);

/**
 * @brief   设置LoRaMac层网络根密钥
 *
 * @param   [输入] unsigned char*类型的指针,设置的网络根密钥.
 *
 * @retval  LORAMAC_STATUS_OK 表示设置成功
 *
 * @retval  !LORAMAC_STATUS_OK 表示设置失败
 *
 */
LoRaMacStatus_t LORAMAC_VAT_SetNWKKey(uint8_t *pucNwkKey);

/**
 * @brief   设置LoRaMac层转发网络密钥完整性密钥
 *
 * @param   [输入] unsigned char*类型的指针,设置的转发网络会话完整性密钥.
 *
 * @retval  LORAMAC_STATUS_OK 表示设置成功
 *
 * @retval  !LORAMAC_STATUS_OK 表示设置失败
 *
 */
LoRaMacStatus_t LORAMAC_VAT_SetFNWKKey(uint8_t *pucFNwkKey);

/**
 * @brief   设置LoRaMac层入网加密密钥..
 *
 * @param   [输入] unsigned char*类型的指针,设置的入网加密密钥.
 *
 * @retval  LORAMAC_STATUS_OK 表示设置成功
 *
 * @retval  !LORAMAC_STATUS_OK 表示设置失败
 *
 */
LoRaMacStatus_t LORAMAC_VAT_SetSNWKKey(uint8_t *pucSNwkKey);

/**
 * @brief   设置LoRaMac层网络会话加密密钥.
 *
 * @param   [输入] unsigned char*类型的指针,设置的网络会话加密密钥.
 *
 * @retval  LORAMAC_STATUS_OK 表示设置成功
 *
 * @retval  !LORAMAC_STATUS_OK 表示设置失败
 *
 */
LoRaMacStatus_t LORAMAC_VAT_SetNWKSKey(uint8_t *pucNwkSKey);

/**
 * @brief   设置LoRaMac层应用会话密钥.
 *
 * @param   [输入] unsigned char*类型的指针,设置的应用会话密钥.
 *
 * @retval  LORAMAC_STATUS_OK 表示设置成功
 *
 * @retval  !LORAMAC_STATUS_OK 表示设置失败
 *
 */
LoRaMacStatus_t LORAMAC_VAT_SetAPPSKey(uint8_t *pucAppSKey);

/**
 * @brief   设置LoRaMac层设备EUI.
 *
 * @param   [输入] unsigned char*类型的指针,设置的设备EUI.
 *
 * @retval  LORAMAC_STATUS_OK 表示设置成功
 *
 * @retval  !LORAMAC_STATUS_OK 表示设置失败
 *
 */
LoRaMacStatus_t LORAMAC_VAT_SetDevEUI(uint8_t *pucDevEUI);

/**
 * @brief   设置LoRaMac层入网EUI也称应用EUI.
 *
 * @param   [输入] unsigned char*类型的指针,设置的入网EUI.
 *
 * @retval  LORAMAC_STATUS_OK 表示设置成功
 *
 * @retval  !LORAMAC_STATUS_OK 表示设置失败
 *
 */
LoRaMacStatus_t LORAMAC_VAT_SetJoinEUI(uint8_t *pucJoinEUI);

/**
 * @brief   设置LoRaMac层工作模式.
 *
 * @param   [输入] DeviceClass_t类型工作模式,
 *        ,enClass =  CLASS_A 表示ClassA模式
 *        ,enClass =  CLASS_B 表示ClassB模式
 *        ,enClass =  CLASS_C 表示ClassC模式
 *
 * @retval  LORAMAC_STATUS_OK 表示设置成功
 *
 * @retval  !LORAMAC_STATUS_OK 表示设置失败
 *
 */
LoRaMacStatus_t LORAMAC_VAT_SetClassMode(DeviceClass_t enClass);

/**
 * @brief   设置LoRaMac层自适应速率开关.
 *
 * @param   [输入] bool类型的使能开关
 *          ,bADRON = true 表示打开ADR功能
 *          ,bADRON = false 表示关闭ADR功能.
 *
 * @retval  LORAMAC_STATUS_OK 表示设置成功
 *
 * @retval  !LORAMAC_STATUS_OK 表示设置失败
 *
 */
LoRaMacStatus_t LORAMAC_VAT_SetADR(bool bADRON);

/**
 * @brief   设置LoRaMac层和vat层发射速率.
 *
 * @param   [输入] uinsined char类型的速率索引
 *
 * @retval  LORAMAC_STATUS_OK 表示设置成功
 *
 * @retval  !LORAMAC_STATUS_OK 表示设置失败
 *
 */
LoRaMacStatus_t LORAMAC_VAT_SetDR(uint8_t ucDR);

/**
 * @brief   设置LoRaMac层TxDone到Rx1窗口打开的延时.
 *
 * @param   [输入] uinsined char类型的延时时间,单位:秒.
 *
 * @retval  LORAMAC_STATUS_OK 表示设置成功
 *
 * @retval  !LORAMAC_STATUS_OK 表示设置失败
 *
 */
LoRaMacStatus_t LORAMAC_VAT_SetRX1Delay(uint8_t ucRx1Delay);

/**
 * @brief   设置LoRaMac层发射功率.
 *
 * @param   [输入] uinsined char类型的发射功率索引.
 *
 * @retval  LORAMAC_STATUS_OK 表示设置成功
 *
 * @retval  !LORAMAC_STATUS_OK 表示设置失败
 *
 */
LoRaMacStatus_t LORAMAC_VAT_SetTxPower(uint8_t ucTxPower);

/**
 * @brief   设置LoRaMac层默认发射信道的频率.
 *
 * @param   [输入] uinsined char类型的工作信道组索引.
 *
 * @par 示例:ucChnlGrp = 1表示设置入网发射频率470.3~471.7MHZ
 *
 * @retval  LORAMAC_STATUS_OK 表示设置成功
 *
 * @retval  !LORAMAC_STATUS_OK 表示设置失败
 *
 */
LoRaMacStatus_t LORAMAC_VAT_SetDefaultChannelGroup(uint8_t ucChnlGrp);

/**
 * @brief   设置LoRaMac层工作发射信道的频率.
 *
 * @param   [输入] uinsined char类型的工作信道组索引.
 *
 * @par 示例:ucChnlGrp = 1表示设置入网发射频率470.3~471.7MHZ
 *
 * @retval  LORAMAC_STATUS_OK 表示设置成功
 *
 * @retval  !LORAMAC_STATUS_OK 表示设置失败
 *
 */
LoRaMacStatus_t LORAMAC_VAT_SetWorkChannelGroup(uint8_t ucChnlGrp);

/**
 * @brief   发送数据.
 *
 * @param   [输入]   bool类型变量
 *          ,bConfirm = true 表示发送需要确认的报文 
 *          ,bConfirm = false 表示发送不需要确认的报文.
 *
 * @param   [输入] unsigned char 类型的端口
 *
 * @param   [输入] char* 类型的指针，指向发送数据的缓存
 *
 * @param   [输入] unsigned char 类型的发送数据长度
 *
 * @retval  LORAMAC_STATUS_OK 表示发送成功
 *
 * @retval  !LORAMAC_STATUS_OK 表示发送失败
 *
 */
LoRaMacStatus_t LORAMAC_VAT_Send(bool bConfirm, uint8_t ucFPort,
                                         char *pData, uint8_t ucDataSize);
/**
 * @brief   发送入网报文.
 *
 * @retval  LORAMAC_STATUS_OK 表示发送成功
 *
 * @retval  !LORAMAC_STATUS_OK 表示发送失败.
 *
 */
LoRaMacStatus_t LORAMAC_VAT_Join( void );

/**
 * @brief   虚拟命令层初始化.
 *
 * @param   [输入] LoRaMacCallback_t*类型的指针,存放Mac层回调函数.
 *
 * @param   [输入] LoRaMacPrimitives_t* 类型的指针,存放上层应用Mac事件处理函数
 *
 */
void LORAMAC_VAT_Init(LoRaMacCallback_t *pstMacCallbacks, 
                             LoRaMacPrimitives_t *pstMacPrimitives);


#endif /*__LORAMAC_VAT_H__*/



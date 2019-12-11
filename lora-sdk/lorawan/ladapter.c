/**
* @file        ladapter.c
*
* @copyright   Copyright (C) 2019, New H3C Technologies Co., Ltd.            
*              License: Revised BSD License 
*              ,see LICENSE.TXT file included in the project.
*
* @brief       adapt to SPI or Serial Command
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
*/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ladapter.h"
#include "base.h"

#if !defined( LORA_MODULE_WSL30X )
#include "LoRaMac.h"
#include "loramac_vat.h"
#include "NvmCtxMgmt.h"
#else
#include "wsl305s.h"
#endif
#include "Commissioning.h"



/** 下行数据回调处理函数指针结构体*/
LADAPTER_PKTHANDLER_S g_apfLADPTPktHandler = {NULL};

/** Mac处理回调函数注册结构体*/
LADAPTER_HANDLER_S g_stLoraAdpHandler = {NULL,};


/**
 * @brief   适配层报文应答处理函数
 *
 * @param   [输入] unsigned char 类型的参数,表示应答处理结果.
 *             
 */
static void LadapterProcMcpsConfirm( uint8_t ucRet )
{
    if (g_apfLADPTPktHandler.pfLAdapter_ProcMcpsConfirm != NULL)
    {
        g_apfLADPTPktHandler.pfLAdapter_ProcMcpsConfirm(ucRet);
    }

    return;
}


#if !defined( LORA_MODULE_WSL30X )

/**
 * @brief   MCPS-Confirm类型的函数事件
 *
 * @param   [输入] McpsConfirm_t * 类型的指针
 *                 ,包含Mcps类型的确认信息.
 */
static void McpsConfirm( McpsConfirm_t *pstMcpsConfirm )
{
    uint8_t ucRet = ERROR_SUCCESS;
    
    if( pstMcpsConfirm->Status != LORAMAC_EVENT_INFO_STATUS_OK )
    {
        ucRet = ERROR_FAILED;
    }

    LadapterProcMcpsConfirm(ucRet);
    
    return;
}


/**
 * @brief   MCPS-Indication类型的函数事件
 *
 * @param   [输入] mcpsIndication 类型的指针，
 *               包含着Mcps类型的下行指示信息.
 */
static void McpsIndication( McpsIndication_t *pstMcpsIndication )
{
    if( pstMcpsIndication->Status != LORAMAC_EVENT_INFO_STATUS_OK )
    {
        return;
    }

    switch( pstMcpsIndication->McpsIndication )
    {
        case MCPS_UNCONFIRMED:
        {
            break;
        }
        case MCPS_CONFIRMED:
        {
            break;
        }
        case MCPS_PROPRIETARY:
        {
            break;
        }
        case MCPS_MULTICAST:
        {
            break;
        }
        default:
            break;
    }

    if( (pstMcpsIndication->RxData == true) && 
        (pstMcpsIndication->Port > 0) &&
        (g_apfLADPTPktHandler.pfLAdapter_ProcFRMPktReceive != NULL))
    {
        g_apfLADPTPktHandler.pfLAdapter_ProcFRMPktReceive(pstMcpsIndication->Port,
                                       (char *)pstMcpsIndication->Buffer,pstMcpsIndication->BufferSize,
                                       (uint16_t)pstMcpsIndication->Rssi,(uint8_t)pstMcpsIndication->Snr);
    }

    if((g_stLoraAdpHandler.pfLAdapter_ReceiveFPending != NULL) && (pstMcpsIndication->FramePending == true))
    {
        g_stLoraAdpHandler.pfLAdapter_ReceiveFPending((bool)pstMcpsIndication->FramePending);
    }

    return;
}

/**
 * @brief   MLME-Confirm类型的函数事件
 *
 * @param   [输入] mlmeConfirm_t *类型的指针,
 *               包含Mlme类型的确认信息.
 */
static void MlmeConfirm( MlmeConfirm_t *pstMlmeConfirm )
{
    switch( pstMlmeConfirm->MlmeRequest )
    {
        case MLME_JOIN:
        {
            if(g_stLoraAdpHandler.pfLAdapter_JoinServer != NULL)
            {
                g_stLoraAdpHandler.pfLAdapter_JoinServer((uint8_t)pstMlmeConfirm->Status);
            }
            break;
        }
        case MLME_LINK_CHECK:
        {
            break;
        }
        default:
            break;
    }

    return;
}

/**
 * @brief   MLME-Indication类型的函数事件
 *
 * @param   [输入] MlmeIndication_t*类型的指针
 *              ,保存着Mlme类型的指示信息.
 */
static void MlmeIndication( MlmeIndication_t *pstMlmeIndication )
{
    if( pstMlmeIndication->Status != LORAMAC_EVENT_INFO_STATUS_BEACON_LOCKED )
    {
    
    }
    if( pstMlmeIndication->Status != LORAMAC_EVENT_INFO_STATUS_OK )
    {
    
    }

    switch( pstMlmeIndication->MlmeIndication )
    {
        case MLME_SCHEDULE_UPLINK:
        {// The MAC signals that we shall provide an uplink as soon as possible
            //OnTxNextPacketTimerEvent( NULL ); delete by yuxin
            break;
        }
        default:
            break;
    }

    return;
}


#else

/**
 * @brief   应答报文类型处理函数
 *
 * @param   [输入] bool 类型的参数,bConfirmed = true表示收到需要确认的报文   
 *                  ,bConfirmed = false 表示收到不需要回复确认的报文              
 */
static void Wsl305sMcpsConfirm( bool bConfirmePkt, uint8_t ucResult )
{
    LadapterProcMcpsConfirm(ucResult);
    
    return;
}

/**
 * @brief   Mcps类型指示报文处理回调
 *
 * @param   [输入] bool 类型的参数,bConfirmed = true表示收到需要确认的报文   
 *                  ,bConfirmed = false 表示收到不需要回复确认的报文              
 */
static void Wsl305sMcpsIndication( bool bFPending, uint8_t ucFPort, char *pcData, uint8_t ucRecvLen)
{
    uint16_t usRSSI = 0;
    uint8_t  ucSNR = 0;
    
    if( (ucFPort > 0) && (ucRecvLen > 0) &&
        (g_apfLADPTPktHandler.pfLAdapter_ProcFRMPktReceive != NULL))
    {
        /*获取报文质量*/
        (void)WSL305S_AT_GetPktQuality(&usRSSI, &ucSNR);
        g_apfLADPTPktHandler.pfLAdapter_ProcFRMPktReceive(ucFPort, pcData, ucRecvLen, usRSSI, ucSNR);
    }

    if((g_stLoraAdpHandler.pfLAdapter_ReceiveFPending != NULL) && (bFPending == true))
    {
        g_stLoraAdpHandler.pfLAdapter_ReceiveFPending(bFPending);
    }

    return;
}

/**
 * @brief   Mlme类型应答报文处理回调
 *
 * @param   [输入] bool 类型的参数,bJoined = true表示已经入网  
 *                  ,bJoined = false 表示还未入网              
 */
static void Wsl305sMlmeConfirm( bool bJoined )
{
    uint8_t ucRet = ERROR_FAILED;

    if (bJoined)
    {
        ucRet = ERROR_SUCCESS;
    }
    
    if (g_stLoraAdpHandler.pfLAdapter_JoinServer != NULL)
    {
        g_stLoraAdpHandler.pfLAdapter_JoinServer(ucRet);
    }

    return;
}


#endif

uint8_t LADAPTER_SetABPVersion(uint32_t uiVersion)
{
    uint8_t ucRet = ERROR_SUCCESS;
    
#if !defined( LORA_MODULE_WSL30X )
    if (LORAMAC_STATUS_OK != LORAMAC_VAT_SetABPVersion(uiVersion))
    {
        ucRet = ERROR_FAILED;
    }
#endif

    return ucRet;
}

uint8_t LADAPTER_SetAPPKey(uint8_t *pucAppKey)
{
    uint8_t ucRet = ERROR_SUCCESS;
    
#if defined( LORA_MODULE_WSL30X )
    ucRet = WSL305S_AT_SetAppKey((char *)pucAppKey);
#else
    if (LORAMAC_STATUS_OK != LORAMAC_VAT_SetAPPKey(pucAppKey))
    {
        ucRet = ERROR_FAILED;
    }
#endif

    return ucRet;
}

uint8_t LADAPTER_SetNWKKey(uint8_t *pucNwkKey)
{
    uint8_t ucRet = ERROR_SUCCESS;
    
#if !defined( LORA_MODULE_WSL30X )
    if (LORAMAC_STATUS_OK != LORAMAC_VAT_SetNWKKey(pucNwkKey))
    {
        ucRet = ERROR_FAILED;
    }
#endif

    return ucRet;
}

uint8_t LADAPTER_SetFNWKKey(uint8_t *pucFNwkKey)
{
    uint8_t ucRet = ERROR_SUCCESS;
    
#if !defined( LORA_MODULE_WSL30X )
    if (LORAMAC_STATUS_OK != LORAMAC_VAT_SetFNWKKey(pucFNwkKey))
    {
        ucRet = ERROR_FAILED;
    }
#endif

    return ucRet;
}

uint8_t LADAPTER_SetSNWKKey(uint8_t *pucSNwkKey)
{
    uint8_t ucRet = ERROR_SUCCESS;
    
#if !defined( LORA_MODULE_WSL30X )
    if (LORAMAC_STATUS_OK != LORAMAC_VAT_SetSNWKKey(pucSNwkKey))
    {
        ucRet = ERROR_FAILED;
    }
#endif

    return ucRet;
}

uint8_t LADAPTER_SetNWKSKey(uint8_t *pucNwkSKey)
{
    uint8_t ucRet = ERROR_SUCCESS;
    
#if defined( LORA_MODULE_WSL30X )
    ucRet = WSL305S_AT_SetATCMD(AT_NWKSKEY_SET, (char *)pucNwkSKey);
#else
    if (LORAMAC_STATUS_OK != LORAMAC_VAT_SetNWKSKey(pucNwkSKey))
    {
        ucRet = ERROR_FAILED;
    }
#endif

    return ucRet;
}

uint8_t LADAPTER_SetAPPSKey(uint8_t *pucAppSKey)
{
    uint8_t ucRet = ERROR_SUCCESS;
    
#if defined( LORA_MODULE_WSL30X )
    ucRet = WSL305S_AT_SetATCMD(AT_APPSKEY_SET, (char *)pucAppSKey);
#else
    if (LORAMAC_STATUS_OK != LORAMAC_VAT_SetAPPSKey(pucAppSKey))
    {
        ucRet = ERROR_FAILED;
    }
#endif

    return ucRet;
}

uint8_t LADAPTER_SetJoinEUI(uint8_t *pucJoinEUI)
{
    uint8_t ucRet = ERROR_SUCCESS;
    
#if defined( LORA_MODULE_WSL30X )
    ucRet = WSL305S_AT_SetAppEUI((char *)pucJoinEUI);
#else
    if (LORAMAC_STATUS_OK != LORAMAC_VAT_SetJoinEUI(pucJoinEUI))
    {
        ucRet = ERROR_FAILED;
    }
#endif

    return ucRet;
}

uint8_t LADAPTER_SetDevEUI(uint8_t *pucDevEUI)
{
    uint8_t ucRet = ERROR_SUCCESS;
    
#if defined( LORA_MODULE_WSL30X )
    ucRet = WSL305S_AT_SetDevEUI((char *)pucDevEUI);
#else
    if (LORAMAC_STATUS_OK != LORAMAC_VAT_SetDevEUI(pucDevEUI))
    {
        ucRet = ERROR_FAILED;
    }
#endif

    return ucRet;
}

uint8_t LADAPTER_SetNetID(uint32_t uiNetID)
{
    uint8_t ucRet = ERROR_SUCCESS;
    
#if !defined( LORA_MODULE_WSL30X )
    ucRet = LORAMAC_VAT_SetNetID(uiNetID);
#endif

    return ucRet;
}

uint8_t LADAPTER_SetDevAddr(uint32_t uiDevAddr)
{
    uint8_t ucRet = ERROR_SUCCESS;
    
#if defined( LORA_MODULE_WSL30X )
    ucRet = WSL305S_AT_SetDevAddr(uiDevAddr);
#else
    if (LORAMAC_STATUS_OK != LORAMAC_VAT_SetDevAddr(uiDevAddr))
    {
        ucRet = ERROR_FAILED;
    }
#endif

    return ucRet;
}

uint8_t LADAPTER_SetNetwork(LADAPTER_NWKTYPE_E enNetWork)
{
    uint8_t ucRet = ERROR_SUCCESS;
    
#if defined( LORA_MODULE_WSL30X )
    char *pcValue = NJM_SET_OTAA;

    if (LADAPTER_NWKTYPE_OTAA != enNetWork)
    {
        pcValue = NJM_SET_ABP;
    }
    ucRet = WSL305S_AT_SetATCMD(AT_NJM_SET, pcValue);
#else
    ActivationType_t aNetWorkMap[LADAPTER_NWKTYPE_MAX] = {ACTIVATION_TYPE_OTAA, ACTIVATION_TYPE_ABP};

    if (LORAMAC_STATUS_OK != LORAMAC_VAT_SetNetwork(aNetWorkMap[enNetWork]))
    {
        ucRet = ERROR_FAILED;
    }
#endif

    return ucRet;
}

uint8_t LADAPTER_SetClassMode(LADAPTER_CLASS_E enClass)
{
    uint8_t ucRet = ERROR_SUCCESS;
    
#if defined( LORA_MODULE_WSL30X )
    char auCmdData[2] = {'A', 0};

    auCmdData[0] = auCmdData[0] + (uint8_t)enClass;
    ucRet = WSL305S_AT_SetATCMD(AT_CLASS_SET, auCmdData);
#else
    if (LORAMAC_STATUS_OK != LORAMAC_VAT_SetClassMode((DeviceClass_t)enClass))
    {
        ucRet = ERROR_FAILED;
    }
#endif

    return ucRet;
}

uint8_t LADAPTER_SetADR(bool bAdrON)
{
    uint8_t ucRet = ERROR_SUCCESS;
    
#if defined( LORA_MODULE_WSL30X )
    char auCmdData[2] = {'0', 0};

    if(true == bAdrON)
    {
        auCmdData[0] = '1';
    }
    ucRet = WSL305S_AT_SetATCMD(AT_ADR_SET, auCmdData);
#else
    if (LORAMAC_STATUS_OK != LORAMAC_VAT_SetADR(bAdrON))
    {
        ucRet = ERROR_FAILED;
    }
#endif

    return ucRet;
}

uint8_t LADAPTER_SetDR(LADAPTER_DR_E enDR)
{
    uint8_t ucRet = ERROR_SUCCESS;
    
#if defined( LORA_MODULE_WSL30X )
    char auCmdData[2] = {0};

    auCmdData[0] = '0';
    auCmdData[0] = auCmdData[0] + enDR;
    ucRet = WSL305S_AT_SetATCMD(AT_DR_SET, auCmdData);
#else
    if (LORAMAC_STATUS_OK != LORAMAC_VAT_SetDR((uint8_t)enDR))
    {
        ucRet = ERROR_FAILED;
    }
#endif

    return ucRet;
}

uint8_t LADAPTER_SetRX1Delay(uint8_t ucRx1Delay)
{
    uint8_t ucRet = ERROR_SUCCESS;

    if ((ucRx1Delay < 1) || (ucRx1Delay > 15))
    {
        return ERROR_FAILED;
    }
    
#if defined( LORA_MODULE_WSL30X )
    ucRet = WSL305S_AT_SetRX1Delay(ucRx1Delay);
#else
    if (LORAMAC_STATUS_OK != LORAMAC_VAT_SetRX1Delay(ucRx1Delay))
    {
        ucRet = ERROR_FAILED;
    }
#endif

    return ucRet;
}

uint8_t LADAPTER_SetTxPower(LADAPTER_PWRLEVEL_E enPowerLevel)
{
    uint8_t ucRet = ERROR_SUCCESS;
    
#if defined( LORA_MODULE_WSL30X )
    char auCmdData[2] = {0};

    auCmdData[0] = '0';
    auCmdData[0] = auCmdData[0] + (uint8_t)enPowerLevel;
    ucRet = WSL305S_AT_SetATCMD(AT_TXP_SET, auCmdData);
#else
    if (LORAMAC_STATUS_OK != LORAMAC_VAT_SetTxPower((uint8_t)enPowerLevel))
    {
        ucRet = ERROR_FAILED;
    }
#endif

    return ucRet;
}

bool LADAPTER_IsNetworkJoined(void)
{
#if defined( LORA_MODULE_WSL30X )
    return WSL305S_AT_IsNetworkJoined();
#else
    return LORAMAC_VAT_IsNetworkJoined();
#endif
}

uint8_t LADAPTER_SetJoinChannelGroup(uint8_t ucChnlGrp)
{
    uint8_t ucRet = ERROR_SUCCESS;

    //Only CN470 surpport 
    if ((ucChnlGrp > 12) || (ucChnlGrp == 0))
    {
        return ERROR_FAILED;
    }
    
#if defined( LORA_MODULE_WSL30X )
    ucRet = WSL305S_AT_SetChannelGroup(ucChnlGrp);
#else
    if (LORAMAC_STATUS_OK != LORAMAC_VAT_SetDefaultChannelGroup(ucChnlGrp))
    {
        ucRet = ERROR_FAILED;
    }
#endif
    return ucRet;
}

uint8_t LADAPTER_SetWorkChannelGroup(uint8_t ucChnlGrp)
{
    uint8_t ucRet = ERROR_SUCCESS;
    
    //Only CN470 surpport 
    if ((ucChnlGrp > 12) || (ucChnlGrp == 0))
    {
        return ERROR_FAILED;
    }    
#if defined( LORA_MODULE_WSL30X )
    ucRet = WSL305S_AT_SetChannelGroup(ucChnlGrp);
#else
    if (LORAMAC_STATUS_OK != LORAMAC_VAT_SetWorkChannelGroup(ucChnlGrp))
    {
        ucRet = ERROR_FAILED;
    }
#endif
    return ucRet;
}

void LADAPTER_RegisterFRMPktProc(LADAPTER_PKTHANDLER_S *pstPKtHandler)
{
    if (pstPKtHandler != NULL)
    {
        g_apfLADPTPktHandler = *pstPKtHandler;
    }
    
    return;
}

/**
 * @brief   Mlme类型应答报文处理回调
 *
 * @param   [输入] bool 类型的参数,bJoined = true表示已经入网  
 *                  ,bJoined = false 表示还未入网              
 */
uint8_t LADAPTER_Send(bool bConfirm, uint8_t ucFPort, char *pcData, uint8_t ucDataSize)
{
    uint8_t ucRet = ERROR_SUCCESS;

#if defined( LORA_MODULE_WSL30X )
    ucRet = WSL305S_AT_Send(bConfirm, ucFPort, pcData, ucDataSize);
#else
    if (LORAMAC_STATUS_OK != LORAMAC_VAT_Send(bConfirm, ucFPort, pcData, ucDataSize))
    {
        ucRet = ERROR_FAILED;
    }
#endif

    return ucRet;
}

void LADAPTER_Running(void)
{
#if defined( LORA_MODULE_WSL30X )
    WSL305S_AT_Running();
#else
    LoRaMacProcess();
#endif
    return;
}

uint8_t LADAPTER_Join(void)
{
    uint8_t ucRet = ERROR_SUCCESS;
    
#if defined( LORA_MODULE_WSL30X )
    ucRet = WSL305S_AT_Join();
#else
    if (LORAMAC_STATUS_OK != LORAMAC_VAT_Join())
    {
        ucRet = ERROR_FAILED;
    }
#endif

    return ucRet;
}

void LADAPTER_Init(LADAPTER_HANDLER_S *pstHandler)
{
#if defined( LORA_MODULE_WSL30X )
    WSL305S_CALLBACK_S stWsl305Callback = {NULL,};

    stWsl305Callback.Wsl305sMcpsConfirm = Wsl305sMcpsConfirm;
    stWsl305Callback.Wsl305sMcpsIndication = Wsl305sMcpsIndication;
    stWsl305Callback.Wsl305sMlmeConfirm = Wsl305sMlmeConfirm;
    
    WSL305S_AT_Init(&stWsl305Callback);

#else
    LoRaMacCallback_t stMacCallbacks = {NULL};
    LoRaMacPrimitives_t stMacPrimitives = {NULL};

    stMacPrimitives.MacMcpsConfirm = McpsConfirm;
    stMacPrimitives.MacMcpsIndication = McpsIndication;
    stMacPrimitives.MacMlmeConfirm = MlmeConfirm;
    stMacPrimitives.MacMlmeIndication = MlmeIndication;
    stMacCallbacks.GetBatteryLevel = pstHandler->pfLAdapter_BoardGetBatteryLevel;
    stMacCallbacks.GetTemperatureLevel = pstHandler->pfLAdapter_GetTemperatureLevel;
    stMacCallbacks.NvmContextChange = NvmCtxMgmtEvent;
    stMacCallbacks.MacProcessNotify = pstHandler->pfLAdapter_OnMacProcessNotify;

    LORAMAC_VAT_Init(&stMacCallbacks, &stMacPrimitives);
#endif

    g_stLoraAdpHandler = *pstHandler;

    return;
}



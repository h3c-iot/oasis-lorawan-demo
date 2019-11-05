/*******************************************************************************
*                                                                              *
*                Copyright (C) 2019, New H3C Technologies Co., Ltd.            *
*  License: Revised BSD License, see LICENSE.TXT file included in the project  *
*                                                                              *
*------------------------------------------------------------------------------*
*                                                                              *
*                              ladapter.c                                        *
*                                                                              *
*  Project Code: oasis-sdk                                                     *
*   Module Name: lora adapter                                               *
*  Date Created: 2019-10-22                                                    *
*        Author: yys1819                                                       *
*   Description: adapt to SPI or Serial Command                                *
*                                                                              *
*------------------------------------------------------------------------------*
*  Modification History                                                        *
*  DATE        NAME             DESCRIPTION                                    *
*  ----------------------------------------------------------------------------*
*  YYYY-MM-DD                                                                  *
*                                                                              *
*******************************************************************************/


#include <stdio.h>
#include "ladapter.h"
#if !defined( LORA_MODULE_WSL30X )
#include "LoRaMac.h"
#include "loramac_vat.h"
#include "NvmCtxMgmt.h"
#else
#include "wsl305s.h"
#endif
#include "Commissioning.h"



LADAPTER_ProcessFRMPkt_PF g_apfFRMPayloadHandler[255] = {NULL,};

LADAPTER_HANDLER_S g_stLoraAdpHandler = {NULL,};

#if !defined( LORA_MODULE_WSL30X )

/*!
 * \brief   MCPS-Confirm event function
 *
 * \param   [IN] mcpsConfirm - Pointer to the confirm structure,
 *               containing confirm attributes.
 */
static void McpsConfirm( McpsConfirm_t *pstMcpsConfirm )
{
    uint8_t ucRet = LADAPTER_SUCCES;
    bool bConfirm = false;
    
    if( pstMcpsConfirm->Status != LORAMAC_EVENT_INFO_STATUS_OK )
    {
        ucRet = LADAPTER_FAILED;
    }

    if ( pstMcpsConfirm->McpsRequest == MCPS_CONFIRMED )
    {
        bConfirm = true;
    }

    if(g_stLoraAdpHandler.pfLAdapter_McpsConfirm != NULL)
    {
        g_stLoraAdpHandler.pfLAdapter_McpsConfirm(bConfirm, ucRet);
    }
    
    return;
}


/*!
 * \brief   MCPS-Indication event function
 *
 * \param   [IN] mcpsIndication - Pointer to the indication structure,
 *               containing indication attributes.
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
        (g_apfFRMPayloadHandler[pstMcpsIndication->Port] != NULL))
    {
        g_apfFRMPayloadHandler[pstMcpsIndication->Port]((char *)pstMcpsIndication->Buffer,
                                                       pstMcpsIndication->BufferSize);
    }

    if(g_stLoraAdpHandler.pfLAdapter_ReceiveFPending != NULL)
    {
        g_stLoraAdpHandler.pfLAdapter_ReceiveFPending((bool)pstMcpsIndication->FramePending);
    }

    return;
}

/*!
 * \brief   MLME-Confirm event function
 *
 * \param   [IN] mlmeConfirm - Pointer to the confirm structure,
 *               containing confirm attributes.
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

/*!
 * \brief   MLME-Indication event function
 *
 * \param   [IN] mlmeIndication - Pointer to the indication structure.
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

static void Wsl305sMcpsConfirm( bool bConfirmePkt, uint8_t ucResult )
{
    uint8_t ucRet = LADAPTER_SUCCES;
    
    if( ucResult != AT_CMD_SUCCESS )
    {
        ucRet = LADAPTER_FAILED;
    }

    if(g_stLoraAdpHandler.pfLAdapter_McpsConfirm != NULL)
    {
        g_stLoraAdpHandler.pfLAdapter_McpsConfirm(bConfirmePkt, ucRet);
    }
    
    return;
}

static void Wsl305sMcpsIndication( bool bFPending, uint8_t ucFPort, char *pcData, uint8_t ucRecvLen)
{
    if( (ucFPort > 0) &&
        (g_apfFRMPayloadHandler[ucFPort] != NULL))
    {
        g_apfFRMPayloadHandler[ucFPort](pcData, ucRecvLen);
    }

    if(g_stLoraAdpHandler.pfLAdapter_ReceiveFPending != NULL)
    {
        g_stLoraAdpHandler.pfLAdapter_ReceiveFPending(bFPending);
    }

    return;
}

static void Wsl305sMlmeConfirm( bool bJoined )
{
    uint8_t ucRet = LADAPTER_FAILED;

    if (bJoined)
    {
        ucRet = LADAPTER_SUCCES;
    }
    
    if(g_stLoraAdpHandler.pfLAdapter_JoinServer != NULL)
    {
        g_stLoraAdpHandler.pfLAdapter_JoinServer(ucRet);
    }

    return;
}


#endif


/**
 * Process MacRunning.
 */ 
void LADAPTER_Running(void)
{
#if defined( LORA_MODULE_WSL30X )
    WSL305S_AT_Running();
#else
    LoRaMacProcess();
#endif
    return;
}

uint8_t LADAPTER_SetABPVersion(uint32_t uiVersion)
{
    uint8_t ucRet = LADAPTER_SUCCES;
    
#if !defined( LORA_MODULE_WSL30X )
    if (LORAMAC_STATUS_OK != LORAMAC_VAT_SetABPVersion(uiVersion))
    {
        ucRet = LADAPTER_FAILED;
    }
#endif

    return ucRet;
}


uint8_t LADAPTER_SetAPPKey(uint8_t *pucAppKey)
{
    uint8_t ucRet = LADAPTER_SUCCES;
    
#if defined( LORA_MODULE_WSL30X )
    if (AT_CMD_SUCCESS != WSL305S_AT_SetAppKey((char *)pucAppKey))
    {
        ucRet = LADAPTER_FAILED;
    }
#else
    if (LORAMAC_STATUS_OK != LORAMAC_VAT_SetAPPKey(pucAppKey))
    {
        ucRet = LADAPTER_FAILED;
    }
#endif

    return ucRet;
}

uint8_t LADAPTER_SetNWKKey(uint8_t *pucNwkKey)
{
    uint8_t ucRet = LADAPTER_SUCCES;
    
#if !defined( LORA_MODULE_WSL30X )
    if (LORAMAC_STATUS_OK != LORAMAC_VAT_SetNWKKey(pucNwkKey))
    {
        ucRet = LADAPTER_FAILED;
    }
#endif

    return ucRet;
}

uint8_t LADAPTER_SetFNWKKey(uint8_t *pucFNwkKey)
{
    uint8_t ucRet = LADAPTER_SUCCES;
    
#if !defined( LORA_MODULE_WSL30X )
    if (LORAMAC_STATUS_OK != LORAMAC_VAT_SetFNWKKey(pucFNwkKey))
    {
        ucRet = LADAPTER_FAILED;
    }
#endif

    return ucRet;
}

uint8_t LADAPTER_SetSNWKKey(uint8_t *pucSNwkKey)
{
    uint8_t ucRet = LADAPTER_SUCCES;
    
#if !defined( LORA_MODULE_WSL30X )
    if (LORAMAC_STATUS_OK != LORAMAC_VAT_SetSNWKKey(pucSNwkKey))
    {
        ucRet = LADAPTER_FAILED;
    }
#endif

    return ucRet;
}

uint8_t LADAPTER_SetNWKSKey(uint8_t *pucNwkSKey)
{
    uint8_t ucRet = LADAPTER_SUCCES;
    
#if defined( LORA_MODULE_WSL30X )
    if (AT_CMD_SUCCESS != WSL305S_AT_SetATCMD(AT_NWKSKEY_SET, (char *)pucNwkSKey))
    {
        ucRet = LADAPTER_FAILED;
    }
#else
    if (LORAMAC_STATUS_OK != LORAMAC_VAT_SetNWKSKey(pucNwkSKey))
    {
        ucRet = LADAPTER_FAILED;
    }
#endif

    return ucRet;
}

uint8_t LADAPTER_SetAPPSKey(uint8_t *pucAppSKey)
{
    uint8_t ucRet = LADAPTER_SUCCES;
    
#if defined( LORA_MODULE_WSL30X )
    if (AT_CMD_SUCCESS != WSL305S_AT_SetATCMD(AT_APPSKEY_SET, (char *)pucAppSKey))
    {
        ucRet = LADAPTER_FAILED;
    }
#else
    if (LORAMAC_STATUS_OK != LORAMAC_VAT_SetAPPSKey(pucAppSKey))
    {
        ucRet = LADAPTER_FAILED;
    }
#endif


    return ucRet;
}

uint8_t LADAPTER_SetJoinEUI(uint8_t *pucJoinEUI)
{
    uint8_t ucRet = LADAPTER_SUCCES;
    
#if defined( LORA_MODULE_WSL30X )
    if (AT_CMD_SUCCESS != WSL305S_AT_SetAppEUI((char *)pucJoinEUI))
    {
        ucRet = LADAPTER_FAILED;
    }
#else
    if (LORAMAC_STATUS_OK != LORAMAC_VAT_SetJoinEUI(pucJoinEUI))
    {
        ucRet = LADAPTER_FAILED;
    }
#endif

    return ucRet;
}

uint8_t LADAPTER_SetDevEUI(uint8_t *pucDevEUI)
{
    uint8_t ucRet = LADAPTER_SUCCES;
    
#if defined( LORA_MODULE_WSL30X )
    if (AT_CMD_SUCCESS != WSL305S_AT_SetDevEUI((char *)pucDevEUI))
    {
        ucRet = LADAPTER_FAILED;
    }
#else
    if (LORAMAC_STATUS_OK != LORAMAC_VAT_SetDevEUI(pucDevEUI))
    {
        ucRet = LADAPTER_FAILED;
    }
#endif

    return ucRet;
}

uint8_t LADAPTER_SetNetID(uint32_t uiNetID)
{
    uint8_t ucRet = LADAPTER_SUCCES;
    
#if !defined( LORA_MODULE_WSL30X )
    if (LORAMAC_STATUS_OK != LORAMAC_VAT_SetNetID(uiNetID))
    {
        ucRet = LADAPTER_FAILED;
    }
#endif

    return ucRet;
}

uint8_t LADAPTER_SetDevAddr(uint32_t uiDevAddr)
{
    uint8_t ucRet = LADAPTER_SUCCES;
    
#if defined( LORA_MODULE_WSL30X )
    if (AT_CMD_SUCCESS != WSL305S_AT_SetDevAddr(uiDevAddr))
    {
        ucRet = LADAPTER_FAILED;
    }
#else
    if (LORAMAC_STATUS_OK != LORAMAC_VAT_SetDevAddr(uiDevAddr))
    {
        ucRet = LADAPTER_FAILED;
    }
#endif


    return ucRet;
}


uint8_t LADAPTER_SetNetwork(LADAPTER_NWKTYPE_E enNetWork)
{
    uint8_t ucRet = LADAPTER_SUCCES;
    
#if defined( LORA_MODULE_WSL30X )
    char *pcValue = NJM_SET_OTAA;

    if (LADAPTER_NWKTYPE_OTAA != enNetWork)
    {
        pcValue = NJM_SET_ABP;
    }
    if ( AT_CMD_SUCCESS != WSL305S_AT_SetATCMD(AT_NJM_SET, pcValue))
    {
        ucRet = LADAPTER_FAILED;
    }
#else
    ActivationType_t aNetWorkMap[LADAPTER_NWKTYPE_MAX] = {ACTIVATION_TYPE_OTAA, ACTIVATION_TYPE_ABP};

    if (LORAMAC_STATUS_OK != LORAMAC_VAT_SetNetwork(aNetWorkMap[enNetWork]))
    {
        ucRet = LADAPTER_FAILED;
    }
#endif

    return ucRet;
}

uint8_t LADAPTER_SetClassMode(LADAPTER_CLASS_E enClass)
{
    uint8_t ucRet = LADAPTER_SUCCES;
    
#if defined( LORA_MODULE_WSL30X )
    char auCmdData[2] = {'A', 0};

    auCmdData[0] = auCmdData[0] + (uint8_t)enClass;
    if (AT_CMD_SUCCESS != WSL305S_AT_SetATCMD(AT_CLASS_SET, auCmdData))
    {
        ucRet = LADAPTER_FAILED;
    }
#else
    if (LORAMAC_STATUS_OK != LORAMAC_VAT_SetClassMode((DeviceClass_t)enClass))
    {
        ucRet = LADAPTER_FAILED;
    }
#endif

    return ucRet;
}

uint8_t LADAPTER_SetADR(bool bAdrON)
{
    uint8_t ucRet = LADAPTER_SUCCES;
    
#if defined( LORA_MODULE_WSL30X )
    char auCmdData[2] = {'0', 0};

    if(true == bAdrON)
    {
        auCmdData[0] = '1';
    }
    if (AT_CMD_SUCCESS != WSL305S_AT_SetATCMD(AT_ADR_SET, auCmdData))
    {
        ucRet = LADAPTER_FAILED;
    }
#else
    if (LORAMAC_STATUS_OK != LORAMAC_VAT_SetADR(bAdrON))
    {
        ucRet = LADAPTER_FAILED;
    }
#endif

    return ucRet;
}

uint8_t LADAPTER_SetDR(LADAPTER_DR_E enDR)
{
    uint8_t ucRet = LADAPTER_SUCCES;
    
#if defined( LORA_MODULE_WSL30X )
    char auCmdData[2] = {0};

    auCmdData[0] = '0';
    auCmdData[0] = auCmdData[0] + enDR;
    if (AT_CMD_SUCCESS != WSL305S_AT_SetATCMD(AT_DR_SET, auCmdData))
    {
        ucRet = LADAPTER_FAILED;
    }
#else
    if (LORAMAC_STATUS_OK != LORAMAC_VAT_SetDR((uint8_t)enDR))
    {
        ucRet = LADAPTER_FAILED;
    }
#endif

    return ucRet;
}

uint8_t LADAPTER_SetRX1Delay(uint8_t ucRx1Delay)
{
    uint8_t ucRet = LADAPTER_SUCCES;

    if ((ucRx1Delay < 1) || (ucRx1Delay > 15))
    {
        return LADAPTER_FAILED;
    }
    
#if defined( LORA_MODULE_WSL30X )
    if (AT_CMD_SUCCESS != WSL305S_AT_SetRX1Delay(ucRx1Delay))
    {
        ucRet = LADAPTER_FAILED;
    }
#else
    if (LORAMAC_STATUS_OK != LORAMAC_VAT_SetRX1Delay(ucRx1Delay))
    {
        ucRet = LADAPTER_FAILED;
    }
#endif

    return ucRet;
}


uint8_t LADAPTER_SetTxPower(LADAPTER_PWRLEVEL_E enPowerLevel)
{
    uint8_t ucRet = LADAPTER_SUCCES;
    
#if defined( LORA_MODULE_WSL30X )
    char auCmdData[2] = {0};

    auCmdData[0] = '0';
    auCmdData[0] = auCmdData[0] + (uint8_t)enPowerLevel;
    if (AT_CMD_SUCCESS != WSL305S_AT_SetATCMD(AT_TXP_SET, auCmdData))
    {
        ucRet = LADAPTER_FAILED;
    }
#else
    if (LORAMAC_STATUS_OK != LORAMAC_VAT_SetTxPower((uint8_t)enPowerLevel))
    {
        ucRet = LADAPTER_FAILED;
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
    uint8_t ucRet = LADAPTER_SUCCES;

    //Only CN470 surpport 
    if ((ucChnlGrp > 12) || (ucChnlGrp == 0))
    {
        return LADAPTER_FAILED;
    }
    
#if defined( LORA_MODULE_WSL30X )
    if ( AT_CMD_SUCCESS != WSL305S_AT_SetChannelGroup(ucChnlGrp))
    {
        ucRet = LADAPTER_FAILED;
    }
#else
    if (LORAMAC_STATUS_OK != LORAMAC_VAT_SetDefaultChannelGroup(ucChnlGrp))
    {
        ucRet = LADAPTER_FAILED;
    }
#endif
    return ucRet;
}

uint8_t LADAPTER_SetWorkChannelGroup(uint8_t ucChnlGrp)
{
    uint8_t ucRet = LADAPTER_SUCCES;
    
    //Only CN470 surpport 
    if ((ucChnlGrp > 12) || (ucChnlGrp == 0))
    {
        return LADAPTER_FAILED;
    }    
#if defined( LORA_MODULE_WSL30X )
    if ( AT_CMD_SUCCESS != WSL305S_AT_SetChannelGroup(ucChnlGrp))
    {
        ucRet = LADAPTER_FAILED;
    }
#else
    if (LORAMAC_STATUS_OK != LORAMAC_VAT_SetWorkChannelGroup(ucChnlGrp))
    {
        ucRet = LADAPTER_FAILED;
    }
#endif
    return ucRet;
}


/*!
 * Send Frame with Port, Data, ConfirmType, Datasize  
 */
uint8_t LADAPTER_Send(LADAPTER_PKTSTYPE_E enConfirmType, uint8_t ucFPort, char *pData, uint8_t ucDataSize)
{
    uint8_t ucRet = LADAPTER_SUCCES;
    bool bConfirm = true;

    if (LADAPTER_PKTSTYPE_CONFIRMED != enConfirmType)
    {
        bConfirm = false;
    }

#if defined( LORA_MODULE_WSL30X )
    if ( AT_CMD_SUCCESS != WSL305S_AT_Send(bConfirm, ucFPort, pData, ucDataSize))
    {
        ucRet = LADAPTER_FAILED;
    }
#else
    if (LORAMAC_STATUS_OK != LORAMAC_VAT_Send(bConfirm, ucFPort, pData, ucDataSize))
    {
        ucRet = LADAPTER_FAILED;
    }
#endif

    return ucRet;
}

void LADAPTER_FRMPktRegister(uint8_t ucFPort, LADAPTER_ProcessFRMPkt_PF pfHandler)
{
    /* FPort=0 is MAC command */
    if (ucFPort > 0)
    {
        g_apfFRMPayloadHandler[ucFPort] = pfHandler;
    }
    
    return;
}


/*!
 * Start NetConfig  
 */
uint8_t LADAPTER_Join(void)
{
    uint8_t ucRet = LADAPTER_SUCCES;
    
#if defined( LORA_MODULE_WSL30X )
    if (AT_CMD_SUCCESS != WSL305S_AT_Join())
    {
        ucRet = LADAPTER_FAILED;
    }
#else
    if (LORAMAC_STATUS_OK != LORAMAC_VAT_Join())
    {
        ucRet = LADAPTER_FAILED;
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


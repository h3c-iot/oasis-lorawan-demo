/**
* @file        loramac_vat.c
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

#include "LoRaMac.h"

#ifndef ACTIVE_REGION

#warning "No active region defined, LORAMAC_REGION_CN470 will be used as default."

#define ACTIVE_REGION LORAMAC_REGION_CN470

#endif

/**　静态全局Mac层回调函数保存的结构体 */
static LoRaMacCallback_t g_stMacCallbacks = {NULL};

/**　静态全局通知上层应用处理Mac事件回调函数保存的结构体 */
static LoRaMacPrimitives_t g_stMacPrimitives = {NULL};

/**　Mcp发送速率全局变量 */
static uint8_t g_ucDR = DR_0;


bool LORAMAC_VAT_IsNetworkJoined(void)
{
    MibRequestConfirm_t stMibReq;
    LoRaMacStatus_t enStatus = LORAMAC_STATUS_OK;
    bool bJoined = false;

    stMibReq.Type = MIB_NETWORK_ACTIVATION;
    enStatus = LoRaMacMibGetRequestConfirm( &stMibReq );
    if( (enStatus == LORAMAC_STATUS_OK) &&
        ( stMibReq.Param.NetworkActivation != ACTIVATION_TYPE_NONE ) )
    {
        bJoined = true;
    }

    return bJoined;
}

LoRaMacStatus_t LORAMAC_VAT_SetABPVersion(uint32_t uiABPVersion)
{
    MibRequestConfirm_t stMibReq;
    LoRaMacStatus_t enStatus = LORAMAC_STATUS_OK;
    
    stMibReq.Type = MIB_ABP_LORAWAN_VERSION;
    stMibReq.Param.AbpLrWanVersion.Value = uiABPVersion;
    enStatus = LoRaMacMibSetRequestConfirm( &stMibReq );

    return enStatus;
}


LoRaMacStatus_t LORAMAC_VAT_SetNetwork(ActivationType_t enActionType)
{
    MibRequestConfirm_t stMibReq;
    LoRaMacStatus_t enStatus = LORAMAC_STATUS_OK;

    stMibReq.Type = MIB_NETWORK_ACTIVATION;
    stMibReq.Param.NetworkActivation = enActionType;
    enStatus = LoRaMacMibSetRequestConfirm( &stMibReq );

    return enStatus;
}

LoRaMacStatus_t LORAMAC_VAT_SetNetID(uint32_t uiNetID)
{
    MibRequestConfirm_t stMibReq;
    LoRaMacStatus_t enStatus = LORAMAC_STATUS_OK;
    
    stMibReq.Type = MIB_NET_ID;
    stMibReq.Param.NetID = uiNetID;
    enStatus = LoRaMacMibSetRequestConfirm( &stMibReq );

    return enStatus;
}

LoRaMacStatus_t LORAMAC_VAT_SetDevAddr(uint32_t uiDevAddr)
{
    MibRequestConfirm_t stMibReq;
    LoRaMacStatus_t enStatus = LORAMAC_STATUS_OK;
    
    stMibReq.Type = MIB_DEV_ADDR;
    stMibReq.Param.DevAddr = uiDevAddr;
    enStatus = LoRaMacMibSetRequestConfirm( &stMibReq );

    return enStatus;
}

LoRaMacStatus_t LORAMAC_VAT_SetAPPKey(uint8_t *pucAppKey)
{
    MibRequestConfirm_t stMibReq;
    LoRaMacStatus_t enStatus = LORAMAC_STATUS_OK;
    
#if( ABP_ACTIVATION_LRWAN_VERSION == ABP_ACTIVATION_LRWAN_VERSION_V10x )
    stMibReq.Type = MIB_GEN_APP_KEY;
    stMibReq.Param.GenAppKey = pucAppKey;
#else
    stMibReq.Type = MIB_APP_KEY;
    stMibReq.Param.AppKey = pucAppKey;
#endif
    enStatus = LoRaMacMibSetRequestConfirm( &stMibReq );

    return enStatus;
}

LoRaMacStatus_t LORAMAC_VAT_SetNWKKey(uint8_t *pucNwkKey)
{
    MibRequestConfirm_t stMibReq;
    LoRaMacStatus_t enStatus = LORAMAC_STATUS_OK;
    
    stMibReq.Type = MIB_NWK_KEY;
    stMibReq.Param.NwkKey = pucNwkKey;
    enStatus = LoRaMacMibSetRequestConfirm( &stMibReq );

    return enStatus;
}

LoRaMacStatus_t LORAMAC_VAT_SetFNWKKey(uint8_t *pucFNwkKey)
{
    MibRequestConfirm_t stMibReq;
    LoRaMacStatus_t enStatus = LORAMAC_STATUS_OK;
    
    stMibReq.Type = MIB_F_NWK_S_INT_KEY;
    stMibReq.Param.FNwkSIntKey = pucFNwkKey;
    enStatus = LoRaMacMibSetRequestConfirm( &stMibReq );

    return enStatus;
}

LoRaMacStatus_t LORAMAC_VAT_SetSNWKKey(uint8_t *pucSNwkKey)
{
    MibRequestConfirm_t stMibReq;
    LoRaMacStatus_t enStatus = LORAMAC_STATUS_OK;
    
    stMibReq.Type = MIB_S_NWK_S_INT_KEY;
    stMibReq.Param.SNwkSIntKey = pucSNwkKey;
    enStatus = LoRaMacMibSetRequestConfirm( &stMibReq );

    return enStatus;
}

LoRaMacStatus_t LORAMAC_VAT_SetNWKSKey(uint8_t *pucNwkSKey)
{
    MibRequestConfirm_t stMibReq;
    LoRaMacStatus_t enStatus = LORAMAC_STATUS_OK;
    
    stMibReq.Type = MIB_NWK_S_ENC_KEY;
    stMibReq.Param.NwkSEncKey = pucNwkSKey;
    enStatus = LoRaMacMibSetRequestConfirm( &stMibReq );

    return enStatus;
}

LoRaMacStatus_t LORAMAC_VAT_SetAPPSKey(uint8_t *pucAppSKey)
{
    MibRequestConfirm_t stMibReq;
    LoRaMacStatus_t enStatus = LORAMAC_STATUS_OK;
    
    stMibReq.Type = MIB_APP_S_KEY;
    stMibReq.Param.AppSKey = pucAppSKey;
    enStatus = LoRaMacMibSetRequestConfirm( &stMibReq );

    return enStatus;
}

LoRaMacStatus_t LORAMAC_VAT_SetDevEUI(uint8_t *pucDevEUI)
{
    MibRequestConfirm_t stMibReq;
    LoRaMacStatus_t enStatus = LORAMAC_STATUS_OK;
    
    stMibReq.Type = MIB_DEV_EUI;
    stMibReq.Param.DevEui = pucDevEUI;
    enStatus = LoRaMacMibSetRequestConfirm( &stMibReq );

    return enStatus;
}

LoRaMacStatus_t LORAMAC_VAT_SetJoinEUI(uint8_t *pucJoinEUI)
{
    MibRequestConfirm_t stMibReq;
    LoRaMacStatus_t enStatus = LORAMAC_STATUS_OK;
    
    stMibReq.Type = MIB_JOIN_EUI;
    stMibReq.Param.JoinEui = pucJoinEUI;
    enStatus = LoRaMacMibSetRequestConfirm( &stMibReq );

    return enStatus;
}

LoRaMacStatus_t LORAMAC_VAT_SetClassMode(DeviceClass_t enClass)
{
    MibRequestConfirm_t stMibReq;
    LoRaMacStatus_t enStatus = LORAMAC_STATUS_OK;
    
    stMibReq.Type = MIB_DEVICE_CLASS;
    stMibReq.Param.Class = enClass;
    enStatus = LoRaMacMibSetRequestConfirm( &stMibReq );

    return enStatus;
}

LoRaMacStatus_t LORAMAC_VAT_SetADR(bool bADRON)
{
    MibRequestConfirm_t stMibReq;
    LoRaMacStatus_t enStatus = LORAMAC_STATUS_OK;
    
    stMibReq.Type = MIB_ADR;
    stMibReq.Param.AdrEnable = bADRON;
    enStatus = LoRaMacMibSetRequestConfirm( &stMibReq );

    return enStatus;
}

LoRaMacStatus_t LORAMAC_VAT_SetDR(uint8_t ucDR)
{
    MibRequestConfirm_t stMibReq;
    LoRaMacStatus_t enStatus = LORAMAC_STATUS_OK;

    g_ucDR = ucDR;
    stMibReq.Type = MIB_CHANNELS_DATARATE;
    stMibReq.Param.ChannelsDatarate = ucDR;
    enStatus = LoRaMacMibSetRequestConfirm( &stMibReq );

    return enStatus;
}

LoRaMacStatus_t LORAMAC_VAT_SetRX1Delay(uint8_t ucRx1Delay)
{
    MibRequestConfirm_t stMibReq;
    LoRaMacStatus_t enStatus = LORAMAC_STATUS_OK;
    
    stMibReq.Type = MIB_RECEIVE_DELAY_1;
    stMibReq.Param.ReceiveDelay1 = ucRx1Delay;
    enStatus = LoRaMacMibSetRequestConfirm( &stMibReq );

    return enStatus;
}

LoRaMacStatus_t LORAMAC_VAT_SetTxPower(uint8_t ucTxPower)
{
    MibRequestConfirm_t stMibReq;
    LoRaMacStatus_t enStatus = LORAMAC_STATUS_OK;
    
    stMibReq.Type = MIB_CHANNELS_DEFAULT_TX_POWER;
    stMibReq.Param.ChannelsDefaultTxPower = ucTxPower;
    enStatus = LoRaMacMibSetRequestConfirm( &stMibReq );

    return enStatus;
}

LoRaMacStatus_t LORAMAC_VAT_SetDefaultChannelGroup(uint8_t ucChnlGrp)
{
    MibRequestConfirm_t stMibReq;
    uint16_t ausChannelsMask[6] = {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000};
    LoRaMacStatus_t enStatus = LORAMAC_STATUS_OK;
    
    ausChannelsMask[(ucChnlGrp - 1) >> 1] = 0xFF << (((ucChnlGrp - 1) % 2) * 8);
    stMibReq.Type = MIB_CHANNELS_DEFAULT_MASK;
    stMibReq.Param.ChannelsMask = ausChannelsMask;
    enStatus = LoRaMacMibSetRequestConfirm( &stMibReq );
    
    return enStatus;
}

LoRaMacStatus_t LORAMAC_VAT_SetWorkChannelGroup(uint8_t ucChnlGrp)
{
    MibRequestConfirm_t stMibReq;
    uint16_t ausChannelsMask[6] = {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000};
    LoRaMacStatus_t enStatus = LORAMAC_STATUS_OK;

    /* get first */
    stMibReq.Type = MIB_CHANNELS_MASK;
    enStatus = LoRaMacMibGetRequestConfirm( &stMibReq );
    
    ausChannelsMask[(ucChnlGrp - 1) >> 1] = 0xFF << (((ucChnlGrp - 1) % 2) * 8);
    ausChannelsMask[(ucChnlGrp - 1) >> 1] &= stMibReq.Param.ChannelsMask[(ucChnlGrp - 1) >> 1];

    stMibReq.Param.ChannelsMask = ausChannelsMask;
    enStatus = LoRaMacMibSetRequestConfirm( &stMibReq );
    
    return enStatus;
}

LoRaMacStatus_t LORAMAC_VAT_Send(bool bConfirm, uint8_t ucFPort, 
                                char *pData, uint8_t ucDataSize)
{
    LoRaMacStatus_t enStatus = LORAMAC_STATUS_OK;
    McpsReq_t stMcpsReq;
    LoRaMacTxInfo_t stTxInfo;

    enStatus = LoRaMacQueryTxPossible(ucDataSize,&stTxInfo);
    if (LORAMAC_STATUS_OK == enStatus)
    {
        if (bConfirm == true)
        {
            stMcpsReq.Type = MCPS_CONFIRMED;
            stMcpsReq.Req.Confirmed.fPort = ucFPort;
            stMcpsReq.Req.Confirmed.Datarate = g_ucDR;
            stMcpsReq.Req.Confirmed.fBuffer = pData;
            stMcpsReq.Req.Confirmed.NbTrials = 3;
            stMcpsReq.Req.Confirmed.fBufferSize = ucDataSize;
        }
        else
        {
            stMcpsReq.Type = MCPS_UNCONFIRMED;
            stMcpsReq.Req.Unconfirmed.fPort = ucFPort;
            stMcpsReq.Req.Unconfirmed.Datarate = g_ucDR;
            stMcpsReq.Req.Unconfirmed.fBuffer = pData;
            stMcpsReq.Req.Unconfirmed.fBufferSize = ucDataSize;
        }

        enStatus = LoRaMacMcpsRequest( &stMcpsReq );
    }

    return enStatus;
}

LoRaMacStatus_t LORAMAC_VAT_Join( void )
{
    LoRaMacStatus_t enStatus = LORAMAC_STATUS_OK;
    MlmeReq_t stMlmeReq;
    
    stMlmeReq.Type = MLME_JOIN;
    stMlmeReq.Req.Join.Datarate = DR_1;

    // Starts the join procedure
    enStatus = LoRaMacMlmeRequest( &stMlmeReq );

    return enStatus;
}

void LORAMAC_VAT_Init(LoRaMacCallback_t *pstMacCallbacks, LoRaMacPrimitives_t *pstMacPrimitives)
{
    MibRequestConfirm_t stMibReq;
    
    g_stMacPrimitives.MacMcpsConfirm = pstMacPrimitives->MacMcpsConfirm;
    g_stMacPrimitives.MacMcpsIndication = pstMacPrimitives->MacMcpsIndication;
    g_stMacPrimitives.MacMlmeConfirm = pstMacPrimitives->MacMlmeConfirm;
    g_stMacPrimitives.MacMlmeIndication = pstMacPrimitives->MacMlmeIndication;
    g_stMacCallbacks.GetBatteryLevel = pstMacCallbacks->GetBatteryLevel;
    g_stMacCallbacks.GetTemperatureLevel = pstMacCallbacks->GetTemperatureLevel;
    g_stMacCallbacks.NvmContextChange = pstMacCallbacks->NvmContextChange;
    g_stMacCallbacks.MacProcessNotify = pstMacCallbacks->MacProcessNotify;
    
    LoRaMacInitialization( &g_stMacPrimitives, &g_stMacCallbacks, ACTIVE_REGION );

    stMibReq.Type = MIB_PUBLIC_NETWORK;
    stMibReq.Param.EnablePublicNetwork = true;
    LoRaMacMibSetRequestConfirm( &stMibReq );

    stMibReq.Type = MIB_SYSTEM_MAX_RX_ERROR;
    stMibReq.Param.SystemMaxRxError = 20;
    LoRaMacMibSetRequestConfirm( &stMibReq );

    LoRaMacStart( );
    
    return;
}



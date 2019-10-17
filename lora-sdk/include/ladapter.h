/*******************************************************************************
*                                                                              *
*                Copyright (C) 2019, New H3C Technologies Co., Ltd.            *
*  License: Revised BSD License, see LICENSE.TXT file included in the project  *
*                                                                              *
*------------------------------------------------------------------------------*
*                                                                              *
*                              ladapter.h                                      *
*                                                                              *
*  Project Code: oasis-sdk                                                     *
*   Module Name: lora adapter                                                  *
*  Date Created: 2019-10-27                                                    *
*        Author: wangzhen                                                      *
*   Description: adapt to SPI or Serial Command                                *
*                                                                              *
*------------------------------------------------------------------------------*
*  Modification History                                                        *
*  DATE        NAME             DESCRIPTION                                    *
*  ----------------------------------------------------------------------------*
*  YYYY-MM-DD                                                                  *
*                                                                              *
*******************************************************************************/

#ifndef __LADAPTER_H__
#define __LADAPTER_H__

#include <stdbool.h>


#define    LADAPTER_SUCCES  0
#define    LADAPTER_FAILED  1

typedef enum tagLAdapterNetworkType
{
    LADAPTER_NWKTYPE_OTAA,
    LADAPTER_NWKTYPE_ABP,
    LADAPTER_NWKTYPE_MAX
}LADAPTER_NWKTYPE_E;

typedef enum tagLAdapterPktsType
{
    LADAPTER_PKTSTYPE_UNCONFIRMED,
    LADAPTER_PKTSTYPE_CONFIRMED,
}LADAPTER_PKTSTYPE_E;

typedef enum tagLAdapterDR
{
    LADAPTER_DR_0,
    LADAPTER_DR_1,
    LADAPTER_DR_2,
    LADAPTER_DR_3,
    LADAPTER_DR_4,
    LADAPTER_DR_5,
    LADAPTER_DR_6,
    LADAPTER_DR_7,
}LADAPTER_DR_E;

typedef enum tagLAdapterPowerLevel
{
    LADAPTER_PWRLEVEL_0, /* CN470: 17dbm */
    LADAPTER_PWRLEVEL_1,
    LADAPTER_PWRLEVEL_2,
    LADAPTER_PWRLEVEL_3,
    LADAPTER_PWRLEVEL_4,
    LADAPTER_PWRLEVEL_5,
    LADAPTER_PWRLEVEL_6,
    LADAPTER_PWRLEVEL_7,
}LADAPTER_PWRLEVEL_E;


typedef enum tagLAdaterWorkMode
{
    LADAPTER_CLASS_A,
    LADAPTER_CLASS_B,  
    LADAPTER_CLASS_C,
}LADAPTER_CLASS_E;


typedef void ( *LADAPTER_ProcessFRMPkt_PF )( char *pcData, uint8_t ucDataLen, bool bFPending );

typedef struct tagLAdapterHandler
{
    void ( *pfLAdapter_JoinServer )(uint8_t ucJoinResult);
    float ( *pfLAdapter_GetTemperatureLevel)(void);
    uint8_t ( *pfLAdapter_BoardGetBatteryLevel)( void );
    void ( *pfLAdapter_OnMacProcessNotify)( void );
}LADAPTER_HANDLER_S;


void LADAPTER_Init(LADAPTER_HANDLER_S *pHander);
uint8_t LADAPTER_Join(void);
void LADAPTER_Running(void);

void LADAPTER_FRMPktRegister(uint8_t ucFPort, LADAPTER_ProcessFRMPkt_PF pfHandler);

bool LADAPTER_IsNetworkJoined(void);
uint8_t LADAPTER_Send(LADAPTER_PKTSTYPE_E enConfirmType, uint8_t ucFPort, char * pData, uint8_t ucDataSize);
uint8_t LADAPTER_SetJoinChannelGroup(uint8_t ucChnlGrp);
uint8_t LADAPTER_SetWorkChannelGroup(uint8_t ucChnlGrp);
uint8_t LADAPTER_SetNetwork(LADAPTER_NWKTYPE_E enNetWork);
uint8_t LADAPTER_SetTxPower(LADAPTER_PWRLEVEL_E enPowerLevel);
uint8_t LADAPTER_SetDR(LADAPTER_DR_E enDR);
uint8_t LADAPTER_SetADR(bool bAdrON);
uint8_t LADAPTER_SetRX1Delay(uint8_t ucRx1Delay);
uint8_t LADAPTER_SetClassMode(LADAPTER_CLASS_E enClass);
uint8_t LADAPTER_SetDevAddr(uint32_t uiDevAddr);
uint8_t LADAPTER_SetABPVersion(uint32_t uiVersion);
uint8_t LADAPTER_SetAPPKey(uint8_t *pucAppKey);
uint8_t LADAPTER_SetNWKKey(uint8_t *pucNwkKey);
uint8_t LADAPTER_SetFNWKKey(uint8_t *pucFNwkKey);
uint8_t LADAPTER_SetSNWKKey(uint8_t *pucSNwkKey);
uint8_t LADAPTER_SetNWKSKey(uint8_t *pucNwkSKey);
uint8_t LADAPTER_SetAPPSKey(uint8_t *pucAppSKey);
uint8_t LADAPTER_SetJoinEUI(uint8_t *pucJoinEUI);
uint8_t LADAPTER_SetDevEUI(uint8_t *pucDevEUI);
uint8_t LADAPTER_SetNetID(uint32_t uiNetID);


#endif /*__LADAPTER_H__*/


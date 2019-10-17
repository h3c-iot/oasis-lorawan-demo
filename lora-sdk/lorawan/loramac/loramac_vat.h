/*******************************************************************************
*                                                                              *
*                Copyright (C) 2019, New H3C Technologies Co., Ltd.            *
*  License: Revised BSD License, see LICENSE.TXT file included in the project  *
*                                                                              *
*------------------------------------------------------------------------------*
*                                                                              *
*                              loramac_vat.h                                      *
*                                                                              *
*  Project Code: oasis-sdk                                                     *
*   Module Name: lora adapt                                                    *
*  Date Created: 2019-10-26                                                    *
*        Author: wangzhen                                                       *
*   Description:                                                                    *
*                                                                              *
*------------------------------------------------------------------------------*
*  Modification History                                                        *
*  DATE        NAME             DESCRIPTION                                    *
*  ----------------------------------------------------------------------------*
*  YYYY-MM-DD                                                                  *
*                                                                              *
*******************************************************************************/

#ifndef __LORAMAC_VAT_H__
#define __LORAMAC_VAT_H__

bool LORAMAC_VAT_IsNetworkJoined(void);
LoRaMacStatus_t LORAMAC_VAT_SetABPVersion(uint32_t uiABPVersion);

LoRaMacStatus_t LORAMAC_VAT_SetNetwork(ActivationType_t enActionType);
LoRaMacStatus_t LORAMAC_VAT_SetNetID(uint32_t uiNetID);
LoRaMacStatus_t LORAMAC_VAT_SetDevAddr(uint32_t uiDevAddr);
LoRaMacStatus_t LORAMAC_VAT_SetAPPKey(uint8_t *pucAppKey);
LoRaMacStatus_t LORAMAC_VAT_SetNWKKey(uint8_t *pucNwkKey);
LoRaMacStatus_t LORAMAC_VAT_SetFNWKKey(uint8_t *pucFNwkKey);
LoRaMacStatus_t LORAMAC_VAT_SetSNWKKey(uint8_t *pucSNwkKey);
LoRaMacStatus_t LORAMAC_VAT_SetNWKSKey(uint8_t *pucNwkSKey);
LoRaMacStatus_t LORAMAC_VAT_SetAPPSKey(uint8_t *pucAppSKey);
LoRaMacStatus_t LORAMAC_VAT_SetDevEUI(uint8_t *pucDevEUI);
LoRaMacStatus_t LORAMAC_VAT_SetJoinEUI(uint8_t *pucJoinEUI);
LoRaMacStatus_t LORAMAC_VAT_SetClassMode(DeviceClass_t enClass);
LoRaMacStatus_t LORAMAC_VAT_SetADR(bool bADRON);
LoRaMacStatus_t LORAMAC_VAT_SetDR(uint8_t ucDR);
LoRaMacStatus_t LORAMAC_VAT_SetRX1Delay(uint8_t ucRx1Delay);
LoRaMacStatus_t LORAMAC_VAT_SetTxPower(uint8_t ucTxPower);
LoRaMacStatus_t LORAMAC_VAT_SetDefaultChannelGroup(uint8_t ucChnlGrp);
LoRaMacStatus_t LORAMAC_VAT_SetWorkChannelGroup(uint8_t ucChnlGrp);
LoRaMacStatus_t LORAMAC_VAT_Send(bool bConfirm, uint8_t ucFPort,
                                         char *pData, uint8_t ucDataSize);

LoRaMacStatus_t LORAMAC_VAT_Join( void );
void LORAMAC_VAT_Init(LoRaMacCallback_t *pstMacCallbacks, 
                             LoRaMacPrimitives_t *pstMacPrimitives);




#endif /*__LORAMAC_VAT_H__*/



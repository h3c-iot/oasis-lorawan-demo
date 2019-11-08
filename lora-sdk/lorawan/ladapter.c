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
#if !defined( LORA_MODULE_WSL30X )
#include "LoRaMac.h"
#include "loramac_vat.h"
#include "NvmCtxMgmt.h"
#else
#include "wsl305s.h"
#endif
#include "Commissioning.h"

static uint8_t LadapterSendPkt(bool bConfirm, uint8_t ucFPort, char *pcData, uint8_t ucDataSize);
static void LadapterProcMcpsConfirm( uint8_t ucRet );

/**
 *发送报文最大长度
 */
#define LADAPTER_MAX_DATA_LEN      255

/**
 * 报文信息存储结构体
 */
typedef struct tagLadapterPKTNode
{
    uint8_t ucFPort;
    bool bConfirm;
    uint8_t ucLen;
    char aucData[LADAPTER_MAX_DATA_LEN];
}LADAPTER_PKTNODE_S;

/**
 * 报文进出Chace状态结构体
 */
typedef struct tagLadapterPKTCache
{
    /**
     * 当前进Cache的报文索引
     */
    uint8_t ucIn;
    
    /**
     * 当前出Cache的报文索引
     */
    uint8_t ucOut;
    /**
     * 当前待出Cache的报文数
     */    
    uint8_t ucCnt;
    /**
     * Cache允许存储的最大报文数
     */      
    uint8_t ucMaxCacheCnt;
    /**
     * 报文是否处于发送状态
     */
    bool bSending;
    /**
     * 报文是否处于发送状态
     */
    LADAPTER_PKTNODE_S *pstDataList;
}LADAPTER_PKTCACHE_S;

/** 全局结构体变量,用于存放上层应用待发送的报文*/
static LADAPTER_PKTCACHE_S g_stLadapterPktCache = {0};

/**
 * @brief   获取当前Cache内存中是否有待发送的报文
 *
 * @return  返回存放于Cache待发送报文信息的结构体指针 
 *          ,如果返回NULL 表示Cache中没有需要发送的报文
 */
static inline bool LadapterIsCacheEmpty(void)
{
    return (g_stLadapterPktCache.ucCnt == 0);
}

/**
 * @brief   获取当前Cache内存中是否堆满了待发送的报文
 *
 * @retval  返回true 表示Cache内存中堆满了待发送的报文 
 *
 * @retval  返回false 表示Cache内存中没有堆满待发送的报文
 */
static inline bool LadapterIsCacheFull(void)
{
    return (g_stLadapterPktCache.ucMaxCacheCnt == g_stLadapterPktCache.ucCnt);
}

/**
 * @brief   往Cache中添加一组待发送的报文
 *
 * @return  返回添加完Cache待发送报文信息的结构体指针 
 *          ,如果返回NULL 表示Cache中已经堆满了待发送的报文
 */
static LADAPTER_PKTNODE_S *LadapterAddPktNode(bool bConfirm, uint8_t ucFPort, 
                                   char *pcData, uint8_t ucDataSize)
{
    LADAPTER_PKTNODE_S *pstNode = NULL;
    
    if (true == LadapterIsCacheFull())
    {
        /* buffer is full */
        return NULL;
    }

    pstNode = g_stLadapterPktCache.pstDataList + g_stLadapterPktCache.ucIn;
    pstNode->bConfirm = bConfirm;
    pstNode->ucLen = ucDataSize;
    pstNode->ucFPort = ucFPort;
    memcpy(pstNode->aucData, pcData, ucDataSize);

    g_stLadapterPktCache.ucIn = (g_stLadapterPktCache.ucIn + 1) % g_stLadapterPktCache.ucMaxCacheCnt;
    g_stLadapterPktCache.ucCnt++;

    return pstNode;
}
                                   
/**
 * @brief   获取当前Cache中待发送的报文
 *
 * @return  返回存放于Cache待发送报文信息的结构体指针 
 *          ,如果返回NULL 表示Cache中没有需要发送的报文
 */
static LADAPTER_PKTNODE_S *LadapterGetSndNode(void)
{
    LADAPTER_PKTNODE_S *pstPktNode = NULL;
    
    if (false == LadapterIsCacheEmpty())
    {
        pstPktNode = g_stLadapterPktCache.pstDataList + g_stLadapterPktCache.ucOut;
    }
    
    return pstPktNode;
}

/**
 * @brief   清除Cache当前正在发送的报文
 */
static void LadapterDelSndNode(void)
{
    if (false == LadapterIsCacheEmpty())
    {
        memset(g_stLadapterPktCache.pstDataList + g_stLadapterPktCache.ucOut, 0, sizeof(LADAPTER_PKTNODE_S));
        g_stLadapterPktCache.ucOut = (g_stLadapterPktCache.ucOut + 1) % g_stLadapterPktCache.ucMaxCacheCnt;
        g_stLadapterPktCache.ucCnt--;
    }

    return;
}

/**
 * @brief   发送Cache中存放的报文
 */
static void LadapterTriggerSend(void)
{
    LADAPTER_PKTNODE_S *pstNode = NULL;

    if ( (false == LadapterIsCacheEmpty()) &&
          (g_stLadapterPktCache.bSending == false) )
    {
        pstNode = g_stLadapterPktCache.pstDataList + g_stLadapterPktCache.ucOut;
        if (LADAPTER_SUCCES == LadapterSendPkt(pstNode->bConfirm, pstNode->ucFPort, 
                                            pstNode->aucData, pstNode->ucLen))
        {
            g_stLadapterPktCache.bSending = true;
        }
    }
    
    return;
}

/**
 * @brief   Cache初始化
 *
 * @param   [输入] unsigned char类型的变量,
 *                 初始化Cache的大小.
 */
static uint8_t LadapterCacheInit(uint8_t ucCacheCnt)
{
    LADAPTER_PKTNODE_S *pstPktNodeList = NULL;
    uint8_t ucCnt = 0;
    uint8_t ucRet = LADAPTER_FAILED;
    
    g_stLadapterPktCache.bSending = false;
    g_stLadapterPktCache.ucIn = 0;
    g_stLadapterPktCache.ucOut = 0;
    g_stLadapterPktCache.ucCnt = 0;

    ucCnt = (ucCacheCnt == 0) ? 1 : ucCacheCnt;
    pstPktNodeList = (LADAPTER_PKTNODE_S *)malloc(sizeof(LADAPTER_PKTNODE_S) * ucCnt);
    if (pstPktNodeList != NULL)
    {
        g_stLadapterPktCache.pstDataList = pstPktNodeList;
        g_stLadapterPktCache.ucMaxCacheCnt = ucCnt;
        
        ucRet = LADAPTER_SUCCES;
    }
    
    return ucRet;
}

/*
static void LadapterCacheFini(void)
{
    uint8_t ucCnt = g_stLadapterPktCache.ucCnt;
    while (ucCnt > 0)
    {
        LadapterProcMcpsConfirm(LADAPTER_FAILED);
        ucCnt--;
    }
    
    if (g_stLadapterPktCache.pstDataList != NULL)
    {
        free(g_stLadapterPktCache.pstDataList);
        g_stLadapterPktCache.pstDataList = NULL;
    }
    g_stLadapterPktCache.bSending = false;
    g_stLadapterPktCache.ucIn = 0;
    g_stLadapterPktCache.ucOut = 0;
    g_stLadapterPktCache.ucCnt = 0;
    g_stLadapterPktCache.ucMaxCacheCnt = 0;
    
    return;
}
*/

/** 下行数据回调处理函数指针结构体*/
LADAPTER_PKTHANDLER_S *g_apfFRMPayloadHandler[255] = {NULL};

/** Mac处理回调函数注册结构体*/
LADAPTER_HANDLER_S g_stLoraAdpHandler = {NULL,};

#if !defined( LORA_MODULE_WSL30X )

/**
 * @brief   MCPS-Confirm类型的函数事件
 *
 * @param   [输入] McpsConfirm_t * 类型的指针
 *                 ,包含Mcps类型的确认信息.
 */
static void McpsConfirm( McpsConfirm_t *pstMcpsConfirm )
{
    uint8_t ucRet = LADAPTER_SUCCES;
    
    if( pstMcpsConfirm->Status != LORAMAC_EVENT_INFO_STATUS_OK )
    {
        ucRet = LADAPTER_FAILED;
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
        (g_apfFRMPayloadHandler[pstMcpsIndication->Port] != NULL))
    {
        g_apfFRMPayloadHandler[pstMcpsIndication->Port]->pfLAdapter_ProcFRMPktReceive(
                                       (char *)pstMcpsIndication->Buffer,pstMcpsIndication->BufferSize);
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
    uint8_t ucRet = LADAPTER_SUCCES;
    
    if( ucResult != AT_CMD_SUCCESS )
    {
        ucRet = LADAPTER_FAILED;
    }
    LadapterProcMcpsConfirm(ucRet);
    
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
    if( (ucFPort > 0) &&
        (g_apfFRMPayloadHandler[ucFPort] != NULL))
    {
        g_apfFRMPayloadHandler[ucFPort]->pfLAdapter_ProcFRMPktReceive(pcData, ucRecvLen);
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
 * @brief   Mlme类型应答报文处理回调
 *
 * @param   [输入] bool 类型的参数,bJoined = true表示已经入网  
 *                  ,bJoined = false 表示还未入网              
 */
static uint8_t LadapterSendPkt(bool bConfirm, uint8_t ucFPort, char *pcData, uint8_t ucDataSize)
{
    uint8_t ucRet = LADAPTER_SUCCES;

#if defined( LORA_MODULE_WSL30X )
    if ( AT_CMD_SUCCESS != WSL305S_AT_Send(bConfirm, ucFPort, pcData, ucDataSize))
    {
        ucRet = LADAPTER_FAILED;
    }
#else
    if (LORAMAC_STATUS_OK != LORAMAC_VAT_Send(bConfirm, ucFPort, pcData, ucDataSize))
    {
        ucRet = LADAPTER_FAILED;
    }
#endif

    return ucRet;
}

/**
 * @brief   适配层报文应答处理函数
 *
 * @param   [输入] unsigned char 类型的参数,表示应答处理结果.
 *             
 */
static void LadapterProcMcpsConfirm( uint8_t ucRet )
{
    LADAPTER_PKTNODE_S *pstPktNode = NULL;

    pstPktNode = LadapterGetSndNode();
    if ((pstPktNode != NULL) && g_apfFRMPayloadHandler[pstPktNode->ucFPort] != NULL)
    {
        g_apfFRMPayloadHandler[pstPktNode->ucFPort]->pfLAdapter_ProcMcpsConfirm(ucRet);
        LadapterDelSndNode();
    }

    g_stLadapterPktCache.bSending = false;
    
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

void *LADAPTER_Send(LADAPTER_PKTSTYPE_E enConfirmType, uint8_t ucFPort, char *pData, uint8_t ucDataSize)
{
    bool bConfirm = true;
    LADAPTER_PKTNODE_S *pstPktNode = NULL;

    if (LADAPTER_PKTSTYPE_CONFIRMED != enConfirmType)
    {
        bConfirm = false;
    }

    pstPktNode = LadapterAddPktNode(bConfirm, ucFPort, pData, ucDataSize);

    LadapterTriggerSend();

    return pstPktNode;
}

uint8_t LADAPTER_RegisterFRMPktProc(uint8_t ucFPort, LADAPTER_PKTHANDLER_S *pstPKtHandler)
{
    LADAPTER_PKTHANDLER_S *pstHandler = NULL;
    uint8_t ucRet = LADAPTER_FAILED;
    
    /* FPort=0 is MAC command */
    if (ucFPort > 0)
    {
        pstHandler = (LADAPTER_PKTHANDLER_S *)malloc(sizeof(LADAPTER_PKTHANDLER_S));
        if (pstHandler != NULL)
        {
            pstHandler->pfLAdapter_ProcFRMPktReceive = pstPKtHandler->pfLAdapter_ProcFRMPktReceive;
            pstHandler->pfLAdapter_ProcMcpsConfirm = pstPKtHandler->pfLAdapter_ProcMcpsConfirm;
            g_apfFRMPayloadHandler[ucFPort] = pstHandler;
            ucRet = LADAPTER_SUCCES;
        }
    }
    
    return ucRet;
}

void LADAPTER_DeRegisterFRMPktProc(uint8_t ucFPort)
{
    LADAPTER_PKTHANDLER_S *pstHandler = NULL;
    
    /* FPort=0 is MAC command */
    if ((ucFPort > 0) && (g_apfFRMPayloadHandler[ucFPort] != NULL))
    {
        pstHandler = g_apfFRMPayloadHandler[ucFPort];
        free(pstHandler);
        g_apfFRMPayloadHandler[ucFPort] = NULL;
    }
    
    return;
}

void LADAPTER_Running(void)
{
    LadapterTriggerSend();
    
#if defined( LORA_MODULE_WSL30X )
    WSL305S_AT_Running();
#else
    LoRaMacProcess();
#endif
    return;
}

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

void LADAPTER_Init(LADAPTER_HANDLER_S *pstHandler, uint8_t ucCacheCnt)
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
    LadapterCacheInit(ucCacheCnt);

    return;
}



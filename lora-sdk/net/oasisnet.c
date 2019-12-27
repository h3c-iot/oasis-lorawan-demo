/**
* @file        oasis_net.c
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
*                          Module Name: oasis
*
*                          Date Created: 2019-12-05                           
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
#include "debug.h"
#include "utilities.h"
#include "timer.h"
#include "base.h"
#include "oasisnet.h"
#include "sdk-config.h"
#include "ladapter.h"

#define THIS_IS(para)   1


/** 下行数据回调处理函数指针结构体*/
static OASISNET_PKT_HANDLER_S *g_apfFRMPayloadHandler[255] = {NULL};

/**
 * 表示丢包次数
 */
static uint8_t g_ucPktDroped = 0;

/**
 * 表示自适应速率开关
 */
static bool g_bAdrOn = false;

/**
 * 表示发送数据速率
 */
static LADAPTER_DR_E g_enDR = LADAPTER_DR_1;

/**
 * 表示基础发送数据速率
 */
static LADAPTER_DR_E g_enBaseDR = LADAPTER_DR_1;

/**
 * 表示是否入网
 */
static bool g_bJoined = false;

/**
 * 表示是否处于入网处理流程
 */
static bool g_bJoining = false;

/**
 * 表示发射信道组 g_ucGrp = 1:发射频率470.3 ~ 471.7MHZ
 */
static uint8_t g_ucGrp = 1;

/**
 * 表示当前信道组入网的次数
 */
static uint8_t g_ucChannelRetry = 0;

/**
 * 表示已探测的入网信道组数
 */
static uint8_t g_ucGrpRetry = 0;

/**
 * 入网处理函数指针
 */
OASISNET_HANDLER_S g_stOasisNetHandler = {NULL,};

/**
 * 拉去数据定时器
 */
static TimerEvent_t g_stOasisNetPullTimer;


#if THIS_IS("NET_PKT")
/**
 *发送报文最大长度
 */
#define OASIS_NETPKT_MAX_DATA_LEN      255

/**
 * 报文信息存储结构体
 */
typedef struct tagOasisNETPKTNode
{
    /**
     * 端口
     */
    uint8_t ucFPort;
    /**
     * 是否确认
     */
    bool bConfirm;
    
    /**
     * 数据长度
     */
    uint8_t ucLen;

    /**
     * 数据
     */
    char aucData[OASIS_NETPKT_MAX_DATA_LEN];
}OASIS_NETPKT_NODE_S;

/**
 * 报文进出Chace状态结构体
 */
typedef struct tagOasisNETPKTCache
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
    OASIS_NETPKT_NODE_S **ppstDataList;
}OASIS_NETPKT_CACHE_S;

/** 全局结构体变量,用于存放上层应用待发送的报文*/
static OASIS_NETPKT_CACHE_S g_stOasisNetPktCache = {0};

/**
 * @brief   获取当前Cache内存中是否为空
 *
 * @retval  返回true 表示Cache队列中为空
 *
 * @retval  返回false 表示Cache队列不为空
 */
static inline bool OasisNetPkt_IsCacheEmpty(void)
{
    return (g_stOasisNetPktCache.ucCnt == 0);
}

/**
 * @brief   获取当前Cache内存中是否堆满了待发送的报文
 *
 * @retval  返回true 表示Cache队列中堆满了待发送的报文 
 *
 * @retval  返回false 表示Cache队列中没有堆满待发送的报文
 */
static inline bool OasisNetPkt_IsCacheFull(void)
{
    return (g_stOasisNetPktCache.ucMaxCacheCnt == g_stOasisNetPktCache.ucCnt);
}

/**
 * @brief   Cache初始化
 *
 * @param   [输入] unsigned char类型的变量,
 *                 初始化Cache的大小.
 */
static uint8_t OasisNetPkt_InitCache(uint8_t ucCacheCnt)
{
    OASIS_NETPKT_NODE_S **ppstPktNodeList = NULL;
    uint8_t ucCnt = 0;
    uint8_t ucRet = ERROR_FAILED;
    
    g_stOasisNetPktCache.bSending = false;
    g_stOasisNetPktCache.ucIn = 0;
    g_stOasisNetPktCache.ucOut = 0;
    g_stOasisNetPktCache.ucCnt = 0;

    ucCnt = (ucCacheCnt == 0) ? 1 : ucCacheCnt;
    ppstPktNodeList = malloc(sizeof(OASIS_NETPKT_NODE_S *) * ucCnt);
    if (ppstPktNodeList != NULL)
    {
        for (int i=0; i<ucCnt; i++)
        {
            ppstPktNodeList[i] = NULL;
        }
        g_stOasisNetPktCache.ppstDataList = ppstPktNodeList;
        g_stOasisNetPktCache.ucMaxCacheCnt = ucCnt;
        
        ucRet = ERROR_SUCCESS;
    }
    
    return ucRet;
}

/*
static void OasisNetPkt_CacheFini(void)
{
    uint8_t ucCnt = g_stOasisNetPktCache.ucCnt;
    while (ucCnt > 0)
    {
        OasisNet_ProcMcpsConfirm(ERROR_FAILED);
        ucCnt--;
    }
    
    if (g_stOasisNetPktCache.pstDataList != NULL)
    {
        free(g_stOasisNetPktCache.pstDataList);
        g_stOasisNetPktCache.pstDataList = NULL;
    }
    g_stOasisNetPktCache.bSending = false;
    g_stOasisNetPktCache.ucIn = 0;
    g_stOasisNetPktCache.ucOut = 0;
    g_stOasisNetPktCache.ucCnt = 0;
    g_stOasisNetPktCache.ucMaxCacheCnt = 0;
    
    return;
}
*/


/**
 * @brief   往Cache中添加一组待发送的报文
 *
 * @return  返回添加完Cache待发送报文信息的结构体指针 
 *          ,如果返回NULL 表示Cache中已经堆满了待发送的报文
 */
static OASIS_NETPKT_NODE_S *OasisNetPkt_AddPktNode(bool bConfirm, uint8_t ucFPort, 
                                   char *pcData, uint8_t ucDataSize)
{
    OASIS_NETPKT_NODE_S *pstNode = NULL;
    
    if (true == OasisNetPkt_IsCacheFull())
    {
        /* buffer is full */
        return NULL;
    }

    pstNode = (OASIS_NETPKT_NODE_S *)malloc(sizeof(OASIS_NETPKT_NODE_S));
    if (pstNode != NULL)
    {
        pstNode->bConfirm = bConfirm;
        pstNode->ucLen = ucDataSize;
        pstNode->ucFPort = ucFPort;
        memcpy(pstNode->aucData, pcData, ucDataSize);
        g_stOasisNetPktCache.ppstDataList[g_stOasisNetPktCache.ucIn] = pstNode;
        g_stOasisNetPktCache.ucIn = (g_stOasisNetPktCache.ucIn + 1) % g_stOasisNetPktCache.ucMaxCacheCnt;
        g_stOasisNetPktCache.ucCnt++;
    }

    return pstNode;
}
                                   
/**
 * @brief   获取当前Cache中待发送的报文
 *
 * @return  返回存放于Cache待发送报文信息的结构体指针 
 *          ,如果返回NULL 表示Cache中没有需要发送的报文
 */
static OASIS_NETPKT_NODE_S *OasisNetPkt_GetSndNode(void)
{
    OASIS_NETPKT_NODE_S *pstPktNode = NULL;
    
    if (false == OasisNetPkt_IsCacheEmpty())
    {
        pstPktNode = g_stOasisNetPktCache.ppstDataList[g_stOasisNetPktCache.ucOut];
    }

    return pstPktNode;
}

/**
 * @brief   清除Cache当前正在发送的报文
 */
static OASIS_NETPKT_NODE_S *OasisNetPkt_DelSndNode(void)
{
    OASIS_NETPKT_NODE_S *pstPktNode = NULL;
    
    if (false == OasisNetPkt_IsCacheEmpty())
    {
        pstPktNode = g_stOasisNetPktCache.ppstDataList[g_stOasisNetPktCache.ucOut];
        g_stOasisNetPktCache.ppstDataList[g_stOasisNetPktCache.ucOut] = NULL;
        g_stOasisNetPktCache.ucOut = (g_stOasisNetPktCache.ucOut + 1) % g_stOasisNetPktCache.ucMaxCacheCnt;
        g_stOasisNetPktCache.ucCnt--;
    }

    return pstPktNode;
}

/**
 * @brief   发送Cache中存放的报文
 */
static void OasisNetPkt_TriggerSend(void)
{
    OASIS_NETPKT_NODE_S *pstNode = NULL;

    if (g_stOasisNetPktCache.bSending == false)
    {
        pstNode = OasisNetPkt_GetSndNode();
        if (pstNode != NULL)
        {
            DEBUG_PRINT("[Net]Trigger send packet, confrim=%d, fport=%d.\r\n", 
                         pstNode->bConfirm, pstNode->ucFPort);
            if (g_bAdrOn == false)
            {
                /* ADR没有开启，则由应用层决定发包速率*/
                LADAPTER_SetDR(g_enDR);
            }
            if (ERROR_SUCCESS == LADAPTER_Send(pstNode->bConfirm, pstNode->ucFPort, 
                                                pstNode->aucData, pstNode->ucLen))
            {
                g_stOasisNetPktCache.bSending = true;
            }
        }
    }
    
    return;
}
#endif

#if ( OASIS_SDK_INTELIGENCE_DATARATE == 1 )
/**
 * 智能调速开关
 */
static bool g_bIntellengencyAdr = true;

/** 连续收到报文的计数*/
static uint8_t g_ucPktRcvCnt = 0;

/**
 * 信号强度平均值
 *
 */
static int16_t g_sRSSIAvg = 0;

/**
 * 信噪比平均值
 *
 */
static int8_t g_cSNRAvg = 0;

/**
 * @brief   智能调速状态恢复默认
 *
 */
static inline void OasisNet_ClearIntelligentDR(void)
{
    g_ucPktRcvCnt = 0;
    g_sRSSIAvg = 0;
    g_cSNRAvg = 0;

    return;
}

/**
 * @brief   智能调速处理流程
 *
 * @param   [输入] unsigned short类型的信号强度
 *                         
 * @param   [输入] unsigned char类型的信噪比
 */
static void OasisNet_IntelligentDR(uint16_t usRSSI, uint8_t ucSNR)
{
    if (g_bIntellengencyAdr)
    {
        g_ucPktRcvCnt++;
        g_sRSSIAvg = (g_sRSSIAvg == 0) ? usRSSI : ((int16_t)usRSSI + g_sRSSIAvg) / 2;
        g_cSNRAvg = (g_cSNRAvg == 0) ? ucSNR : ((int8_t)ucSNR + g_cSNRAvg) / 2;
        if (g_ucPktRcvCnt >= 3)
        {
            /*连续三次收到报文，则进行动态调速*/
            if ((g_sRSSIAvg >= -90) && (g_cSNRAvg >= 5))
            {
                g_enDR = MAX(LADAPTER_DR_5, g_enBaseDR);
            }
            else if ((g_sRSSIAvg >= -105) && (g_cSNRAvg >= -5))
            {
                g_enDR = MAX(LADAPTER_DR_4, g_enBaseDR);
            }
            else if ((g_sRSSIAvg >= -115) && (g_cSNRAvg >= -12))
            {
                g_enDR = MAX(LADAPTER_DR_3, g_enBaseDR);
            }
            else
            {
                g_enDR = g_enBaseDR;
            }
            
            /*清零，重新开始计数*/
            OasisNet_ClearIntelligentDR();
        }
    }

    return;
}

/**
* @brief   设置智能调速开关.
*
* @param   [输入] bool类型的变量
*          ,true表示打开智能调速
*          ,false表示关闭智能调速
*/
void OASISNET_SetIntelligentDR(bool bOn)
{
    if (bOn == false)
    {
        /* 关闭自动调试，则恢复到基础速率*/
        g_enDR = g_enBaseDR;
        OasisNet_ClearIntelligentDR();
    }
    g_bIntellengencyAdr = bOn;

    return;
}

#endif

/**
* @brief   开始入网流程.
*
* @param   [输入] uinsined char类型的入网结果
*          ,0表示入网成功
*          ,非0表示入网失败
*/
static void OasisNet_JoinChannelSurvey(uint8_t ucJoinResult)
{
    bool bReJoin = false;
    
    if( ucJoinResult == ERROR_SUCCESS)
    {
        DEBUG_PRINT("[Net]Join success, group[%d]\r\n", g_ucGrp);
        LADAPTER_SetWorkChannelGroup(g_ucGrp);
        g_bJoining = false;
        g_bJoined = true;
        if (g_stOasisNetHandler.pfOasisNet_JOINProcess != NULL)
        {
            g_stOasisNetHandler.pfOasisNet_JOINProcess(true);
        }
    }
    else
    {
        /** Join was not successful. Try to join again */
        g_ucChannelRetry++;
        if (g_ucChannelRetry < 3)
        {
            bReJoin = true;
        }
        else
        {
            g_ucGrpRetry++;
            g_ucChannelRetry = 0;
            if (g_ucGrpRetry < 12)
            {
                bReJoin = true;
            }
            
            g_ucGrp++;
            if (g_ucGrp > 12)
            {
                g_ucGrp = 1;
            }
            LADAPTER_SetJoinChannelGroup(g_ucGrp);
        }
        
        if (bReJoin == true)
        {
            DEBUG_PRINT("[Net]Join retry, group[%d], channel retry[%d], group retry[%d]\r\n", 
                      g_ucGrp, g_ucChannelRetry, g_ucGrpRetry);
//#if !defined( LORA_MODULE_WSL30X )
            (void)LADAPTER_Join( );
//#endif 
            g_bJoining = true;
        }
        else
        {
            DEBUG_PRINT("[Net]Join failed.\r\n");
            g_bJoining = false;
            if (g_stOasisNetHandler.pfOasisNet_JOINProcess != NULL)
            {
                g_stOasisNetHandler.pfOasisNet_JOINProcess(false);
            }
        }
    }

    return;
}

void OasisNet_ReJoin(void)
{
    g_bJoined = false;

    OASISNET_Join();
    return;
}

static void OasisNet_ProcPktDroped(void)
{
    /* 丢包后，智能调速需要重新计算*/
#if ( OASIS_SDK_INTELIGENCE_DATARATE == 1 )
    OasisNet_ClearIntelligentDR();
#endif
    g_ucPktDroped++;
    if (g_ucPktDroped >= 3)
    {
        if (g_enDR > g_enBaseDR)
        {
            g_enDR = g_enBaseDR;
        }
        else
        {
            /* 触发重新入网*/
            OasisNet_ReJoin();
        }
        g_ucPktDroped = 0;
    }

    return;
}

/**
 * @brief   适配层报文应答处理函数
 *
 * @param   [输入] unsigned char 类型的参数,表示应答处理结果.
 *             
 */
static void OasisNet_ProcMcpsConfirm( uint8_t ucRet )
{
    OASIS_NETPKT_NODE_S *pstPktNode = NULL;

    g_stOasisNetPktCache.bSending = false;
    pstPktNode = OasisNetPkt_DelSndNode();
    if (pstPktNode != NULL)
    {
        DEBUG_PRINT("[Net]Receive packet confirmed, confrim=%d, fport=%d, result=%d.\r\n",
                     pstPktNode->bConfirm, pstPktNode->ucFPort, ucRet);

        if (pstPktNode->bConfirm == true)
        {
            if (ucRet != ERROR_SUCCESS)
            {
                OasisNet_ProcPktDroped();
            }
            else
            {
                g_ucPktDroped = 0;
            }
        }

        if ((g_apfFRMPayloadHandler[pstPktNode->ucFPort] != NULL) &&
            (g_apfFRMPayloadHandler[pstPktNode->ucFPort]->pfOasis_ProcMcpsConfirm != NULL))
        {
            g_apfFRMPayloadHandler[pstPktNode->ucFPort]->pfOasis_ProcMcpsConfirm((OASIS_NETPKT_HANDLER)pstPktNode, ucRet);
        }

        free(pstPktNode);
    }
    
    return;
}

/**
 * @brief   报文接收处理处理函数
 *
 * @param   [输入] unsigned char 类型的参数,表示应答处理结果.
 *             
 */
static void OasisNet_ProcFRMPktReceive(uint8_t ucFPort, char *pcData, uint8_t ucDataLen, 
                                                uint16_t usRSSI, uint8_t ucSNR)
{
    DEBUG_PRINT("[Net]Receive packet, Fport=%d, length=%d, RSSI=%hd, SNR=%hd.\r\n",
                ucFPort, ucDataLen, (int16_t)usRSSI, (int8_t)ucSNR);
    for(int i=0; i<ucDataLen; i++)
    {
        DEBUG_PRINT("%02X ", pcData[i]);
    }
    DEBUG_PRINT("\r\n");
    
    g_ucPktDroped = 0;
#if ( OASIS_SDK_INTELIGENCE_DATARATE == 1 )
    OasisNet_IntelligentDR(usRSSI, ucSNR);
#endif
    if ((g_apfFRMPayloadHandler[ucFPort] != NULL) &&
        (g_apfFRMPayloadHandler[ucFPort]->pfOasis_ProcFRMPktReceive != NULL))
    {
        g_apfFRMPayloadHandler[ucFPort]->pfOasis_ProcFRMPktReceive(pcData, ucDataLen);
    }

    return;
}

/**
 * @brief   发送拉取数据超时事件
 *
 * @param   [输入] void* 类型的参数,暂无.
 *             
 */
static void OasisNet_PullTimerCB(void *context)
{
    DEBUG_PRINT("[Net]Pull timer timeout.\r\n");
    OASISNET_SendPullDataReq(OASIS_NETPKT_CONFIRMED);
    
    return;
}

/**
 * @brief   Fpendig标记处理
 *
 * @param   [输入] bool 类型标记true表示下行报文fpending位为1
 *                 ,false 为 0
 */
static void OasisNet_ReceiveFPending(bool bFPending)
{
    DEBUG_PRINT("[Net]Receive FPending.\r\n");
    
    TimerStart(&g_stOasisNetPullTimer);
    return;
}


/**
 * @brief   发送拉取数据超时事件
 *
 * @param   [输入] OASIS_NETPKT_TYPE_E enConfirmType 确认类型
 * 
 * @param   [输入] unsigned char 类型的端口
 * 
 * @param   [输入] char* 类型的指针表示发送的数据
 *             
 * @param   [输入] unsigned char 类型的发送数据的长度
 *
 * @return OASIS_NETPKT_HANDLER 类型表示进队列的位置
 */
OASIS_NETPKT_HANDLER OASISNET_SendPkt(OASIS_NETPKT_TYPE_E enConfirmType, uint8_t ucFPort, char *pData, uint8_t ucDataSize)
{
    bool bConfirm = true;
    OASIS_NETPKT_HANDLER hPktHandler = OASIS_NETPKT_HANDLER_INVALID;

    if (g_bJoined == true)
    {
        if (OASIS_NETPKT_CONFIRMED != enConfirmType)
        {
            bConfirm = false;
        }

        hPktHandler = (OASIS_NETPKT_HANDLER)OasisNetPkt_AddPktNode(bConfirm, ucFPort, pData, ucDataSize);
    }

    return hPktHandler;
}

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
uint8_t OASISNET_RegisterFRMPktProc(uint8_t ucFPort, OASISNET_PKT_HANDLER_S *pstPKtHandler)
{
    OASISNET_PKT_HANDLER_S *pstHandler = NULL;
    uint8_t ucRet = ERROR_FAILED;
    
    /* FPort=0 is MAC command */
    if (ucFPort > 0)
    {
        pstHandler = (OASISNET_PKT_HANDLER_S *)malloc(sizeof(OASISNET_PKT_HANDLER_S));
        if (pstHandler != NULL)
        {
            pstHandler->pfOasis_ProcFRMPktReceive = pstPKtHandler->pfOasis_ProcFRMPktReceive;
            pstHandler->pfOasis_ProcMcpsConfirm = pstPKtHandler->pfOasis_ProcMcpsConfirm;
            g_apfFRMPayloadHandler[ucFPort] = pstHandler;
            ucRet = ERROR_SUCCESS;
        }
    }
    
    return ucRet;
}

/**
 * @brief   发送拉取数据超时事件
 *
 * @param   [输入] unsigned char 类型注册端口
 */
void OASISNET_DeRegisterFRMPktProc(uint8_t ucFPort)
{
    OASISNET_PKT_HANDLER_S *pstHandler = NULL;
    
    /* FPort=0 is MAC command */
    if ((ucFPort > 0) && (g_apfFRMPayloadHandler[ucFPort] != NULL))
    {
        pstHandler = g_apfFRMPayloadHandler[ucFPort];
        free(pstHandler);
        g_apfFRMPayloadHandler[ucFPort] = NULL;
    }
    
    return;
}

/**
 * @brief   设置网络层基础速率
 *
 * @param   [输入] LADAPTER_DR_E类型表示设置的数据速率
 */
void OASISNET_SetNetBaseDR(LADAPTER_DR_E enBaseDR)
{
    g_enBaseDR = enBaseDR;
    g_enDR = enBaseDR;

    return;
}


/**
 * @brief   设置网络层工作信道组
 *
 * @param   [输入] unsigned char类型表示设置的工作信道组
 */
void OASISNET_SetChannelGrp(uint8_t ucChnlGrp)
{
    g_ucGrp = ucChnlGrp;

    return;
}

#if ( OASIS_SDK_LINK_CHECK == 1 )
/**
 * @brief   链路检查
 *
 */
void OASISNET_DoLinkCheck(void)
{

    return;
}
#endif

#if ( OASIS_SDK_ADR == 1 )
/**
 * @brief   设置ADR开关
 *
 * @param   [输入] bool类型变量 true表示打开
 *                  ,false表示关闭
 */
uint8_t OASISNET_SetADR(bool bADROn)
{
    uint8_t ucRet = ERROR_SUCCESS;

    ucRet = LADAPTER_SetADR(bADROn);
    if (ERROR_SUCCESS == ucRet)
    {
        g_bAdrOn = bADROn;
    }

    return ucRet;
}
#endif

/**
 * @brief   拉取数据请求
 *
 * @param   [输入] OASIS_NETPKT_TYPE_E 类型变量
 *               OASIS_NETPKT_UNCONFIRMED 表示非确认
 *               OASIS_NETPKT_CONFIRMED 表示需要确认
 */
void OASISNET_SendPullDataReq(OASIS_NETPKT_TYPE_E enPktType)
{
    char szData[1] = {0x00};

    if (OasisNetPkt_IsCacheEmpty())
    {
        DEBUG_PRINT("[Net]Send pull data request[%d].\r\n", enPktType);
        OASISNET_SendPkt(enPktType, 0x03, szData, 1);
    }
    
    return;
}

/**
 * @brief   入网处理流程
 *
 */
uint8_t OASISNET_Join(void)
{
    uint8_t ucRet = ERROR_SUCCESS;
    
    if ((g_bJoining == true) || (g_bJoined == true))
    {
        /* 正在入网或已经入网，返回成功*/
        return ERROR_SUCCESS;
    }

    DEBUG_PRINT("[Net]Start Join survey, group[%d]\r\n", g_ucGrp);
    (void)LADAPTER_SetJoinChannelGroup(g_ucGrp);
    g_ucChannelRetry = 0;
    g_ucGrpRetry = 0;

    ucRet = LADAPTER_Join();
    if (ERROR_SUCCESS == ucRet)
    {
        g_bJoining = true;
    }

    return ucRet;
}

/**
 * @brief   网络层运行函数
 *
 */
void OASISNET_Run(void)
{
    if (g_bJoined == true)
    {
        OasisNetPkt_TriggerSend();
    }

    LADAPTER_Running();
    return;
}

/**
 * @brief   网络层初始化
 *
 */
void OASISNET_Init(OASISNET_HANDLER_S *pstNetHandler, uint8_t ucCacheCnt)
{
    LADAPTER_PKTHANDLER_S stPktHandler = {0};
    LADAPTER_HANDLER_S stHandler = {0};

    g_stOasisNetHandler = *pstNetHandler;
    g_bJoined = false;
    
    /** Handler CallBack Init */
    stHandler.pfLAdapter_JoinServer = OasisNet_JoinChannelSurvey;
    stHandler.pfLAdapter_ReceiveFPending = OasisNet_ReceiveFPending;
    stHandler.pfLAdapter_BoardGetBatteryLevel = pstNetHandler->pfOasisNet_BoardGetBatteryLevel;
    stHandler.pfLAdapter_OnMacProcessNotify = NULL;
    stHandler.pfLAdapter_GetTemperatureLevel = pstNetHandler->pfOasisNet_GetTemperatureLevel;
    LADAPTER_Init(&stHandler);

    stPktHandler.pfLAdapter_ProcFRMPktReceive = OasisNet_ProcFRMPktReceive;
    stPktHandler.pfLAdapter_ProcMcpsConfirm = OasisNet_ProcMcpsConfirm;
    LADAPTER_RegisterFRMPktProc(&stPktHandler);

    OasisNetPkt_InitCache(ucCacheCnt);

    TimerInit( &g_stOasisNetPullTimer, OasisNet_PullTimerCB );
    TimerSetValue(&g_stOasisNetPullTimer, 3 * 1000);
    
    return;
}



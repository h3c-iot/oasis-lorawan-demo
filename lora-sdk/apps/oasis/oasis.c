/**
* @file        oasis.c
*
* @copyright   Copyright (C) 2019, New H3C Technologies Co., Ltd.            
*              License: Revised BSD License 
*              ,see LICENSE.TXT file included in the project.
*
* @brief       oasis protocol extension
*                                                                                   
* @code        
*                          Author : yuxin
*
*                          Project Code: oasis-sdk
*
*                          Module Name: oasis
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
#include "debug.h"
#include "timer.h"
#include "base.h"
#include "oasis.h"
#include "oasisnet.h"
#include "sdk-config.h"

#define THIS_IS(para)   1


/** 绿洲扩展端口 */
#define OASIS_EXTEND_PROTOCCOL_PORT     0x0E


/** 绿洲最大Cache长度*/
#define OASIS_MAX_CACHE_LEN             10

/**
 * 应用数据最大长度
 */   
#define OASIS_DATA_MAX_SIZE             32

/**
 * 保活报文发送间隔
 */ 
#define OASIS_ALIVE_INTERVAL            60

/**
 * 下行
 */   
#define OASIS_HEAD_DOWNLINK             0x01
#define OASIS_HEAD_SN                   0x02
#define OASIS_HEAD_REQ                  0x04
#define OASIS_HEAD_VERSION              0xC0

/**
 * 重启延时
 */
#define OASIS_TYPE_RESET_DELAY          10000


/**
 * 绿洲Class配置长度
 */
#define OASIS_MSG_SYNC_START_LEN                 2

/**
 * 绿洲智能调速配置长度
 */
#define OASIS_MSG_INTERLLIGENCE_DATARATE_LEN     2


/**
 * 绿洲保活报文间隔
 */
#define OASIS_MSG_INTERVAL_LEN             4

/**
 * 绿洲ADR配置长度
 */
#define OASIS_MSG_ADR_LEN                  2

/**
 * 绿洲退避检测配置长度
 */
#define OASIS_MSG_BACKOFF_LEN              2

/**
 * 绿洲协议type类型枚举
 */    
typedef enum tagOasisMsgType
{
    OASIS_MSGTYPE_SYNC_START = 0,
    OASIS_MSGTYPE_DATA_PULL,
    OASIS_MSGTYPE_ALIVE,
    OASIS_MSGTYPE_SET_ALIVE_INTERVAL,
    OASIS_MSGTYPE_SET_ADR,
    OASIS_MSGTYPE_LINK_CHECK,        /* 5 */
    OASIS_MSGTYPE_REROOT,
    OASIS_MSGTYPE_SET_CSMACA,
    OASIS_MSGTYPE_SET_INTERLLIGENCE_DATARATE,
    OASIS_MSGTYPE_SET_DEVICE_MODE,   /* not support */
    OASIS_MSGTYPE_ACK = 0xFE,
    OASIS_MSGTYPE_SYNC_END
}OASIS_MSGTYPE_E;

/**
 * 重启定时器
 */
static TimerEvent_t g_stResetTimer;

/**
 * 上层回调句柄
 */
static OASIS_HANDLER_S g_stOasisHandler = {0};

#define OASIS_PKT_FLAGS_APP_ACK  0x01

typedef enum tagOASISPktType
{
    /* 链路层不可靠传输*/
    OASIS_PKT_TYPE_UNCFM = 0x00,  
    
    /* 链路层保证可靠传输, 所有下行需要response都用本模型*/
    OASIS_PKT_TYPE_LINKCFM = 0x01, 

    /* 应用层保证可靠传输, request&response模型*/
    OASIS_PKT_TYPE_APPCFM  = 0x02, 
}OASIS_PKT_TYPE_E;

/**
* Packet Node结构体
*/
typedef struct tagPKTNode
{
    OASIS_NETPKT_HANDLER hPkthander;
    OASIS_PKT_TYPE_E enPktType;
    OASIS_MSGTYPE_E enType;
    uint8_t ucFlags;
    uint8_t ucLen;
    char aucData[OASIS_DATA_MAX_SIZE];
}OASIS_PKT_NODE_S;

/**
* Cache数组结构体，先进先出，串行发包
*/
typedef struct tagPKTCache
{
    uint8_t ucIn;
    uint8_t ucOut;
    uint8_t ucCnt;
    OASIS_PKT_NODE_S astDataList[OASIS_MAX_CACHE_LEN];
}OASIS_PKT_CACHE_S;

/**
* 存放发送报文的数组结构体
*/
static OASIS_PKT_CACHE_S g_stOasisPktCache = {0};

static bool g_bOasisPktSending = false;

/**
 * 报文重传定时器
 */
static TimerEvent_t g_stOasisPktReSendTimer;

static uint8_t g_ucOasksPktReSendCnt = 0;

static TimerEvent_t g_stOasisPktPullTimer;

static uint8_t g_ucUPSN = 0x00;
static uint8_t g_ucDownSN = 0x00;

static inline void OasisPkt_StopResendTimer(void)
{
    TimerStop(&g_stOasisPktReSendTimer);

    return;
}

static inline void OasisPkt_StartResendTimer(void)
{
    TimerStart(&g_stOasisPktReSendTimer);
    
    return;
}

static inline bool OasisPkt_IsCacheEmpty(void)
{
    return (g_stOasisPktCache.ucCnt == 0);
}

static uint8_t OasisPkt_AddNode(OASIS_PKT_TYPE_E enPktTYPE, OASIS_MSGTYPE_E enType,
                                     char *pcData, uint8_t ucLen)
{
    OASIS_PKT_NODE_S *pstNode = NULL;
    
    if (g_stOasisPktCache.ucCnt == OASIS_MAX_CACHE_LEN)
    {
        /* buffer is full */
        return ERROR_BUFFER_FULL;
    }

    pstNode = &g_stOasisPktCache.astDataList[g_stOasisPktCache.ucIn];
    pstNode->enPktType = enPktTYPE;
    pstNode->hPkthander = OASIS_NETPKT_HANDLER_INVALID;
    pstNode->enType = enType;
    pstNode->ucFlags = 0;
    pstNode->ucLen = ucLen;
    memcpy(pstNode->aucData, pcData, ucLen);

    g_stOasisPktCache.ucIn = (g_stOasisPktCache.ucIn + 1) % OASIS_MAX_CACHE_LEN;
    g_stOasisPktCache.ucCnt++;

    return ERROR_SUCCESS;
}

static OASIS_PKT_NODE_S *OasisPkt_GetNode(void)
{
    OASIS_PKT_NODE_S *pstNode = NULL;
    
    if (g_stOasisPktCache.ucCnt != 0)
    {
        pstNode = &g_stOasisPktCache.astDataList[g_stOasisPktCache.ucOut];
    }

    return pstNode;
}

static void OasisPkt_DelNode(void)
{
    if (g_stOasisPktCache.ucCnt != 0)
    {
        memset(&g_stOasisPktCache.astDataList[g_stOasisPktCache.ucOut], 0, sizeof(OASIS_PKT_NODE_S));
        g_stOasisPktCache.ucOut = (g_stOasisPktCache.ucOut + 1) % OASIS_MAX_CACHE_LEN;
        g_stOasisPktCache.ucCnt--;
    }

    return;
}

static void OasisPkt_DelNodeByMsgType(OASIS_MSGTYPE_E enMsgType)
{
    OASIS_PKT_NODE_S *pstNode = NULL;

    pstNode = OasisPkt_GetNode();
    if (NULL != pstNode)
    {
        if ((pstNode->enPktType == OASIS_PKT_TYPE_APPCFM) && 
            (pstNode->enType == enMsgType))
        {
            OasisPkt_DelNode();
        }
    }

    return;
}

static uint8_t Oasis_TriggerSendPullDataReq(void)
{
    char szData[2] = {0x00,OASIS_MSGTYPE_DATA_PULL};
    OASIS_NETPKT_HANDLER hPktHandler = OASIS_NETPKT_HANDLER_INVALID;
    uint8_t ucRet = ERROR_FAILED;

    DEBUG_PRINT("[Oasis]Tigger Send pull request.\r\n");
    hPktHandler = OASISNET_SendPkt(OASIS_NETPKT_CONFIRMED, 
                               OASIS_EXTEND_PROTOCCOL_PORT, szData, 2);
    if (OASIS_NETPKT_HANDLER_INVALID != hPktHandler)
    {
        DEBUG_PRINT("[Oasis]Send pull request success, handler=%p.\r\n", (void *)hPktHandler);
        g_bOasisPktSending = true;
        ucRet = ERROR_SUCCESS;
    }

    return ucRet;
}

/**
* @brief 发送Cache数组待确认的报文.
*/
static void Oasis_TriggerSend(void)
{
    OASIS_PKT_NODE_S *pstNode = NULL;
    OASIS_NETPKT_TYPE_E enLADPPktTYPE = OASIS_NETPKT_CONFIRMED;
    OASIS_NETPKT_HANDLER hPktHandler = OASIS_NETPKT_HANDLER_INVALID;

    if (g_bOasisPktSending == false)
    {
        pstNode = OasisPkt_GetNode();
        if (NULL != pstNode)
        {
            if ((pstNode->enPktType == OASIS_PKT_TYPE_APPCFM) && 
                 (pstNode->ucFlags == OASIS_PKT_FLAGS_APP_ACK))
            {
                /* 之前发送的request已经被确认收到，直接发送拉取报文*/
                Oasis_TriggerSendPullDataReq();
            }
            else
            {
                DEBUG_PRINT("[Oasis]Trigger send packet, packet type[%d], msg type[%d]\r\n", 
                        pstNode->enPktType, pstNode->enType);

                if (pstNode->enPktType == OASIS_PKT_TYPE_UNCFM)
                {
                    enLADPPktTYPE = OASIS_NETPKT_UNCONFIRMED;
                }
                hPktHandler = OASISNET_SendPkt(enLADPPktTYPE, OASIS_EXTEND_PROTOCCOL_PORT, 
                                                    pstNode->aucData, pstNode->ucLen);
                if (OASIS_NETPKT_HANDLER_INVALID != hPktHandler)
                {
                    pstNode->hPkthander = hPktHandler;
                    DEBUG_PRINT("[Oasis]Send success, packet type[%d], msg type[%d], handler=%p\r\n", 
                                pstNode->enPktType, pstNode->enType, (void *)hPktHandler);
                    g_bOasisPktSending = true;
                }
            }
            
            /* 开启重传定时器*/
            OasisPkt_StartResendTimer();
        }
    }
    
    return;
}

/**
* @brief   往Cache数组缓存报文，并发送Cache中送报文.
*
* @param   [输入]   bool类型变量
*          ,bConfirm = true 表示发送需要确认的报文 
*          ,bConfirm = false 表示发送不需要确认的报文.
*
*
* @param   [输入] char* 类型的指针,指向发送数据的缓存.
*
* @param   [输入] unsigned char 类型的发送数据长度.
*
* @return  unsigned char类型ucIndex 表示缓存在Cache数组中发送信息的索引
*
*/
static uint8_t Oasis_SendPacket(OASIS_PKT_TYPE_E enPktTYPE, OASIS_MSGTYPE_E enType,
                               char *pcData, uint8_t ucLen)
{
    uint8_t ucRet = ERROR_SUCCESS;
    
    ucRet = OasisPkt_AddNode(enPktTYPE, enType, pcData, ucLen);
    if (ucRet != ERROR_SUCCESS)
    {
        DEBUG_PRINT("[Oasis]Add packet cache failed, packet type[%d], msg type[%d]\r\n", 
                    enPktTYPE, enType);
    }
    
    Oasis_TriggerSend();

    return ucRet;
}
                               
/**
 * @brief  超时重传事件
 */
static void Oasis_ResendPktTimerCB(void *context)
{
    OASIS_PKT_NODE_S *pstNode = NULL;

    OasisPkt_StopResendTimer();
    DEBUG_PRINT("[Oasis]Resend packet timer timeout.\r\n");
    
    g_bOasisPktSending = false;
    pstNode = OasisPkt_GetNode();
    if (pstNode != NULL)
    {
        /* 重传两次后，则删除队列*/
        if (g_ucOasksPktReSendCnt < 2)
        {
            g_ucOasksPktReSendCnt++;
            DEBUG_PRINT("[Oasis]Resend packet for %d times, packet type[%d], msg type[%d]\r\n", 
                        g_ucOasksPktReSendCnt, pstNode->enPktType, pstNode->enType);
        }
        else
        {
            DEBUG_PRINT("[Oasis]Resend packet failed, packet type[%d], msg type[%d]\r\n", 
                        pstNode->enPktType, pstNode->enType);
            OasisPkt_DelNode();
            g_ucOasksPktReSendCnt = 0;
        }
        /*触发下一个报文发送*/
        Oasis_TriggerSend();
    }
    
    return;
}

/**
 * @brief  超时拉取数据请求事件
 *
 */
static void Oasis_PullTimerCB(void *context)
{
    DEBUG_PRINT("[Oasis]Pull timer timeout.\r\n");
    Oasis_TriggerSendPullDataReq();
    return;
}

/**
 * @brief  绿洲扩展协议层应答处理函数
 *
 * @param  [输入] OASIS_NETPKT_HANDLER 上层应答报文对应的请求报文标识. 
 *
 * @param  [输入] unsigned char 类型的应答处理结果. 
 */
static void Oasis_ProcMcpsConfirm(OASIS_NETPKT_HANDLER hPktHandler, uint8_t ucResult)
{
    OASIS_PKT_NODE_S *pstNode = NULL;

    g_bOasisPktSending = false;
    pstNode = OasisPkt_GetNode();
    if ((NULL != pstNode) && (pstNode->hPkthander == hPktHandler))
    {
        DEBUG_PRINT("[Oasis]Receive packet confirm, packet type[%d], msg type[%d], result=%d.\r\n", 
                    pstNode->enPktType, pstNode->enType, ucResult);
        /* 网络层handler已经释放，这里也设置为无效*/
        pstNode->hPkthander = OASIS_NETPKT_HANDLER_INVALID;
        
        if (ucResult != ERROR_SUCCESS)
        {
            /* 上一个报文发送收到失败确认，则可以立刻重传*/
            Oasis_ResendPktTimerCB(NULL);
        }
        else if ((pstNode->enPktType == OASIS_PKT_TYPE_LINKCFM) || 
                 (pstNode->enPktType == OASIS_PKT_TYPE_UNCFM))
        {
            /* 停止重传定时器*/
            OasisPkt_StopResendTimer();

            OasisPkt_DelNode();
            g_ucOasksPktReSendCnt = 0;

            /*触发下一个报文发送*/
            Oasis_TriggerSend();
        }
        else if (pstNode->enPktType == OASIS_PKT_TYPE_APPCFM)
        {
            /* 收到确认，延后发送拉取报文，否则MAC层会失败*/
//            TimerStart(&g_stOasisPktPullTimer);
            Oasis_TriggerSendPullDataReq();
            g_ucOasksPktReSendCnt = 0;
            pstNode->ucFlags = OASIS_PKT_FLAGS_APP_ACK;
        }
    }

    return;
}

/**
 * @brief   返回码处理
 *
 * @return  ucDataLen  表示添加返回码的长度
 */
static uint8_t Oasis_AddConfigResponse(char *pRespData, uint8_t ucResult, uint8_t ucTypeID)
{
    uint8_t ucDataLen = 0;
    
    if (pRespData != NULL)
    {
        pRespData[ucDataLen++] = OASIS_MSGTYPE_ACK;
        pRespData[ucDataLen++] = ucTypeID;
        pRespData[ucDataLen++] = ucResult;

        ucDataLen = 3;
    }
    
    return ucDataLen;
}

#if ( OASIS_SDK_KEEPALIVE == 1)

/**
 * 定期发送应用数据的定时器
 */
static TimerEvent_t g_stOasisAliveTxTimer;

/**
 * 保活报文发送间隔 单位:秒
 */
static uint32_t g_uiAliveInterval = OASIS_ALIVE_INTERVAL;


#if (OASIS_SDK_KEEPALIVE_INTERVAL == 1)
/**
 * @brief   保活报文发送周期
 *
 * @retval  OASIS_TYPE_SUCEESS  表示配置成功
 *
 * @retval  ERROR_FAILED   表示配置失败
 */
static uint8_t Oasis_ProcAliveInterval(char *pcData, uint8_t ucBuffLeft, 
                                    char *pcOutData, uint8_t *ucOutDataLen)
{
    uint8_t ucResult = ERROR_PARAMETER_INVALID;
    uint32_t uiTime = 30;

    if (ucBuffLeft >= OASIS_MSG_INTERVAL_LEN)
    {
        uiTime = ((pcData[1] * 60) + pcData[2]) * 60 + pcData[3];
        if (uiTime < 30)
        {
            uiTime = 30;
        }
        else if (uiTime > 86400)
        {
            uiTime = 86400;
        }
        g_uiAliveInterval = uiTime;
        DEBUG_PRINT("[Oasis]Process set alive interval[%ld]\r\n", g_uiAliveInterval);

        /* 重启定时器*/
        OASIS_StopAliveTimer();
        OASIS_StartAliveTimer();
        ucResult = ERROR_SUCCESS;
    }

    *ucOutDataLen += Oasis_AddConfigResponse(pcOutData, ucResult, OASIS_MSGTYPE_SET_ALIVE_INTERVAL);

    return ucResult;
}
#endif

/**
 * @brief 超时发送保活报文
 *
 * @param void*类型的指针,未使用
 */
static void Oasis_KeepAliveCB(void *context)
{
    char szData[3] = {0x02,0x00,OASIS_MSGTYPE_ALIVE};

    DEBUG_PRINT("[Oasis]Keepalive timer timeout.\r\n");
    if (OasisPkt_IsCacheEmpty())
    {
        g_ucUPSN++;
        szData[1] = g_ucUPSN;
        Oasis_SendPacket(OASIS_PKT_TYPE_APPCFM, OASIS_MSGTYPE_ALIVE, szData, sizeof(szData));
    }

    OASIS_StartAliveTimer();
    
    return;
}

/**
 * @brief 保活报文应答处理  
 */
static void Oasis_ProcKeepAliveRSP(void)
{
    /* 停止重传定时器*/
    OasisPkt_StopResendTimer();
    
    DEBUG_PRINT("[Oasis]Process Keepalive response.\r\n");
    OasisPkt_DelNodeByMsgType(OASIS_MSGTYPE_ALIVE);

    /*触发下一个报文发送*/
    Oasis_TriggerSend();

    return;
}

/**
 * @brief 停止保活报文定时器  
 */
void OASIS_StopAliveTimer(void)
{
    TimerStop(&g_stOasisAliveTxTimer);

    return;
}

/**
 * @brief 启动保活报文定时器   
 */
void OASIS_StartAliveTimer(void)
{
    TimerSetValue(&g_stOasisAliveTxTimer, g_uiAliveInterval * 1000);
    TimerStart(&g_stOasisAliveTxTimer);
    
    return;
}
#endif


#if ( OASIS_SDK_INTELIGENCE_DATARATE == 1 )
/**
 * @brief   设置智能调速开关
 *
 * @retval  OASIS_TYPE_SUCEESS  表示配置成功
 *
 * @retval  ERROR_FAILED   表示配置失败
 */
static uint8_t Oasis_ProcIntelligenceDR(char *pcData, uint8_t ucBuffLeft,
                                           char *pucOutData, uint8_t *ucOutDataLen)
{
    uint8_t ucRet = ERROR_PARAMETER_INVALID;
    uint8_t ucData = (pcData[1] & 0x01);

    if (ucBuffLeft >= OASIS_MSG_INTERLLIGENCE_DATARATE_LEN)
    {
        ucRet = ERROR_SUCCESS;
        OASISNET_SetIntelligentDR(ucData);

        DEBUG_PRINT("[Oasis]Process intellengency adjust datarate : %d\n", ucData);
    }
    *ucOutDataLen += Oasis_AddConfigResponse(pucOutData, ucRet, OASIS_MSGTYPE_SET_INTERLLIGENCE_DATARATE);
    
    return ucRet;
}
#endif

#if ( OASIS_SDK_SYNC_CONFIG == 1 )

/**
 * @brief   设置同步开始配置
 *
 */
static void Oasis_ProcSyncConfigStart(char *pcData, uint8_t ucBuffLeft)
{
    if (ucBuffLeft >= OASIS_MSG_SYNC_START_LEN)
    {
        /* 暂时不用对配置个数做检查*/
        DEBUG_PRINT("[Oasis]Process Sync config start, count=%d.\r\n", pcData[1]);
    }
    
    return;
}

/**
 * @brief   设置同步开始或结束配置
 */
static void Oasis_ProcSyncConfigEnd(void)
{
    /* 停止重传定时器*/
    OasisPkt_StopResendTimer();
    
    DEBUG_PRINT("[Oasis]Process Sync config End.\r\n");
    OasisPkt_DelNodeByMsgType(OASIS_MSGTYPE_SYNC_START);

    /*触发下一个报文发送*/
    Oasis_TriggerSend();
    
    return;
}

/**
 * @brief 发送配置同步请求 
 */
void OASIS_SendConfigSYNCReq(void)
{
    char szData[3] = {0x02,0x00,OASIS_MSGTYPE_SYNC_START};

    DEBUG_PRINT("[Oasis]Send Sync config request.\r\n");
    g_ucUPSN++;
    szData[1] = g_ucUPSN;
    Oasis_SendPacket(OASIS_PKT_TYPE_APPCFM, OASIS_MSGTYPE_SYNC_START, szData, sizeof(szData));

    return;
}

#endif

#if ( OASIS_SDK_ADR == 1 )
/**
 * @brief   设置ADR开关
 *
 * @retval  OASIS_TYPE_SUCEESS  表示配置成功
 *
 * @retval  ERROR_FAILED   表示配置失败
 */
static uint8_t Oasis_ProcSetADRConfig(char *pcData, uint8_t ucBuffLeft,
                                       char *pcOutData, uint8_t *ucOutDataLen)
{
    uint8_t ucResult = ERROR_PARAMETER_INVALID;
    bool bADROn = false;

    if (ucBuffLeft >= OASIS_MSG_ADR_LEN)
    {
        bADROn = (bool)((pcData[1]) & 0x01);
        DEBUG_PRINT("[Oasis]Process ADR switch :%d\r\n", bADROn);
        ucResult = OASISNET_SetADR(bADROn);
        if (ERROR_SUCCESS == ucResult)
        {
            DEBUG_PRINT("[Oasis]Process ADR switch[%d] success.\r\n", bADROn);
        }
    }
    
    *ucOutDataLen += Oasis_AddConfigResponse(pcOutData, ucResult, OASIS_MSGTYPE_SET_ADR);
    
    return ucResult;
}
#endif

#if ( OASIS_SDK_CSMACA == 1 )
/**
 * @brief   设置退避检测模式
 *
 * @retval  OASIS_TYPE_SUCEESS  表示配置成功
 *
 * @retval  ERROR_FAILED   表示配置失败
 */
static uint8_t Oasis_ProcSetCSMACA(char *pcData, uint8_t ucBuffLeft,
                                    char *pucOutData, uint8_t *ucOutDataLen)
{
    uint8_t ucResult = ERROR_PARAMETER_INVALID;

    if (ucBuffLeft >= OASIS_MSG_BACKOFF_LEN)
    {
        DEBUG_PRINT("[Oasis]Process CMSA-CA config, but not support.\r\n");
        ucResult = ERROR_NOT_SUPPORT;
        *ucOutDataLen += Oasis_AddConfigResponse(pucOutData, ucResult, OASIS_MSGTYPE_SET_CSMACA);
    }

    return ucResult;
}
#endif

/**
 * @brief   超时重启事件
 *
 */
static void Oasis_ResetTimerCB(void * context)
{
    if(g_stOasisHandler.pfOasis_ResetModule != NULL)
    {
        g_stOasisHandler.pfOasis_ResetModule();
    }
    return;
}

/**
 * @brief   设置重启配置
 *
 * @retval  OASIS_TYPE_SUCEESS  表示配置成功
 *
 * @retval  ERROR_FAILED   表示配置失败
 */
static void Oasis_ProcResetDevice(void)
{
    char szData[4] = {0x02,0x00,OASIS_MSGTYPE_REROOT, ERROR_SUCCESS};
    
    DEBUG_PRINT("[Oasis]Process Reset device, Timer Delay Time: %d\n", OASIS_TYPE_RESET_DELAY);

    /* 延迟重启，尽量保障回复报文发送*/
    TimerSetValue(&g_stResetTimer, OASIS_TYPE_RESET_DELAY);
    TimerStart(&g_stResetTimer);

    g_ucUPSN++;
    szData[1] = g_ucUPSN;

    /* 重启回复使用unconfirm报文，不保证可靠传输*/
    Oasis_SendPacket(OASIS_PKT_TYPE_UNCFM, OASIS_MSGTYPE_REROOT, szData, sizeof(szData));
    
    return; 
}

/**
 * @brief   绿洲扩展协议包处理
 *
 */
static void Oasis_ProcPort14FRMPkt(char *pcRecvData, uint8_t ucDataLen)
{
    uint8_t i = 0;
    uint8_t ucHead = 0;
    char aucData[OASIS_DATA_MAX_SIZE];
    uint8_t ucSndPktLen = 0;
    uint8_t ucDownSN = 0;
    
    if ((pcRecvData == NULL) || (ucDataLen < 2))
    {
        return;
    }

    /* check header, version & downlink */
    ucHead = pcRecvData[0];
    if (((ucHead & OASIS_HEAD_VERSION) != 0x00) || ((ucHead & OASIS_HEAD_DOWNLINK) != OASIS_HEAD_DOWNLINK))
    {
        return;
    }
    i++;

    if ((ucHead & OASIS_HEAD_SN) != 0x00)
    {
        ucDownSN = pcRecvData[1];
        if (((ucHead & OASIS_HEAD_REQ) == 0x00) && (ucDownSN < g_ucDownSN))
        {
            /* request, 检测下行SN */
            return;
        }
        else if (((ucHead & OASIS_HEAD_REQ) != 0x00) && (ucDownSN != g_ucUPSN))
        {
            /* response, 检测上行SN */
            return;
        }
        
        i++;
    }
    
    DEBUG_PRINT("[Oasis]Process FPort 14 packet, head=0x%02X\r\n", ucHead);
    ucSndPktLen = 2; /* 给头部和SN预留长度*/
    while (i < ucDataLen)
    {
        switch(pcRecvData[i])
        {
            case OASIS_MSGTYPE_SYNC_START:              /* 0 */
#if ( OASIS_SDK_SYNC_CONFIG == 1 )
                Oasis_ProcSyncConfigStart(pcRecvData + i, ucDataLen - i);
#endif
                i += OASIS_MSG_SYNC_START_LEN;
                break;
            case OASIS_MSGTYPE_ALIVE:                   /* 2 */
#if ( OASIS_SDK_KEEPALIVE == 1 )
                Oasis_ProcKeepAliveRSP();
#endif
                i++;
                break;
                case OASIS_MSGTYPE_SET_ALIVE_INTERVAL:  /* 3 */
#if ( OASIS_SDK_KEEPALIVE_INTERVAL == 1 )
                Oasis_ProcAliveInterval(pcRecvData + i, ucDataLen - i, 
                                        aucData + ucSndPktLen, &ucSndPktLen);
#endif 
                i += OASIS_MSG_INTERVAL_LEN;
                break;
            case OASIS_MSGTYPE_SET_ADR:                 /* 4 */
#if ( OASIS_SDK_ADR == 1 )
                Oasis_ProcSetADRConfig(pcRecvData + i, ucDataLen - i,
                                        aucData + ucSndPktLen, &ucSndPktLen);
#endif
                i += OASIS_MSG_ADR_LEN;
                break;
            case OASIS_MSGTYPE_LINK_CHECK:              /* 5 */
#if ( OASIS_SDK_LINK_CHECK == 1 )
                OASISNET_DoLinkCheck();
#endif
                i++;
                break;
            case OASIS_MSGTYPE_REROOT:                   /* 6 */
                Oasis_ProcResetDevice();
                i++;
                break;
            case OASIS_MSGTYPE_SET_CSMACA:               /* 7 */
#if ( OASIS_SDK_CSMACA == 1 )
                Oasis_ProcSetCSMACA(pcRecvData + i, ucDataLen - i,
                                      aucData + ucSndPktLen, &ucSndPktLen);
#endif 
                i += OASIS_MSG_BACKOFF_LEN;
                break;
                
            case OASIS_MSGTYPE_SET_INTERLLIGENCE_DATARATE:  /* 8 */
#if ( OASIS_SDK_INTELIGENCE_DATARATE == 1 )
                Oasis_ProcIntelligenceDR(pcRecvData + i, ucDataLen - i,
                                          aucData + ucSndPktLen, &ucSndPktLen);
#endif
                i += OASIS_MSG_INTERLLIGENCE_DATARATE_LEN;
                break;
            case OASIS_MSGTYPE_SYNC_END:                 /* 0xFF */
#if ( OASIS_SDK_SYNC_CONFIG == 1 )
                Oasis_ProcSyncConfigEnd();
#endif
                i++;
                break;
            default:
                return;
                
        }
    }

    /* 排除头部预留长度2字节，还有数据要发送*/
    if (ucSndPktLen > 2)
    {
        aucData[0] = 0x02; /* 添加头部*/
        aucData[1] = ucDownSN; /* 回复使用下行的SN回复*/
        g_ucDownSN = ucDownSN + 1;
        /* 对于下行配置统一打包回复，链路层确认保障可靠传输*/
        Oasis_SendPacket(OASIS_PKT_TYPE_LINKCFM, OASIS_MSGTYPE_SYNC_END, aucData, ucSndPktLen);
    }

    return;
}

/**
 * @brief   用于数据同步的报文
 *          ,开放给外部使用的
 *          ,使用confirm的方式
 */
void OASIS_SendPullDataReq(void)
{
    char szData[2] = {0x00,OASIS_MSGTYPE_DATA_PULL};

    Oasis_SendPacket(OASIS_PKT_TYPE_LINKCFM, OASIS_MSGTYPE_DATA_PULL, szData, sizeof(szData));
    
    return;
}

/**
 * @brief   绿洲协议初始化
 *
 */
void OASIS_Init(OASIS_HANDLER_S *pstTypeHandler)
{
    OASISNET_PKT_HANDLER_S stNetPktHandler = {0};

    stNetPktHandler.pfOasis_ProcFRMPktReceive = Oasis_ProcPort14FRMPkt;
    stNetPktHandler.pfOasis_ProcMcpsConfirm = Oasis_ProcMcpsConfirm;
    OASISNET_RegisterFRMPktProc(OASIS_EXTEND_PROTOCCOL_PORT, &stNetPktHandler);

    g_stOasisHandler.pfOasis_ResetModule = pstTypeHandler->pfOasis_ResetModule;
    TimerInit( &g_stResetTimer, Oasis_ResetTimerCB );

#if ( OASIS_SDK_KEEPALIVE == 1)
    TimerInit( &g_stOasisAliveTxTimer, Oasis_KeepAliveCB );
#endif

    TimerInit( &g_stOasisPktReSendTimer, Oasis_ResendPktTimerCB );
    TimerSetValue(&g_stOasisPktReSendTimer, 60 * 1000);

    TimerInit( &g_stOasisPktPullTimer, Oasis_PullTimerCB );
    TimerSetValue(&g_stOasisPktPullTimer, 3 * 1000);

    return;
}


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


/** ������չ�˿� */
#define OASIS_EXTEND_PROTOCCOL_PORT     0x0E


/** �������Cache����*/
#define OASIS_MAX_CACHE_LEN             10

/**
 * Ӧ��������󳤶�
 */   
#define OASIS_DATA_MAX_SIZE             32

/**
 * ����ķ��ͼ��
 */ 
#define OASIS_ALIVE_INTERVAL            60

/**
 * ����
 */   
#define OASIS_HEAD_DOWNLINK             0x01
#define OASIS_HEAD_SN                   0x02
#define OASIS_HEAD_REQ                  0x04
#define OASIS_HEAD_VERSION              0xC0

/**
 * ������ʱ
 */
#define OASIS_TYPE_RESET_DELAY          10000


/**
 * ����Class���ó���
 */
#define OASIS_MSG_SYNC_START_LEN                 2

/**
 * �������ܵ������ó���
 */
#define OASIS_MSG_INTERLLIGENCE_DATARATE_LEN     2


/**
 * ���ޱ���ļ��
 */
#define OASIS_MSG_INTERVAL_LEN             4

/**
 * ����ADR���ó���
 */
#define OASIS_MSG_ADR_LEN                  2

/**
 * �����˱ܼ�����ó���
 */
#define OASIS_MSG_BACKOFF_LEN              2

/**
 * ����Э��type����ö��
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
 * ������ʱ��
 */
static TimerEvent_t g_stResetTimer;

/**
 * �ϲ�ص����
 */
static OASIS_HANDLER_S g_stOasisHandler = {0};

#define OASIS_PKT_FLAGS_APP_ACK  0x01

typedef enum tagOASISPktType
{
    /* ��·�㲻�ɿ�����*/
    OASIS_PKT_TYPE_UNCFM = 0x00,  
    
    /* ��·�㱣֤�ɿ�����, ����������Ҫresponse���ñ�ģ��*/
    OASIS_PKT_TYPE_LINKCFM = 0x01, 

    /* Ӧ�ò㱣֤�ɿ�����, request&responseģ��*/
    OASIS_PKT_TYPE_APPCFM  = 0x02, 
}OASIS_PKT_TYPE_E;

/**
* Packet Node�ṹ��
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
* Cache����ṹ�壬�Ƚ��ȳ������з���
*/
typedef struct tagPKTCache
{
    uint8_t ucIn;
    uint8_t ucOut;
    uint8_t ucCnt;
    OASIS_PKT_NODE_S astDataList[OASIS_MAX_CACHE_LEN];
}OASIS_PKT_CACHE_S;

/**
* ��ŷ��ͱ��ĵ�����ṹ��
*/
static OASIS_PKT_CACHE_S g_stOasisPktCache = {0};

static bool g_bOasisPktSending = false;

/**
 * �����ش���ʱ��
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
* @brief ����Cache�����ȷ�ϵı���.
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
                /* ֮ǰ���͵�request�Ѿ���ȷ���յ���ֱ�ӷ�����ȡ����*/
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
            
            /* �����ش���ʱ��*/
            OasisPkt_StartResendTimer();
        }
    }
    
    return;
}

/**
* @brief   ��Cache���黺�汨�ģ�������Cache���ͱ���.
*
* @param   [����]   bool���ͱ���
*          ,bConfirm = true ��ʾ������Ҫȷ�ϵı��� 
*          ,bConfirm = false ��ʾ���Ͳ���Ҫȷ�ϵı���.
*
*
* @param   [����] char* ���͵�ָ��,ָ�������ݵĻ���.
*
* @param   [����] unsigned char ���͵ķ������ݳ���.
*
* @return  unsigned char����ucIndex ��ʾ������Cache�����з�����Ϣ������
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
 * @brief  ��ʱ�ش��¼�
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
        /* �ش����κ���ɾ������*/
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
        /*������һ�����ķ���*/
        Oasis_TriggerSend();
    }
    
    return;
}

/**
 * @brief  ��ʱ��ȡ���������¼�
 *
 */
static void Oasis_PullTimerCB(void *context)
{
    DEBUG_PRINT("[Oasis]Pull timer timeout.\r\n");
    Oasis_TriggerSendPullDataReq();
    return;
}

/**
 * @brief  ������չЭ���Ӧ������
 *
 * @param  [����] OASIS_NETPKT_HANDLER �ϲ�Ӧ���Ķ�Ӧ�������ı�ʶ. 
 *
 * @param  [����] unsigned char ���͵�Ӧ������. 
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
        /* �����handler�Ѿ��ͷţ�����Ҳ����Ϊ��Ч*/
        pstNode->hPkthander = OASIS_NETPKT_HANDLER_INVALID;
        
        if (ucResult != ERROR_SUCCESS)
        {
            /* ��һ�����ķ����յ�ʧ��ȷ�ϣ�����������ش�*/
            Oasis_ResendPktTimerCB(NULL);
        }
        else if ((pstNode->enPktType == OASIS_PKT_TYPE_LINKCFM) || 
                 (pstNode->enPktType == OASIS_PKT_TYPE_UNCFM))
        {
            /* ֹͣ�ش���ʱ��*/
            OasisPkt_StopResendTimer();

            OasisPkt_DelNode();
            g_ucOasksPktReSendCnt = 0;

            /*������һ�����ķ���*/
            Oasis_TriggerSend();
        }
        else if (pstNode->enPktType == OASIS_PKT_TYPE_APPCFM)
        {
            /* �յ�ȷ�ϣ��Ӻ�����ȡ���ģ�����MAC���ʧ��*/
//            TimerStart(&g_stOasisPktPullTimer);
            Oasis_TriggerSendPullDataReq();
            g_ucOasksPktReSendCnt = 0;
            pstNode->ucFlags = OASIS_PKT_FLAGS_APP_ACK;
        }
    }

    return;
}

/**
 * @brief   �����봦��
 *
 * @return  ucDataLen  ��ʾ��ӷ�����ĳ���
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
 * ���ڷ���Ӧ�����ݵĶ�ʱ��
 */
static TimerEvent_t g_stOasisAliveTxTimer;

/**
 * ����ķ��ͼ�� ��λ:��
 */
static uint32_t g_uiAliveInterval = OASIS_ALIVE_INTERVAL;


#if (OASIS_SDK_KEEPALIVE_INTERVAL == 1)
/**
 * @brief   ����ķ�������
 *
 * @retval  OASIS_TYPE_SUCEESS  ��ʾ���óɹ�
 *
 * @retval  ERROR_FAILED   ��ʾ����ʧ��
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

        /* ������ʱ��*/
        OASIS_StopAliveTimer();
        OASIS_StartAliveTimer();
        ucResult = ERROR_SUCCESS;
    }

    *ucOutDataLen += Oasis_AddConfigResponse(pcOutData, ucResult, OASIS_MSGTYPE_SET_ALIVE_INTERVAL);

    return ucResult;
}
#endif

/**
 * @brief ��ʱ���ͱ����
 *
 * @param void*���͵�ָ��,δʹ��
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
 * @brief �����Ӧ����  
 */
static void Oasis_ProcKeepAliveRSP(void)
{
    /* ֹͣ�ش���ʱ��*/
    OasisPkt_StopResendTimer();
    
    DEBUG_PRINT("[Oasis]Process Keepalive response.\r\n");
    OasisPkt_DelNodeByMsgType(OASIS_MSGTYPE_ALIVE);

    /*������һ�����ķ���*/
    Oasis_TriggerSend();

    return;
}

/**
 * @brief ֹͣ����Ķ�ʱ��  
 */
void OASIS_StopAliveTimer(void)
{
    TimerStop(&g_stOasisAliveTxTimer);

    return;
}

/**
 * @brief ��������Ķ�ʱ��   
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
 * @brief   �������ܵ��ٿ���
 *
 * @retval  OASIS_TYPE_SUCEESS  ��ʾ���óɹ�
 *
 * @retval  ERROR_FAILED   ��ʾ����ʧ��
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
 * @brief   ����ͬ����ʼ����
 *
 */
static void Oasis_ProcSyncConfigStart(char *pcData, uint8_t ucBuffLeft)
{
    if (ucBuffLeft >= OASIS_MSG_SYNC_START_LEN)
    {
        /* ��ʱ���ö����ø��������*/
        DEBUG_PRINT("[Oasis]Process Sync config start, count=%d.\r\n", pcData[1]);
    }
    
    return;
}

/**
 * @brief   ����ͬ����ʼ���������
 */
static void Oasis_ProcSyncConfigEnd(void)
{
    /* ֹͣ�ش���ʱ��*/
    OasisPkt_StopResendTimer();
    
    DEBUG_PRINT("[Oasis]Process Sync config End.\r\n");
    OasisPkt_DelNodeByMsgType(OASIS_MSGTYPE_SYNC_START);

    /*������һ�����ķ���*/
    Oasis_TriggerSend();
    
    return;
}

/**
 * @brief ��������ͬ������ 
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
 * @brief   ����ADR����
 *
 * @retval  OASIS_TYPE_SUCEESS  ��ʾ���óɹ�
 *
 * @retval  ERROR_FAILED   ��ʾ����ʧ��
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
 * @brief   �����˱ܼ��ģʽ
 *
 * @retval  OASIS_TYPE_SUCEESS  ��ʾ���óɹ�
 *
 * @retval  ERROR_FAILED   ��ʾ����ʧ��
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
 * @brief   ��ʱ�����¼�
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
 * @brief   ������������
 *
 * @retval  OASIS_TYPE_SUCEESS  ��ʾ���óɹ�
 *
 * @retval  ERROR_FAILED   ��ʾ����ʧ��
 */
static void Oasis_ProcResetDevice(void)
{
    char szData[4] = {0x02,0x00,OASIS_MSGTYPE_REROOT, ERROR_SUCCESS};
    
    DEBUG_PRINT("[Oasis]Process Reset device, Timer Delay Time: %d\n", OASIS_TYPE_RESET_DELAY);

    /* �ӳ��������������ϻظ����ķ���*/
    TimerSetValue(&g_stResetTimer, OASIS_TYPE_RESET_DELAY);
    TimerStart(&g_stResetTimer);

    g_ucUPSN++;
    szData[1] = g_ucUPSN;

    /* �����ظ�ʹ��unconfirm���ģ�����֤�ɿ�����*/
    Oasis_SendPacket(OASIS_PKT_TYPE_UNCFM, OASIS_MSGTYPE_REROOT, szData, sizeof(szData));
    
    return; 
}

/**
 * @brief   ������չЭ�������
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
            /* request, �������SN */
            return;
        }
        else if (((ucHead & OASIS_HEAD_REQ) != 0x00) && (ucDownSN != g_ucUPSN))
        {
            /* response, �������SN */
            return;
        }
        
        i++;
    }
    
    DEBUG_PRINT("[Oasis]Process FPort 14 packet, head=0x%02X\r\n", ucHead);
    ucSndPktLen = 2; /* ��ͷ����SNԤ������*/
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

    /* �ų�ͷ��Ԥ������2�ֽڣ���������Ҫ����*/
    if (ucSndPktLen > 2)
    {
        aucData[0] = 0x02; /* ���ͷ��*/
        aucData[1] = ucDownSN; /* �ظ�ʹ�����е�SN�ظ�*/
        g_ucDownSN = ucDownSN + 1;
        /* ������������ͳһ����ظ�����·��ȷ�ϱ��Ͽɿ�����*/
        Oasis_SendPacket(OASIS_PKT_TYPE_LINKCFM, OASIS_MSGTYPE_SYNC_END, aucData, ucSndPktLen);
    }

    return;
}

/**
 * @brief   ��������ͬ���ı���
 *          ,���Ÿ��ⲿʹ�õ�
 *          ,ʹ��confirm�ķ�ʽ
 */
void OASIS_SendPullDataReq(void)
{
    char szData[2] = {0x00,OASIS_MSGTYPE_DATA_PULL};

    Oasis_SendPacket(OASIS_PKT_TYPE_LINKCFM, OASIS_MSGTYPE_DATA_PULL, szData, sizeof(szData));
    
    return;
}

/**
 * @brief   ����Э���ʼ��
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


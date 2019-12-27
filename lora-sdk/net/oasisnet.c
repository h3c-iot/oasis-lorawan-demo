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


/** �������ݻص�������ָ��ṹ��*/
static OASISNET_PKT_HANDLER_S *g_apfFRMPayloadHandler[255] = {NULL};

/**
 * ��ʾ��������
 */
static uint8_t g_ucPktDroped = 0;

/**
 * ��ʾ����Ӧ���ʿ���
 */
static bool g_bAdrOn = false;

/**
 * ��ʾ������������
 */
static LADAPTER_DR_E g_enDR = LADAPTER_DR_1;

/**
 * ��ʾ����������������
 */
static LADAPTER_DR_E g_enBaseDR = LADAPTER_DR_1;

/**
 * ��ʾ�Ƿ�����
 */
static bool g_bJoined = false;

/**
 * ��ʾ�Ƿ���������������
 */
static bool g_bJoining = false;

/**
 * ��ʾ�����ŵ��� g_ucGrp = 1:����Ƶ��470.3 ~ 471.7MHZ
 */
static uint8_t g_ucGrp = 1;

/**
 * ��ʾ��ǰ�ŵ��������Ĵ���
 */
static uint8_t g_ucChannelRetry = 0;

/**
 * ��ʾ��̽��������ŵ�����
 */
static uint8_t g_ucGrpRetry = 0;

/**
 * ����������ָ��
 */
OASISNET_HANDLER_S g_stOasisNetHandler = {NULL,};

/**
 * ��ȥ���ݶ�ʱ��
 */
static TimerEvent_t g_stOasisNetPullTimer;


#if THIS_IS("NET_PKT")
/**
 *���ͱ�����󳤶�
 */
#define OASIS_NETPKT_MAX_DATA_LEN      255

/**
 * ������Ϣ�洢�ṹ��
 */
typedef struct tagOasisNETPKTNode
{
    /**
     * �˿�
     */
    uint8_t ucFPort;
    /**
     * �Ƿ�ȷ��
     */
    bool bConfirm;
    
    /**
     * ���ݳ���
     */
    uint8_t ucLen;

    /**
     * ����
     */
    char aucData[OASIS_NETPKT_MAX_DATA_LEN];
}OASIS_NETPKT_NODE_S;

/**
 * ���Ľ���Chace״̬�ṹ��
 */
typedef struct tagOasisNETPKTCache
{
    /**
     * ��ǰ��Cache�ı�������
     */
    uint8_t ucIn;
    
    /**
     * ��ǰ��Cache�ı�������
     */
    uint8_t ucOut;
    /**
     * ��ǰ����Cache�ı�����
     */    
    uint8_t ucCnt;
    /**
     * Cache����洢���������
     */      
    uint8_t ucMaxCacheCnt;
    /**
     * �����Ƿ��ڷ���״̬
     */
    bool bSending;
    /**
     * �����Ƿ��ڷ���״̬
     */
    OASIS_NETPKT_NODE_S **ppstDataList;
}OASIS_NETPKT_CACHE_S;

/** ȫ�ֽṹ�����,���ڴ���ϲ�Ӧ�ô����͵ı���*/
static OASIS_NETPKT_CACHE_S g_stOasisNetPktCache = {0};

/**
 * @brief   ��ȡ��ǰCache�ڴ����Ƿ�Ϊ��
 *
 * @retval  ����true ��ʾCache������Ϊ��
 *
 * @retval  ����false ��ʾCache���в�Ϊ��
 */
static inline bool OasisNetPkt_IsCacheEmpty(void)
{
    return (g_stOasisNetPktCache.ucCnt == 0);
}

/**
 * @brief   ��ȡ��ǰCache�ڴ����Ƿ�����˴����͵ı���
 *
 * @retval  ����true ��ʾCache�����ж����˴����͵ı��� 
 *
 * @retval  ����false ��ʾCache������û�ж��������͵ı���
 */
static inline bool OasisNetPkt_IsCacheFull(void)
{
    return (g_stOasisNetPktCache.ucMaxCacheCnt == g_stOasisNetPktCache.ucCnt);
}

/**
 * @brief   Cache��ʼ��
 *
 * @param   [����] unsigned char���͵ı���,
 *                 ��ʼ��Cache�Ĵ�С.
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
 * @brief   ��Cache�����һ������͵ı���
 *
 * @return  ���������Cache�����ͱ�����Ϣ�Ľṹ��ָ�� 
 *          ,�������NULL ��ʾCache���Ѿ������˴����͵ı���
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
 * @brief   ��ȡ��ǰCache�д����͵ı���
 *
 * @return  ���ش����Cache�����ͱ�����Ϣ�Ľṹ��ָ�� 
 *          ,�������NULL ��ʾCache��û����Ҫ���͵ı���
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
 * @brief   ���Cache��ǰ���ڷ��͵ı���
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
 * @brief   ����Cache�д�ŵı���
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
                /* ADRû�п���������Ӧ�ò������������*/
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
 * ���ܵ��ٿ���
 */
static bool g_bIntellengencyAdr = true;

/** �����յ����ĵļ���*/
static uint8_t g_ucPktRcvCnt = 0;

/**
 * �ź�ǿ��ƽ��ֵ
 *
 */
static int16_t g_sRSSIAvg = 0;

/**
 * �����ƽ��ֵ
 *
 */
static int8_t g_cSNRAvg = 0;

/**
 * @brief   ���ܵ���״̬�ָ�Ĭ��
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
 * @brief   ���ܵ��ٴ�������
 *
 * @param   [����] unsigned short���͵��ź�ǿ��
 *                         
 * @param   [����] unsigned char���͵������
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
            /*���������յ����ģ�����ж�̬����*/
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
            
            /*���㣬���¿�ʼ����*/
            OasisNet_ClearIntelligentDR();
        }
    }

    return;
}

/**
* @brief   �������ܵ��ٿ���.
*
* @param   [����] bool���͵ı���
*          ,true��ʾ�����ܵ���
*          ,false��ʾ�ر����ܵ���
*/
void OASISNET_SetIntelligentDR(bool bOn)
{
    if (bOn == false)
    {
        /* �ر��Զ����ԣ���ָ�����������*/
        g_enDR = g_enBaseDR;
        OasisNet_ClearIntelligentDR();
    }
    g_bIntellengencyAdr = bOn;

    return;
}

#endif

/**
* @brief   ��ʼ��������.
*
* @param   [����] uinsined char���͵��������
*          ,0��ʾ�����ɹ�
*          ,��0��ʾ����ʧ��
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
    /* ���������ܵ�����Ҫ���¼���*/
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
            /* ������������*/
            OasisNet_ReJoin();
        }
        g_ucPktDroped = 0;
    }

    return;
}

/**
 * @brief   ����㱨��Ӧ������
 *
 * @param   [����] unsigned char ���͵Ĳ���,��ʾӦ������.
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
 * @brief   ���Ľ��մ�������
 *
 * @param   [����] unsigned char ���͵Ĳ���,��ʾӦ������.
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
 * @brief   ������ȡ���ݳ�ʱ�¼�
 *
 * @param   [����] void* ���͵Ĳ���,����.
 *             
 */
static void OasisNet_PullTimerCB(void *context)
{
    DEBUG_PRINT("[Net]Pull timer timeout.\r\n");
    OASISNET_SendPullDataReq(OASIS_NETPKT_CONFIRMED);
    
    return;
}

/**
 * @brief   Fpendig��Ǵ���
 *
 * @param   [����] bool ���ͱ��true��ʾ���б���fpendingλΪ1
 *                 ,false Ϊ 0
 */
static void OasisNet_ReceiveFPending(bool bFPending)
{
    DEBUG_PRINT("[Net]Receive FPending.\r\n");
    
    TimerStart(&g_stOasisNetPullTimer);
    return;
}


/**
 * @brief   ������ȡ���ݳ�ʱ�¼�
 *
 * @param   [����] OASIS_NETPKT_TYPE_E enConfirmType ȷ������
 * 
 * @param   [����] unsigned char ���͵Ķ˿�
 * 
 * @param   [����] char* ���͵�ָ���ʾ���͵�����
 *             
 * @param   [����] unsigned char ���͵ķ������ݵĳ���
 *
 * @return OASIS_NETPKT_HANDLER ���ͱ�ʾ�����е�λ��
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
 * @brief   Fport�˿ڻص�ע�����
 *
 * @param   [����] unsigned char ���͵�ע��˿�
 *          
 * @param   [����] OASISNET_PKT_HANDLER_S* ���͵��ϲ�ص�����ָ��ṹ��
 *
 * @retval ERROR_SUCCESS ��ʾע��ɹ�
 *
 * @retval ERROR_FAILED ��ʾע��ʧ��
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
 * @brief   ������ȡ���ݳ�ʱ�¼�
 *
 * @param   [����] unsigned char ����ע��˿�
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
 * @brief   ����������������
 *
 * @param   [����] LADAPTER_DR_E���ͱ�ʾ���õ���������
 */
void OASISNET_SetNetBaseDR(LADAPTER_DR_E enBaseDR)
{
    g_enBaseDR = enBaseDR;
    g_enDR = enBaseDR;

    return;
}


/**
 * @brief   ��������㹤���ŵ���
 *
 * @param   [����] unsigned char���ͱ�ʾ���õĹ����ŵ���
 */
void OASISNET_SetChannelGrp(uint8_t ucChnlGrp)
{
    g_ucGrp = ucChnlGrp;

    return;
}

#if ( OASIS_SDK_LINK_CHECK == 1 )
/**
 * @brief   ��·���
 *
 */
void OASISNET_DoLinkCheck(void)
{

    return;
}
#endif

#if ( OASIS_SDK_ADR == 1 )
/**
 * @brief   ����ADR����
 *
 * @param   [����] bool���ͱ��� true��ʾ��
 *                  ,false��ʾ�ر�
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
 * @brief   ��ȡ��������
 *
 * @param   [����] OASIS_NETPKT_TYPE_E ���ͱ���
 *               OASIS_NETPKT_UNCONFIRMED ��ʾ��ȷ��
 *               OASIS_NETPKT_CONFIRMED ��ʾ��Ҫȷ��
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
 * @brief   ������������
 *
 */
uint8_t OASISNET_Join(void)
{
    uint8_t ucRet = ERROR_SUCCESS;
    
    if ((g_bJoining == true) || (g_bJoined == true))
    {
        /* �����������Ѿ����������سɹ�*/
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
 * @brief   ��������к���
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
 * @brief   ������ʼ��
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



/**
* @file        oasisnet.h
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
*                          Module Name: Oasis network                                                 
*                                                     
*                          Date Created: 2019-10-26  
*
*------------------------------------------------------------------------------*
*  Modification History                                                        *
*  DATE              NAME                  DESCRIPTION                         *
*------------------------------------------------------------------------------*
*  YYYY-MM-DD                                                                  *
*------------------------------------------------------------------------------*                                                                             
* @endcode   
*
* @defgroup  OASISNET oasis network  layer
*            this module is oasis network 
* @{
*/

#ifndef __OASISNET_H__
#define __OASISNET_H__


#define OASIS_NETPKT_HANDLER          uint32_t

#define OASIS_NETPKT_HANDLER_INVALID  (OASIS_NETPKT_HANDLER)0


/**
 * ��������
 */
typedef enum tagOASISPktsType
{
    /** 
      * ����Ҫȷ������ 
      */
    OASIS_NETPKT_UNCONFIRMED,
    /** 
      * ��Ҫȷ������ 
      */
    OASIS_NETPKT_CONFIRMED,
}OASIS_NETPKT_TYPE_E;

/**
 * ���ڱ��洦��mlme״̬���¼����ϲ㺯��ָ��ṹ��
 */
typedef struct tagOASISNETHandler
{
    /**
     * ��ȡ�¶Ȼص�����
     */    
    float ( *pfOasisNet_GetTemperatureLevel)(void);
    /**
     * ��ȡ�����ص�����
     */
    uint8_t ( *pfOasisNet_BoardGetBatteryLevel)( void );
    void ( *pfOasisNet_JOINProcess )( bool bJoined);
}OASISNET_HANDLER_S;


/**
 * ���ڱ��洦������������ݺ�ȷ�ϵ��ϲ㺯��ָ��ṹ��
 */
typedef struct tagOasisNetPktHandler
{
    /**
     * �����������ݻص�����
     */    
    void ( *pfOasis_ProcFRMPktReceive )( char *pcData, uint8_t ucDataLen);
    /**
     * ����ȷ�ϴ������ص�����
     */
    void ( *pfOasis_ProcMcpsConfirm )( OASIS_NETPKT_HANDLER hPktHandler, uint8_t ucResult);
}OASISNET_PKT_HANDLER_S;


/**
 * @brief  ���ͽӿ�
 *
 * @param  [����] enConfirmType �������ݵ�����
 *         ,enConfirmType = OASIS_NETPKT_CONFIRMED ��ʾ������Ҫȷ�ϵı���
 *         ,enConfirmType = OASIS_NETPKT_UNCONFIRMED ��ʾ���Ͳ���Ҫȷ�ϵı���.
 *
 * @param  [����] unsigned char ���͵Ķ˿�
 *
 * @param  [����] char* ���͵ķ�������
 *
 * @param  [����] unsigned char ���͵ķ������ݳ���
 *
 * @retval Not NULL ��ʾ���ͱ��ĵ�Handler
 *
 * @retval NULL ��ʾ����ʧ�ܣ������������
 */
OASIS_NETPKT_HANDLER OASISNET_SendPkt(OASIS_NETPKT_TYPE_E enConfirmType, uint8_t ucFPort, char * pData, uint8_t ucDataSize);


/**
 * @brief  Fport�˿ڻص�ע�����
 *
 * @param  [����] uint8_t ���Ķ˿�
 * @param  [����] LADAPTER_PKTHANDLER_S�շ�������ص��������
 *
 */
uint8_t OASISNET_RegisterFRMPktProc(uint8_t ucFPort, OASISNET_PKT_HANDLER_S *pstPKtHandler);

void OASISNET_SendPullDataReq(OASIS_NETPKT_TYPE_E enPktType);

void OASISNET_Init(OASISNET_HANDLER_S *pstNetHandler, uint8_t ucCacheCnt);

uint8_t OASISNET_Join(void);

void OASISNET_Run(void);

void OASISNET_SetIntelligentDR(bool bOn);

uint8_t OASISNET_SetADR(bool bADROn);

void OASISNET_DoLinkCheck(void);



#endif /*__OASISNET_H__*/


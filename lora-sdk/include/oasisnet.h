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

/**
 * �������е�ַ��������
 */
#define OASIS_NETPKT_HANDLER          uint32_t

/**
 * ����������Ч��ַ
 */
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
uint8_t OASISNET_RegisterFRMPktProc(uint8_t ucFPort, OASISNET_PKT_HANDLER_S *pstPKtHandler);

/**
 * @brief   ��ȡ��������
 *
 * @param   [����] OASIS_NETPKT_TYPE_E ���ͱ���
 *               OASIS_NETPKT_UNCONFIRMED ��ʾ��ȷ��
 *               OASIS_NETPKT_CONFIRMED ��ʾ��Ҫȷ��
 */
void OASISNET_SendPullDataReq(OASIS_NETPKT_TYPE_E enPktType);

/**
 * @brief   ������ʼ��
 *
 */
void OASISNET_Init(OASISNET_HANDLER_S *pstNetHandler, uint8_t ucCacheCnt);

/**
 * @brief   ������������
 *
 */
 uint8_t OASISNET_Join(void);

/**
 * @brief   ��������к���
 *
 */
void OASISNET_Run(void);

/**
* @brief   �������ܵ��ٿ���.
*
* @param   [����] bool���͵ı���
*          ,true��ʾ�����ܵ���
*          ,false��ʾ�ر����ܵ���
*/
void OASISNET_SetIntelligentDR(bool bOn);

/**
 * @brief   ����ADR����
 *
 * @param   [����] bool���ͱ��� true��ʾ��
 *                  ,false��ʾ�ر�
 */
uint8_t OASISNET_SetADR(bool bADROn);

/**
 * @brief   ��·���
 *
 */
void OASISNET_DoLinkCheck(void);



#endif /*__OASISNET_H__*/


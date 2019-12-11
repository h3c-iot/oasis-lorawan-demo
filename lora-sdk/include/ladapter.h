 /**
 * @file        ladapter.h
 *
 * @copyright   Copyright (C) 2019, New H3C Technologies Co., Ltd.            
 *              License: Revised BSD License 
 *              ,see LICENSE.TXT file included in the project.
 *
 * @brief       lora adapt to uart or spi
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
 * @defgroup  LADAPTER LoRa Adapt layer
 *            this module adapt to spi or uart.
 * @{
 */

#ifndef __LADAPTER_H__
#define __LADAPTER_H__

#include <stdbool.h>


/**
 * ������ʽ
 */
typedef enum tagLAdapterNetworkType
{
    /** 
      * �������� 
      */
    LADAPTER_NWKTYPE_OTAA,
    /** 
      * �ֶ����� 
      */
    LADAPTER_NWKTYPE_ABP,
    /** 
      * ��Ч������ʽ 
      */
    LADAPTER_NWKTYPE_MAX
}LADAPTER_NWKTYPE_E;
    
/**
 * ��������
 */
typedef enum tagLAdapterDR
{
    /**
     * CN470: SF12
     */
    LADAPTER_DR_0,
    /**
     * CN470: SF11
     */
    LADAPTER_DR_1,
    /**
     * CN470: SF10
     */
    LADAPTER_DR_2,
    /**
     * CN470: SF9
     */
    LADAPTER_DR_3,
    /**
     * CN470: SF8
     */
    LADAPTER_DR_4,
    /**
     * CN470: SF7
     */
    LADAPTER_DR_5,

    LADAPTER_DR_6,

    LADAPTER_DR_7,
}LADAPTER_DR_E;

/**
 * ���书��
 */
typedef enum tagLAdapterPowerLevel
{
    /*
    * CN470: 17dbm 
    */
    LADAPTER_PWRLEVEL_0,
    /*
    * CN470: 15dbm 
    */
    LADAPTER_PWRLEVEL_1,
    /*
    * CN470: 13dbm 
    */
    LADAPTER_PWRLEVEL_2,
    /*
    * CN470: 11dbm 
    */
    LADAPTER_PWRLEVEL_3,
    /*
    * CN470: 9dbm 
    */
    LADAPTER_PWRLEVEL_4,
    /*
    * CN470: 7dbm 
    */
    LADAPTER_PWRLEVEL_5,
    /*
    * CN470: 5dbm 
    */
    LADAPTER_PWRLEVEL_6,
    /*
    * CN470: 3dbm 
    */
    LADAPTER_PWRLEVEL_7,
}LADAPTER_PWRLEVEL_E;

/**
 * ����ģʽ
 */
typedef enum tagLAdaterWorkMode
{
    /**
     * ClassAģʽ
     */
    LADAPTER_CLASS_A,
    /**
     * ClassBģʽ
     */
    LADAPTER_CLASS_B,
    /**
     * ClassCģʽ
     */
    LADAPTER_CLASS_C,
}LADAPTER_CLASS_E;
    
/**
 * ���ڱ��洦������������ݺ�ȷ�ϵ��ϲ㺯��ָ��ṹ��
 */
typedef struct tagLAdapterPktHandler
{
    /**
     * �����������ݻص�����
     */    
    void ( *pfLAdapter_ProcFRMPktReceive )( uint8_t ucFPort, char *pcData, uint8_t ucDataLen, uint16_t usRSSI, uint8_t ucSNR);
    /**
     * ����ȷ�ϴ������ص�����
     */
    void ( *pfLAdapter_ProcMcpsConfirm )( uint8_t ucResult);
}LADAPTER_PKTHANDLER_S;

/**
 * ���ڱ��洦��mlme״̬���¼����ϲ㺯��ָ��ṹ��
 */
typedef struct tagLAdapterHandler
{
    /**
     * �����������ص�����
     */
    void ( *pfLAdapter_JoinServer )(uint8_t ucJoinResult);
    /**
     * �������Fpending��ǻص�����
     */
    void ( *pfLAdapter_ReceiveFPending )(bool bFPending);
    /**
     * ��ȡ�¶Ȼص�����
     */    
    float ( *pfLAdapter_GetTemperatureLevel)(void);
    /**
     * ��ȡ�����ص�����
     */
    uint8_t ( *pfLAdapter_BoardGetBatteryLevel)( void );
    /**
     * ������MacProcss״̬�ص�����
     */
    void ( *pfLAdapter_OnMacProcessNotify)( void );
}LADAPTER_HANDLER_S;

/**
 * @brief  ������ʼ��
 *
 * @param  [����] LADAPTER_HANDLER_S* Mac�������ص��ṹ��
 *
 */
void LADAPTER_Init(LADAPTER_HANDLER_S *pHander);

/**
 * @brief  �������������
 *
 * @retval ERROR_SUCCESS ��ʾ���������ʧ��
 *
 * @retval ERROR_FAILED ��ʾ���������ɹ�
 */
uint8_t LADAPTER_Join(void);

/**
 * @brief  ������������    
 */
void LADAPTER_Running(void);

uint8_t LADAPTER_Send(bool bConfirm, uint8_t ucFPort, char *pcData, uint8_t ucDataSize);


/**
 * @brief  ���Ĵ���ص�ע�����
 *
 * @param  [����] LADAPTER_PKTHANDLER_S�շ�������ص��������
 *
 */
void LADAPTER_RegisterFRMPktProc(LADAPTER_PKTHANDLER_S *pstPKtHandler);



/**
 * @brief  ��ȡ��ǰ�Ƿ�����
 *
 * @retval true ��ʾ������
 *
 * @retval false ��ʾδ����
 */
bool LADAPTER_IsNetworkJoined(void);


/**
 * @brief  �������������ŵ���ķ���Ƶ��
 *
 * @param  [����] unsigned char ���͵��ŵ���,ȡֵ��Χ 1 ~ 12.
 *
 * @par ʾ��:ucChngGrp = 1��ʾ������������Ƶ��470.3~471.7MHZ
 *
 * @retval LADAPTER_SUCCESS ��ʾ���óɹ�
 *
 * @retval ERROR_FAILED ��ʾ����ʧ��
 */
uint8_t LADAPTER_SetJoinChannelGroup(uint8_t ucChnlGrp);

/**
 * @brief  ���ù����ŵ���
 *
 * @param  [����] unsigned char ���͵��ŵ���,ȡֵ��Χ 1 ~ 12.
 *
 * @par:ʾ��:ucChngGrp = 1��ʾ������������Ƶ��470.3~471.7MHZ 
 *
 * @retval LADAPTER_SUCCESS ��ʾ���óɹ�
 *
 * @retval ERROR_FAILED ��ʾ����ʧ��
 */
uint8_t LADAPTER_SetWorkChannelGroup(uint8_t ucChnlGrp);


/**
 * @brief  ����������ʽ
 *
 * @param  [����] LADAPTER_NWKTYPE_E ���͵�������ʽ. 
 *
 * @retval LADAPTER_SUCCESS ��ʾ���óɹ�
 *
 * @retval ERROR_FAILED ��ʾ����ʧ��
 */
uint8_t LADAPTER_SetNetwork(LADAPTER_NWKTYPE_E enNetWork);

/**
 * @brief  ���÷��书��
 *
 * @param  [����] LADAPTER_PWRLEVEL_E ���͵ķ��书������.
 *
 * @retval LADAPTER_SUCCESS ��ʾ���óɹ�
 *
 * @retval ERROR_FAILED ��ʾ����ʧ��
 */
uint8_t LADAPTER_SetTxPower(LADAPTER_PWRLEVEL_E enPowerLevel);

/**
 * @brief  ���÷�������
 *
 * @param  [����] LADAPTER_DR_E ���͵�������������
 *
 * @retval LADAPTER_SUCCESS ��ʾ���óɹ�
 *
 * @retval ERROR_FAILED ��ʾ����ʧ��
 */
uint8_t LADAPTER_SetDR(LADAPTER_DR_E enDR);

/**
 * @brief  ����LoRa����ģʽ
 *
 * @param  [����] bool ���͵ı���
 *             ,bAdrON = true ��ʾ��ADR����
 *             ,bAdrON = false ��ʾ�ر�ADR����.
 *
 * @retval LADAPTER_SUCCESS ��ʾ���óɹ�
 *
 * @retval ERROR_FAILED ��ʾ����ʧ��
 */
uint8_t LADAPTER_SetADR(bool bAdrON);

/**
 * @brief  ����TxDone��RX1���մ��ڴ򿪵���ʱ
 *
 * @param  [����] unsigned char ���͵���ʱʱ��,��λ����, ȡֵ��Χ1 ~ 15
 *
 * @retval LADAPTER_SUCCESS ��ʾ���óɹ�
 *
 * @retval ERROR_FAILED ��ʾ����ʧ��
 */
uint8_t LADAPTER_SetRX1Delay(uint8_t ucRx1Delay);

/**
 * @brief  ����LoRa����ģʽ
 *
 * @param  [����] LADAPTER_CLASS_E ���͵Ĺ���ģʽ
 *        ,enClass =  LADAPTER_CLASS_A ��ʾClassAģʽ
 *        ,enClass =  LADAPTER_CLASS_B ��ʾClassBģʽ
 *        ,enClass =  LADAPTER_CLASS_C ��ʾClassCģʽ
 *
 * @retval LADAPTER_SUCCESS ��ʾ���óɹ�
 *
 * @retval ERROR_FAILED ��ʾ����ʧ��
 */
uint8_t LADAPTER_SetClassMode(LADAPTER_CLASS_E enClass);

/**
 * @brief  �����豸��ַ
 *
 * @param  [����] unsigned int���͵�����ID. 
 *
 * @retval LADAPTER_SUCCESS ��ʾ���óɹ�
 *
 * @retval ERROR_FAILED ��ʾ����ʧ��
 */
uint8_t LADAPTER_SetDevAddr(uint32_t uiDevAddr);

/**
 * @brief  �����˹������汾
 *
 * @param  [����] unsigned int ���͵İ汾��      
 */
uint8_t LADAPTER_SetABPVersion(uint32_t uiVersion);

/**
 * @brief  ���ø���Կ
 * 
 * @param  [����] unsigned char *���͵�ָ��,���õĸ���Կ. 
 * 
 * @retval LADAPTER_SUCCESS ��ʾ���óɹ�
 *
 * @retval ERROR_FAILED ��ʾ����ʧ��
 */
uint8_t LADAPTER_SetAPPKey(uint8_t *pucAppKey);

/**
 * @brief  �����������Կ
 * 
 * @param  [����] unsigned char *���͵�ָ��,���õ��������Կ. 
 * 
 * @retval LADAPTER_SUCCESS ��ʾ���óɹ�
 * 
 * @retval ERROR_FAILED ��ʾ����ʧ��
 */
uint8_t LADAPTER_SetNWKKey(uint8_t *pucNwkKey);

/**
 * @brief  ����ת������Ự��������Կ
 * 
 * @param  [����] unsigned char *���͵�ָ��,���õ�ת������Ự��������Կ. 
 *
 * @retval LADAPTER_SUCCESS ��ʾ���óɹ�
 *
 * @retval ERROR_FAILED ��ʾ����ʧ��
 */
uint8_t LADAPTER_SetFNWKKey(uint8_t *pucFNwkKey);

/**
 * @brief  ���������Ự������Կ
 *
 * @param  [����] unsigned char *���͵�ָ��,���õ�����������Կ. 
 * 
 * @retval LADAPTER_SUCCESS ��ʾ���óɹ�
 *
 * @retval ERROR_FAILED ��ʾ����ʧ��
 */
uint8_t LADAPTER_SetSNWKKey(uint8_t *pucSNwkKey);

/**
 * @brief  ��������Ự������Կ
 *
 * @param  [����] unsigned char *���͵�ָ��,���õ�����Ự������Կ.
 *
 * @retval LADAPTER_SUCCESS ��ʾ���óɹ�
 *
 * @retval ERROR_FAILED ��ʾ����ʧ��
 */
uint8_t LADAPTER_SetNWKSKey(uint8_t *pucNwkSKey);

/**
 * @brief  ����Ӧ�ûỰ��Կ
 *
 * @param  [����] unsigned char *���͵�ָ��,���õ�Ӧ�ûỰ��Կ.
 *
 * @retval LADAPTER_SUCCESS ��ʾ���óɹ�
 *
 * @retval ERROR_FAILED ��ʾ����ʧ��
 */
uint8_t LADAPTER_SetAPPSKey(uint8_t *pucAppSKey);

/**
 * @brief  ����JoinEuiҲ��AppEui
 *
 * @param  [����] unsigned char *���͵�ָ��,���õ�JoinEui.
 *
 * @retval LADAPTER_SUCCESS ��ʾ���óɹ�
 *
 * @retval ERROR_FAILED ��ʾ����ʧ��
 */
uint8_t LADAPTER_SetJoinEUI(uint8_t *pucJoinEUI);

/**
 * @brief  ����DeviceEui
 *
 * @param  [����] unsigned char *���͵�ָ��,���õ�DeviceEui.
 *
 * @retval LADAPTER_SUCCESS ��ʾ���óɹ�
 *
 * @retval ERROR_FAILED ��ʾ����ʧ��
 */
uint8_t LADAPTER_SetDevEUI(uint8_t *pucDevEUI);

/**
 * @brief  ����DeviceEui
 *
 * @param  [����] unsigned char *���͵�ָ��,���õ�DeviceEui. 
 *
 * @retval LADAPTER_SUCCESS ��ʾ���óɹ�
 *
 * @retval ERROR_FAILED ��ʾ����ʧ��
 */
uint8_t LADAPTER_SetNetID(uint32_t uiNetID);


#endif /*__LADAPTER_H__*/


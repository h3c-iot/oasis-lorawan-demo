/*******************************************************************************
*                                                                              *
*                Copyright (C) 2019, New H3C Technologies Co., Ltd.            *
*  License: Revised BSD License, see LICENSE.TXT file included in the project  *
*                                                                              *
*------------------------------------------------------------------------------*
*                                                                              *
*                                  wsl305s.h                                  *
*                                                                              *
*  Project Code: oasis-sdk                                                     *
*   Module Name: Lora Module WSL305S                                           *
*  Date Created: 2019-10-24                                                    *
*        Author: hxb18134                                                      *
*   Description: adapt to wsl305s module AT Command                            *
*                                                                              *
*------------------------------------------------------------------------------*
*  Modification History                                                        *
*  DATE        NAME             DESCRIPTION                                    *
*  ----------------------------------------------------------------------------*
*  YYYY-MM-DD                                                                  *
*                                                                              *
*******************************************************************************/


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __WSL305S_H__
#define __WSL305S_H__

#include <stdbool.h>


/** AT CMD 定义*/
/*****************************************************************************/
/** 检查连线是否正常运作 */
#define AT_CONNECT_CHECK    "@AT"               
#define AT_SHORT_HELP       "@AT?"

/** 模组重启 */
#define AT_MCU_RESET_HELP   "@ATZ?"
#define AT_MCU_RESET        "@ATZ"

/** 设置或查询APPEUI */
#define AT_APPEUI_HELP      "@AT+APPEUI?"
#define AT_APPEUI_GET       "@AT+APPEUI=?"
#define AT_APPEUI_SET       "@AT+APPEUI="       

/** 设置或查询APPKEY */
#define AT_APPKEY_HELP      "@AT+APPKEY?"
#define AT_APPKEY_GET       "@AT+APPKEY=?"
#define AT_APPKEY_SET       "@AT+APPKEY="       

/** 设置或查询app session key */
#define AT_APPSKEY_HELP     "@AT+APPSKEY?"
#define AT_APPSKEY_GET      "@AT+APPSKEY=?"
#define AT_APPSKEY_SET      "@AT+APPSKEY=" 

/** 设置或查询device address */
#define AT_DADDR_HELP       "@AT+DADDR?"
#define AT_DADDR_GET        "@AT+DADDR=?"
#define AT_DADDR_SET        "@AT+DADDR="   

/** 设置或查询device EUI */
#define AT_DEUI_HELP        "@AT+DEUI?"
#define AT_DEUI_GET         "@AT+DEUI=?"
#define AT_DEUI_SET         "@AT+DEUI="         

/** 设置或查询network session key */
#define AT_NWKSKEY_HELP     "@AT+NWKSKEY?"
#define AT_NWKSKEY_GET      "@AT+NWKSKEY=?"
#define AT_NWKSKEY_SET      "@AT+NWKSKEY="      


/** 设置confirm mode */
#define AT_CFM_HELP         "@AT+CFM?"          
#define AT_CFM_GET          "@AT+CFM=?"
#define AT_CFM_SET          "@AT+CFM="  

/** 设置confirm status */
#define AT_CFS_HELP         "@AT+CFS?"          
#define AT_CFS_GET          "@AT+CFS=?"
#define AT_CFS_SET          "@AT+CFS="          

/** join Lora network */
#define AT_JOIN_HELP        "@AT+JOIN?"         
#define AT_JOIN_RUN         "@AT+JOIN"         

/** Lora network join mode,0:ABP/1:OTAA */
#define AT_NJM_HELP         "@AT+NJM?"          
#define AT_NJM_GET          "@AT+NJM=?"
#define AT_NJM_SET          "@AT+NJM="         

/** 获取Lora连接当前状态 0：not join /1:joined */
#define AT_NJS_HELP         "@AT+NJS?"          
#define AT_NJS_GET          "@AT+NJS=?"         

/** 以原始格式获取最近收到的字串数据 */
#define AT_RECV_HELP        "@AT+RECV?"         
#define AT_RECV_GET         "@AT+RECV=?"        

/** 以二进制格式获取最近收到的字串数据 */
#define AT_RECVB_HELP       "@AT+RECVB?"            
#define AT_RECVB_GET        "@AT+RECVB=?"       

/** 在专用端口号上发送字串数据 */
#define AT_SEND_HELP        "@AT+SEND?"         
#define AT_SEND_RUN         "@AT+SEND="         

/** 在专用端口号上以二进制格式发送字串数据 */
#define AT_SENDB_HELP       "@AT+SENDB?"            
#define AT_SENDB_RUN        "@AT+SENDB="        

/** 设置自适应数据速率状态    ,0:off/1:on        */
#define AT_ADR_HELP         "@AT+ADR?"          
#define AT_ADR_GET          "@AT+ADR=?"         
#define AT_ADR_SET          "@AT+ADR="          

/** 获取用户LoraWAN频别 A/B/C        */
#define AT_CLASS_HELP       "@AT+CLASS?"        
#define AT_CLASS_GET        "@AT+CLASS=?"      
#define AT_CLASS_SET        "@AT+CLASS="        

/** 设置用户占空比参数状态              0:disable /1:enable */
#define AT_DCS_HELP         "@AT+DCS?"          
#define AT_DCS_GET          "@AT+DCS=?"         
#define AT_DCS_SET          "@AT+DCS="          

/** 获取用户数据速率           ，0~7 */
#define AT_DR_HELP          "@AT+DR?"          
#define AT_DR_GET           "@AT+DR=?"          
#define AT_DR_SET           "@AT+DR="           

/** 查询设置Fpending开关状态，0表示打开，1表示关闭 */
#define AT_FPC_HELP         "@AT+FPC?"          
#define AT_FPC_GET          "@AT+FPC=?"         
#define AT_FPC_SET          "@AT+FPC="          

/** 查询模组是否处于Fpending状态 */   
#define AT_FPS_GET          "@AT+FPS=?"    	           

/** 获取用户下行帧计数器           ，返回字符串格式整形数 */
#define AT_FCD_HELP         "@AT+FCD?"          
#define AT_FCD_GET          "@AT+FCD=?"         
#define AT_FCD_SET          "@AT+FCD="          

/** 获取用户上行帧计数器           ，返回字符串格式整形数 */
#define AT_FCU_HELP         "@AT+FCU?"          
#define AT_FCU_GET          "@AT+FCU=?"         
#define AT_FCU_SET          "@AT+FCU="          

/** 允许用户存取RX窗口1上的加入延迟时间                 */
#define AT_JN1DL_HELP       "@AT+JN1DL?"        
#define AT_JN1DL_GET        "@AT+JN1DL=?"       
#define AT_JN1DL_SET        "@AT+JN1DL="       

/** 允许用户存取RX窗口2上的加入延迟时间                 */
#define AT_JN2DL_HELP       "@AT+JN2DL?"        
#define AT_JN2DL_GET        "@AT+JN2DL=?"      
#define AT_JN2DL_SET        "@AT+JN2DL="        

/** 获取公共网络模式         0:off/1:on */
#define AT_PNM_HELP         "@AT+PNM?"          
#define AT_PNM_GET          "@AT+PNM=?"        
#define AT_PNM_SET          "@AT+PNM="    

/** 接收到的窗口1的延迟时间 */
#define AT_RX1DL_HELP       "@AT+RX1DL?"        
#define AT_RX1DL_GET        "@AT+RX1DL=?"       
#define AT_RX1DL_SET        "@AT+RX1DL="        

/** 接收到的窗口2的延迟时间 */
#define AT_RX2DL_HELP       "@AT+RX2DL?"        
#define AT_RX2DL_GET        "@AT+RX2DL=?"      
#define AT_RX2DL_SET        "@AT+RX2DL="        

/** 接收到的窗口2的数据速率 */
#define AT_RX2DR_HELP       "@AT+RX2DR?"        
#define AT_RX2DR_GET        "@AT+RX2DR=?"       
#define AT_RX2DR_SET        "@AT+RX2DR="        

/** 接收到的窗口2的频率 */
#define AT_RX2FQ_HELP       "@AT+RX2FQ?"        
#define AT_RX2FQ_GET        "@AT+RX2FQ=?"       
#define AT_RX2FQ_SET        "@AT+RX2FQ="        

/** 存取发送功率 */
#define AT_TXP_HELP         "@AT+TXP?"          
#define AT_TXP_GET          "@AT+TXP=?"         
#define AT_TXP_SET          "@AT+TXP="          

/** 存取接收最后一包的RSSI */
#define AT_RSSI_HELP        "@AT+RSSI?"         
#define AT_RSSI_GET        "@AT+RSSI=?"       

/** 存取最后收到的SNR（signal noise ratio）数据组 */
#define AT_SNR_HELP         "@AT+SNR?"          
#define AT_SNR_GET          "@AT+SNR=?"         

/** 存取模组软件版本 */
#define AT_VER_HELP         "@AT+VER?"          
#define AT_VER_GET          "@AT+VER=?"         

/** RF RSSI频率测试 */
#define AT_TRSSI_HELP       "@AT+TRSSI?"        
#define AT_TRSSI_RUN        "@AT+TRSSI"         

/* RF频率测试 */
#define AT_TTONE_HELP       "@AT+TTONE?"        
#define AT_TTONE_RUN        "@AT+TTONE"         

/** RF Tx Lora测试 */
#define AT_TTLRA_HELP       "@AT+TTLRA?"       
#define AT_TTLRA_RUN        "@AT+TTLRA"        

/** RF Rx Lora测试 */
#define AT_TRLRA_HELP       "@AT+TRLRA?"        
#define AT_TRLRA_RUN        "@AT+TRLRA"         

/** Lora测试设定 */
#define AT_TCONF_HELP       "@AT+TCONF?"        
#define AT_TCONF_GET        "@AT+TCONF=?"       
#define AT_TCONF_SET        "@AT+TCONF="        

/** set the module in LoraWAN Certification Mode */
#define AT_CERTIF_HELP      "@AT+CERTIF?"       
#define AT_CERTIF_RUN       "@AT+CERTIF"       

/** RX1频率 =   TX频率   */
#define AT_TRX_HELP         "@AT+TRX?"          
#define AT_TRX_GET          "@AT+TRX=?"         
#define AT_TRX_SET          "@AT+TRX="          

/** 存取接收到的窗口1的频率            */
#define AT_RX1FQ_HELP       "@AT+RX1FQ?"        
#define AT_RX1FQ_GET        "@AT+RX1FQ=?"       
#define AT_RX1FQ_SET        "@AT+RX1FQ="        

/** 存取发射通道1的频率            */
#define AT_TX1FQ_HELP       "@AT+TX1FQ?"        
#define AT_TX1FQ_GET        "@AT+TX1FQ=?"       
#define AT_TX1FQ_SET        "@AT+TX1FQ="        

/** 存取发射通道2的频率            */
#define AT_TX2FQ_HELP       "@AT+TX2FQ?"        
#define AT_TX2FQ_GET        "@AT+TX2FQ=?"      
#define AT_TX2FQ_SET        "@AT+TX2FQ="    

/** 存取发射通道3的频率            */
#define AT_TX3FQ_HELP       "@AT+TX3FQ?"        
#define AT_TX3FQ_GET        "@AT+TX3FQ=?"       
#define AT_TX3FQ_SET        "@AT+TX3FQ="    

/** 存取发射通道4的频率            */
#define AT_TX4FQ_HELP       "@AT+TX4FQ?"       
#define AT_TX4FQ_GET        "@AT+TX4FQ=?"       
#define AT_TX4FQ_SET        "@AT+TX4FQ="    

/** 存取发射通道5的频率            */
#define AT_TX5FQ_HELP       "@AT+TX5FQ?"       
#define AT_TX5FQ_GET        "@AT+TX5FQ=?"       
#define AT_TX5FQ_SET        "@AT+TX5FQ=" 

/** 存取发射通道6的频率            */
#define AT_TX6FQ_HELP       "@AT+TX6FQ?"       
#define AT_TX6FQ_GET        "@AT+TX6FQ=?"       
#define AT_TX6FQ_SET        "@AT+TX6FQ="   

/** 存取发射通道7的频率            */
#define AT_TX7FQ_HELP       "@AT+TX7FQ?"        
#define AT_TX7FQ_GET        "@AT+TX7FQ=?"       
#define AT_TX7FQ_SET        "@AT+TX7FQ="   

/* 存取发射通道8的频率            */
#define AT_TX8FQ_HELP       "@AT+TX8FQ?"        
#define AT_TX8FQ_GET        "@AT+TX8FQ=?"       
#define AT_TX8FQ_SET        "@AT+TX8FQ="        

/** 保存客户参数       */
#define AT_SAVE_RUN         "@AT+SAVE"          

/** 存取发射功率，0~20, 且不受区域限制               */
#define AT_TXO_HELP         "@AT+TXO?"          
#define AT_TXO_GET          "@AT+TXO=?"         
#define AT_TXO_SET          "@AT+TXO="         


/** CMD STATUS定义 */
/*****************************************************************************/
/** 命令执行正确返回OK */
#define AT_STATUS_OK                "OK" 

/** 一般错误 */
#define AT_STATUS_ERROR             "AT_ERROR"   

/** 命令执行正确返回OK */
#define AT_STATUS_SEND_OK           "Send OK" 

#define AT_PARAM_ERROR              "AT_PARAM_ERROR" 

/** 网络忙碌，命令无法完成 */
#define AT_BUSY_EEROR               "AT_BUSY_EEROR"    

/** 该参数太长 */
#define AT_TEST_PARAM_OVERFLOW      "AT_TEST_PARAM_OVERFLOW"  

/** 网络忙碌，尚未加入 */
#define AT_NO_NETWORK_JOINED        "AT_NO_NETWORK_JOINED"

/** 接收命令期间检测错误 */
#define AT_RX_ERROR                 "AT_RX_ERROR"               

/** AT指令执行成功 */
#define AT_CMD_SUCCESS              0  

/** AT指令执行不成功 */
#define AT_CMD_FAILUE               1                          

/** Lora入网模式 */
#define NJM_SET_ABP                 "0"
#define NJM_SET_OTAA                "1"

#define CFS_STATUS_OK               "1"
#define FPS_PENDING                 "1"

#define DEV_CLASS_A                 "A"
#define DEV_CLASS_B                 "B"
#define DEV_CLASS_C                 "C"

#define SET_DISABLE                 "0"
#define SET_ENABLE                  "1"

#define JOIN_FAILED                 "0"
#define JOIN_SUCCEED                "1"

#define DATA_RATE_LEVEL0            "0" 
#define DATA_RATE_LEVEL1            "1"  
#define DATA_RATE_LEVEL2            "2" 
#define DATA_RATE_LEVEL3            "3" 
#define DATA_RATE_LEVEL4            "4" 
#define DATA_RATE_LEVEL5            "5"  
#define DATA_RATE_LEVEL6            "6" 
#define DATA_RATE_LEVEL7            "7" 

/** 发送功率级别，0级17dBm，往后2dB递减 */
#define TX_POWER_LEVEL0             "0"             /* 17dBm */
#define TX_POWER_LEVEL1             "1"             /* 15dBm */
#define TX_POWER_LEVEL2             "2"             /* 13dBm */
#define TX_POWER_LEVEL3             "3" 
#define TX_POWER_LEVEL4             "4" 
#define TX_POWER_LEVEL5             "5"  
#define TX_POWER_LEVEL6             "6" 
#define TX_POWER_LEVEL7             "7"             /* 3dBm */


typedef struct tagWSL305Callback
{
    void ( *Wsl305sMcpsConfirm )( bool bConfirmePkt, uint8_t ucResult );
    void ( *Wsl305sMcpsIndication )( bool bFPending, uint8_t ucFPort, char *pcData, uint8_t ucRecvLen);
    void ( *Wsl305sMlmeConfirm )( bool bJoined );
}WSL305S_CALLBACK_S;


/* Exported functions ------------------------------------------------------- */
uint8_t WSL305S_AT_GetATCMD(const char *pucATCMD,char *pucRetValue, uint16_t usBuffLen);
uint8_t WSL305S_AT_SetATCMD(const char *pucATCMD ,char *pucValue);

void WSL305S_AT_ResetModule(void);
uint8_t WSL305S_AT_SetChannelGroup(uint8_t ucChnlGrp);
uint8_t WSL305S_AT_SaveConfig(void);
uint8_t WSL305S_AT_CheckBoard(void);

bool WSL305S_AT_IsNetworkJoined(void);
bool WSL305S_AT_IsPktConfirmed(void);
bool WSL305S_AT_IsServerCached(void);

uint8_t WSL305S_AT_SetDR(uint8_t ucDR);

uint8_t WSL305S_AT_SetRX1Delay(uint8_t ucRX1Delay);

uint8_t WSL305S_AT_SetAppKey(char *pucValue);

uint8_t WSL305S_AT_SetDevEUI(char *pcDevEUI);

uint8_t WSL305S_AT_SetAppEUI(char *pcAppEUI);

uint8_t WSL305S_AT_SetNwksKey(char *pcNwksKey);

uint8_t WSL305S_AT_SetAppsKey(char *pucValue);

uint8_t WSL305S_AT_SetDevAddr(uint32_t uiDevAddr);

uint8_t WSL305S_AT_SetCFMMode(bool bConfirm);

uint8_t WSL305S_AT_Join(void);

uint8_t WSL305S_AT_Send(bool bConfirm, 
                 uint8_t ucFPort, char *pData, uint8_t ucDataSize);

uint8_t WSL305S_AT_GetData(char *pcData, uint8_t ucBufferLen, uint8_t *pcFPort);

void WSL305S_AT_Running(void);

uint8_t WSL305S_AT_OpenUart(void);

void WSL305S_AT_CloseUart(void);

void WSL305S_AT_Init(WSL305S_CALLBACK_S *pHandler);


#endif /* __LORA_WSL305S_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

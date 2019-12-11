/*******************************************************************************
*                                                                              *
*                Copyright (C) 2019, New H3C Technologies Co., Ltd.            *
*  License: Revised BSD License, see LICENSE.TXT file included in the project  *
*                                                                              *
*------------------------------------------------------------------------------*
*                                                                              *
*                                    wsl305s.c                                 *
*                                                                              *
*  Project Code: oasis-sdk                                                     *
*   Module Name: WSL305S                                                       *
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


/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include <stdio.h>
#include "uart.h"
#include "delay.h"
#include "wsl305s.h"
#include "base.h"
#include "timer.h"
#include "debug.h"


/**
  *串口全局指针变量
  */
static Uart_t *g_pstUartSerial = NULL;

/** 每组信道的信道数 */
#define FQ_GRP_NUM                  8  

/** AT指令发送缓存 */
#define AT_CMDLEN_MAX               128                        

/*8 KEY 定义 */                
#define ENTER_KEY                   0x0D                       
#define RETURN_KEY                  0x0A                       

/** 信道频率字符串长度 */
#define WSL305S_FQ_STR_LENTH        9

/** 信道频率指令长度 */
#define WSL305S_TXFQ_CMD_LEN        10

/** 0号信道频率 */
#define WSL305S_CN470_FREQ_START    470300000U 

/** 信道带宽 */
#define WSL305S_LORA_BANDWIDTH      200000U

/** 数据缓存长度       */
#define WSL305S_MAX_DATA_LEN        242
#define WSL305S_MAX_ATCMD_LEN       20
#define WSL305S_MAX_SENDB_LEN       ((WSL305S_MAX_DATA_LEN * 2) + 4)
#define WSL305S_MAX_TX_LEN          (WSL305S_MAX_SENDB_LEN + WSL305S_MAX_ATCMD_LEN)
#define WSL305S_MAX_RX_LEN          ((WSL305S_MAX_DATA_LEN * 2) + 12)

#define HEX_LEN_16                  16
#define HEX_LEN_8                   8

/**
 *下发Confirm报文标志位
*/
bool g_bConfirm = false;

/**
 *Confirm报文确认标志位
*/
bool g_bPktConfirmed = true;

/**
 *接收Unconfirm报文确认标志位
*/
bool g_bUnconfirmSend = false;

/**
 *入网成功标志位
*/
bool g_bJoined = false;

/**
 *模组数据接收接收延迟默认值
*/
uint8_t g_ucRx1Delay = 1;

/**
 *模组相关回调数组
*/
WSL305S_CALLBACK_S g_stHandler = {NULL, };

/**
 *confirm报文报文响应定时器
*/
static TimerEvent_t g_stConfirmTimer;


/**
 *confirm报文报文响应定时超时事件处理
 *@param[in]      context  
*/
static void wsl305s_OnConfirmTimerEvent( void* context )
{
    TimerStop( &g_stConfirmTimer );

    g_bPktConfirmed = true;
    if (g_stHandler.Wsl305sMcpsConfirm != NULL)
    {
        g_stHandler.Wsl305sMcpsConfirm(true, ERROR_FAILED);
    }

    return;
}


/**
 *判断数据是否为十六进制格式
 *@param[in]      ucData    需要判断的数据
 *@retval         true      表示数据为16进制数据
 *@retval         false     表示数据不为16进制数据
*/
static bool wsl305s_IsHexFormat(uint8_t ucData)
{
    bool bHex = false;
    
    if (((ucData >= '0') && (ucData <= '9')) ||
         ((ucData >= 'a') && (ucData <= 'f')) ||
         ((ucData >= 'A') && (ucData <= 'F')))
    {
        bHex = true;
    }

    return bHex;
}


/**
 *模组AT指令发送
 *@param[in]      pucATCmd              AT指令部分字符串指针
 *@param[in]      pucData               AT指令参数部分字符串指针
 *@retval         ERROR_SUCCESS        表示指令发送成功
 *@retval         ERROR_FAILED         表示指令发送失败
*/
static uint8_t wsl305s_SendATCMD(const char *pucATCmd,
                                      char *pucData)
{

    char aucTxBuffer[WSL305S_MAX_TX_LEN];
    uint8_t ucTxIndex   = 0; 
    uint16_t usDataLen = 0;
    uint8_t ucRet = ERROR_SUCCESS;

    if ((pucATCmd == NULL) || (g_pstUartSerial == NULL))
    {
        return ERROR_FAILED;
    }
    
    aucTxBuffer[0] = 0;
    memcpy(aucTxBuffer, pucATCmd, strlen(pucATCmd));
    ucTxIndex += strlen(pucATCmd);

    if(NULL != pucData)
    {
        usDataLen = strlen(pucData);
        memcpy(aucTxBuffer + ucTxIndex, pucData, usDataLen);
        ucTxIndex += usDataLen;
    }

    aucTxBuffer[ucTxIndex++] = ENTER_KEY;                                   /* 结尾加上ENTER键 */
    aucTxBuffer[ucTxIndex++] = RETURN_KEY;                                  /* 结尾加上RETURN_KEY键 */
    aucTxBuffer[ucTxIndex] = 0;
    DEBUG_PRINT("aucTxBuffer ==> %s",aucTxBuffer);
    if (0 != UartPutBuffer(g_pstUartSerial, (uint8_t *)aucTxBuffer, ucTxIndex))
    {
        ucRet = ERROR_FAILED;
    }
    
    return ucRet;
}


/**
 *模组AT指令响应数据接收
 *@param[in]      pucGetData        接收数据缓存指针
 *@param[in]      usBufferLen       接收数据缓存长度
 *@retval         usRxDataLen       返回接收数据长度
*/
static uint16_t wsl305s_RecvUart(char *pucGetData, uint16_t usBufferLen)
{
    uint16_t usRxDataLen = 0;

    if ((pucGetData == NULL) || (g_pstUartSerial == NULL))
    {
        return 0;
    }

    UartGetBuffer(g_pstUartSerial, (uint8_t *)pucGetData, usBufferLen, &usRxDataLen);

    return usRxDataLen;
}


/**
 *通过AT指令在特定端口上发送字符串数据
 *@param[in]      pucData               发送数据字符串指针
 *@retval         ERROR_SUCCESS        表示发送成功
 *@retval         ERROR_FAILED         表示发送失败
*/
static uint8_t wsl305s_SendData(char *pucData)
{
    /* 发送数据发送AT指令 */
    return WSL305S_AT_SetATCMD(AT_SENDB_RUN, pucData);
}


/**
 *将数组转换为模组AT指令字符串参数格式
 *@param[in]      szArrData               转换后字符串指针
 *@param[in]      pucPreArr               待转换数组指针
 *@param[in]      uc8ArrLen               待转换数组字节数
*/
static void  wsl305s_Arr2Str(char *szArrData, char *pucPreArr, uint8_t uc8ArrLen)
{
    for (int i = 0; i < uc8ArrLen; i++)
    {
        sprintf(szArrData + i*3, "%02X:", pucPreArr[i]);
    }   
    
    szArrData[uc8ArrLen * 3 - 1] = 0;
    return;
}


/**
 *AT设置指令
 *@param[in]      pucATCMD               AT设置指令部分字符串指针
 *@param[in]      pucValue               AT设置参数部分字符串指针
 *@retval         ERROR_SUCCESS         表示指令执行成功
 *@retval         ERROR_FAILED          表示指令执行失败
*/
uint8_t WSL305S_AT_SetATCMD(const char *pucATCMD ,char *pucValue)
{
    char aucCmdData[AT_CMDLEN_MAX + 1] = "";
    uint8_t ucRet  = ERROR_FAILED;
    uint8_t ucRecvLen = 0;

    WSL305S_AT_OpenUart();
    DelayMs(5);

    /* 发送DevEUI设置指令 */
    if (ERROR_SUCCESS == wsl305s_SendATCMD(pucATCMD, pucValue))
    {
        DelayMs(100 + strlen(pucValue) / 30 * 60);

        ucRecvLen = wsl305s_RecvUart(aucCmdData, AT_CMDLEN_MAX);
        if (ucRecvLen != 0)
        {
            aucCmdData[ucRecvLen] = 0;
            if(strstr(aucCmdData, AT_STATUS_OK) != NULL)
            {
                DEBUG_PRINT("Send Ok, Data:[%s%s]\r\n", pucATCMD, ((pucValue == NULL) ? "" : pucValue));
                ucRet = ERROR_SUCCESS;
            }
            else
            {
                DEBUG_PRINT("Send failed, Receive Data <==\r\n%s\r\n", aucCmdData);
            }
        }
    }

    WSL305S_AT_CloseUart();

    return ucRet;
}


/**
 *AT查询参数指令
 *@param[in]      pucATCMD               AT查询指令部分字符串指针
 *@param[in]      pucRetValue            AT查询返回数据字符串指针
 *@param[in]      usBuffLen              AT查询返回数据缓存区长度
 *@retval         ERROR_SUCCESS         表示指令执行成功
 *@retval         ERROR_FAILED          表示指令执行失败
*/
uint8_t WSL305S_AT_GetATCMD(const char *pucATCMD, char *pucRetValue, uint16_t usBuffLen)
{
    char aucCmdData[WSL305S_MAX_RX_LEN + 1] = {0};
    char *pucDataEnd = NULL;
    char *pucDataStart = NULL;
    uint16_t usGetLen  = 0;
    uint16_t usRecvLen = 0;
    uint8_t ucRet = ERROR_FAILED;

    WSL305S_AT_OpenUart();
    DelayMs(5);

    /* 发送DevEUI设置指令 */
    if (ERROR_SUCCESS == wsl305s_SendATCMD(pucATCMD, NULL))
    {
        DelayMs(200);

        usRecvLen = wsl305s_RecvUart(aucCmdData, WSL305S_MAX_RX_LEN);
        if (usRecvLen != 0)
        {
            aucCmdData[usRecvLen] = 0;
            if (strstr(aucCmdData, AT_STATUS_OK) != NULL)
            {
                DEBUG_PRINT("Send Ok, Data==>%s\r\nReceive Data<==\r\n%s\r\n", pucATCMD, aucCmdData);

                pucDataEnd = strstr(aucCmdData, "\r\nOK\r\n");
                if (pucDataEnd != NULL)
                {
                    /* 跳过\r\n, 光宝模组每次Get操作都会有\r\n */
                    pucDataEnd = pucDataEnd - 2;

                    /* 从后往前逐个字符进行检测，直到遍历结束或遇到\r\n */
                    pucDataStart = pucDataEnd - 1;
                    while ((pucDataStart - aucCmdData) >= 0)
                    {
                        if ((*pucDataStart == '\r') || (*pucDataStart == '\n'))
                        {
                            break;
                        }
                        pucDataStart = pucDataStart - 1;
                    }

                    /* 指向需要获取的字符串的首字符位置*/
                    pucDataStart = pucDataStart + 1;
                    if ((pucDataStart >= aucCmdData) && (pucDataStart < pucDataEnd))
                    {
                        usGetLen = pucDataEnd - pucDataStart;
                        memcpy(pucRetValue, pucDataStart, usGetLen);
                        *(pucRetValue + usGetLen) = '\0';
                        ucRet = ERROR_SUCCESS;
                    }
                }

                /*
                pucDataEnd = strstr(aucCmdData, "\r\n");
                if (pucDataEnd != NULL)
                {
                    usGetLen = pucDataEnd - aucCmdData;
                    usGetLen = (usGetLen > (usBuffLen-1)) ? (usBuffLen-1) : usGetLen;
                    if (usGetLen > 0)
                    {
                        memcpy(pucRetValue, aucCmdData, usGetLen);
                        *(pucRetValue + usGetLen) = '\0';
                        ucRet = ERROR_SUCCESS;
                    }
                }*/
            }
            else
            {
                DEBUG_PRINT("Send failed, Receive Data<==\r\n%s\r\n", aucCmdData);
            }
        }
    }

    WSL305S_AT_CloseUart();
    
    return ucRet;
}


/**
 *AT指令模组复位重启
 *@param[in]        无
 *@note             该指令执行成功后，模组会软重启
*/
void WSL305S_AT_ResetModule(void)
{
    DEBUG_PRINT("@ATZ\r\n");

    WSL305S_AT_OpenUart();
    
    /* 发送AT重启模组 */
    (void)wsl305s_SendATCMD(AT_MCU_RESET, NULL);

    DelayMs(100);
    WSL305S_AT_CloseUart();
    
    return;
}


/**
 *设置发送某信道组的1-8信道的频率
 *@param[in]      ucChnlGrp              信道组编号，取值为1-12
 *@retval         ERROR_SUCCESS         表示指令执行成功
 *@retval         ERROR_FAILED          表示指令执行失败
*/
uint8_t WSL305S_AT_SetChannelGroup(uint8_t ucChnlGrp)
{
    uint8_t  ucRet = ERROR_SUCCESS;
    uint32_t auiLoraTxFrq[8] = {0};
    char  aucLoraFrqGrp[FQ_GRP_NUM][WSL305S_FQ_STR_LENTH + 1] ={0};             /* 信道组字符数组 */

    char aucATCmdTxFqGp[FQ_GRP_NUM][WSL305S_TXFQ_CMD_LEN + 1] = 
    {
        AT_TX1FQ_SET,
        AT_TX2FQ_SET,
        AT_TX3FQ_SET,
        AT_TX4FQ_SET,
        AT_TX5FQ_SET,
        AT_TX6FQ_SET,
        AT_TX7FQ_SET,
        AT_TX8FQ_SET
    };

    DEBUG_PRINT("WSL305S_AT_SetChannelGroup\r\n");   

    auiLoraTxFrq[0] = WSL305S_CN470_FREQ_START + WSL305S_LORA_BANDWIDTH * FQ_GRP_NUM * (ucChnlGrp - 1); 
    for(int i = 0; i < 8; i++)
    {
        auiLoraTxFrq[i] = auiLoraTxFrq[0] + WSL305S_LORA_BANDWIDTH * i; 
        sprintf(aucLoraFrqGrp[i], "%ld", auiLoraTxFrq[i]);
        if (ERROR_SUCCESS != WSL305S_AT_SetATCMD(aucATCmdTxFqGp[i], aucLoraFrqGrp[i]))
        {
            ucRet = ERROR_FAILED;
            break;
        }
    }
    
    return ucRet;
}


/**
 *保存配置参数到模组flash中
 *@param[in]        无
 *@note             模组无返回信息
*/
uint8_t WSL305S_AT_SaveConfig(void)
{
    DEBUG_PRINT("WSL305S_AT_SaveConfig\r\n"); 
    
    /* 发送参数保存AT指令 */
    return WSL305S_AT_SetATCMD(AT_SAVE_RUN, NULL);
}


/**
 *AT指令通信连接状态检查
 *@param[in]                           无
 *@retval       ERROR_SUCCESS         表示通信连接正常
 *@retval       ERROR_FAILED          表示通信连接失败
*/
uint8_t WSL305S_AT_CheckBoard(void)
{
    DEBUG_PRINT("WSL305S_AT_CheckBoard\r\n");
    
    /* 发送AT指令连接状态查询 */
    return WSL305S_AT_SetATCMD(AT_CONNECT_CHECK, NULL);
}


/**
 *判断是否入网成功
 *@param[in]                 无
 *@retval       true         表示模组入网成功
 *@retval       false        表示模组入网不成功
*/
bool WSL305S_AT_IsNetworkJoined(void)
{
    char aucCmdData[AT_CMDLEN_MAX] = {0};
    bool bJoined  = false;

    DEBUG_PRINT("WSL305S_AT_IsNetworkJoined\r\n");
    if ((ERROR_SUCCESS == WSL305S_AT_GetATCMD(AT_NJM_GET, aucCmdData, AT_CMDLEN_MAX)) && 
        (strstr(aucCmdData, NJM_SET_ABP) != NULL))
    {
        bJoined = true;
    }
    
    /* 发送入网AT指令 */
    if ((bJoined == false) &&
        (ERROR_SUCCESS == WSL305S_AT_GetATCMD(AT_NJS_GET, aucCmdData, AT_CMDLEN_MAX)) &&
        (strstr(aucCmdData, JOIN_SUCCEED) != NULL))
    {
        bJoined = true;
    }
    
    return bJoined;
}


/**
 *获取报文发送状态
 *@param[in]                 无
 *@retval       true         表示报文发送完成
 *@retval       false        表示报文发送未完成
*/
bool WSL305S_AT_IsPktConfirmed(void)
{
    char aucCmdData[AT_CMDLEN_MAX] = {0};
    bool bConfirmed  = false;

    DEBUG_PRINT("WSL305S_AT_IsPktConfirmed\r\n");
    if ((ERROR_SUCCESS == WSL305S_AT_GetATCMD(AT_CFS_GET, aucCmdData, AT_CMDLEN_MAX)) && 
        (strstr(aucCmdData, CFS_STATUS_OK) != NULL))
    {
        bConfirmed = true;
    }
    
    return bConfirmed;
}


/**
 *获取模组Fpending状态
 *@param[in]                 无
 *@retval       true         表示模组处于Fpending状态
 *@retval       false        表示模组未处于Fpending状态
*/
bool WSL305S_AT_IsServerCached(void)
{
    char aucCmdData[AT_CMDLEN_MAX] = {0};
    bool bFPending  = false;

    DEBUG_PRINT("WSL305S_AT_IsServerCached\r\n");
    if ((ERROR_SUCCESS == WSL305S_AT_GetATCMD(AT_FPS_GET, aucCmdData, AT_CMDLEN_MAX)) && 
        (strstr(aucCmdData, FPS_PENDING) != NULL))
    {
        bFPending = true;
    }
    
    return bFPending;
}

/**
 *获取最后接收包的信号质量
 *@param[in]    pusRSSI   返回获取到的RSSI值
 *@param[in]    pucSNR   返回获取到的SNR值
 *@retval       ERROR_SUCCESS         表示指令执行成功
 *@retval       ERROR_FAILED          表示指令执行失败
*/
uint8_t WSL305S_AT_GetPktQuality(uint16_t *pusRSSI, uint8_t *pucSNR)
{
    char aucCmdData[AT_CMDLEN_MAX] = {0};
    uint8_t ucRet  = ERROR_FAILED;
    uint16_t usRSSI = 0;
    uint8_t ucSNR = 0;

    DEBUG_PRINT("WSL305S_AT_GetPktQuality\r\n");
    if ((ERROR_SUCCESS == WSL305S_AT_GetATCMD(AT_RSSI_GET, aucCmdData, AT_CMDLEN_MAX)) && 
        (0 < sscanf(aucCmdData, "%hd", (int16_t *)&usRSSI)) &&
        (ERROR_SUCCESS == WSL305S_AT_GetATCMD(AT_SNR_GET, aucCmdData, AT_CMDLEN_MAX)) && 
        (0 < sscanf(aucCmdData, "%hhd", (int8_t *)&ucSNR)))
    {
        ucRet = ERROR_SUCCESS;
        *pusRSSI = usRSSI;
        *pucSNR = ucSNR;
    }
    
    return ucRet;
}


/**
 *AT指令设置数据速率
 *@param[in]    ucDR                   速率等级0~7
 *@retval       ERROR_SUCCESS         表示指令执行成功
 *@retval       ERROR_FAILED          表示指令执行失败
*/
uint8_t WSL305S_AT_SetDR(uint8_t ucDR)
{
    char ucValue = 0;

    ucValue = '0' + ucDR;
    return WSL305S_AT_SetATCMD(AT_DR_SET, &ucValue);
}


/**
 *AT指令设置接收窗口1延迟时间
 *@param[in]    ucRX1Delay             单位ms
 *@retval       ERROR_SUCCESS         表示指令执行成功
 *@retval       ERROR_FAILED          表示指令执行失败
*/
uint8_t WSL305S_AT_SetRX1Delay(uint8_t ucRX1Delay)
{
    char    szRX1Delay[8] = {0};

    g_ucRx1Delay = ucRX1Delay;

    sprintf(szRX1Delay, "%d", ucRX1Delay);
    return WSL305S_AT_SetATCMD(AT_RX1DL_SET, szRX1Delay);
}


/**
 *AT指令设置AppKey
 *@param[in]    pucValue               AppKey字符串指针
 *@retval       ERROR_SUCCESS         表示指令执行成功
 *@retval       ERROR_FAILED          表示指令执行失败
*/
uint8_t WSL305S_AT_SetAppKey(char *pucValue)
{
    char    szAppKey[HEX_LEN_16 * 3] = {0};

    wsl305s_Arr2Str(szAppKey, pucValue, HEX_LEN_16);

    DelayMs(200);
    return WSL305S_AT_SetATCMD(AT_APPKEY_SET, szAppKey);
}


/**
 *AT指令设置DevEUI
 *@param[in]    pucValue               DevEUI字符串指针
 *@retval       ERROR_SUCCESS         表示指令执行成功
 *@retval       ERROR_FAILED          表示指令执行失败
*/
uint8_t WSL305S_AT_SetDevEUI(char *pucValue)
{
    char    szDevEUI[HEX_LEN_8 * 3] = {0};
       
    wsl305s_Arr2Str(szDevEUI, pucValue, HEX_LEN_8); 

    DelayMs(400);
    return WSL305S_AT_SetATCMD(AT_DEUI_SET, szDevEUI);
}


/**
 *AT指令设置DevEUI
 *@param[in]    pucValue               DevEUI字符串指针
 *@retval       ERROR_SUCCESS         表示指令执行成功
 *@retval       ERROR_FAILED          表示指令执行失败
*/
uint8_t WSL305S_AT_SetAppEUI(char *pucValue)
{
    char    szAppEUI[HEX_LEN_8 * 3] = {0};
       
    wsl305s_Arr2Str(szAppEUI, pucValue, HEX_LEN_8); 

    DelayMs(300);
    return WSL305S_AT_SetATCMD(AT_APPEUI_SET, szAppEUI);
}


/**
 *AT指令设置NwksKey
 *@param[in]    pucValue               NwksKey字符串指针
 *@retval       ERROR_SUCCESS         表示指令执行成功
 *@retval       ERROR_FAILED          表示指令执行失败
*/
uint8_t WSL305S_AT_SetNwksKey(char *pucValue)
{
    char    szNwksKey[HEX_LEN_16 * 3] = {0};
       
    wsl305s_Arr2Str(szNwksKey, pucValue, HEX_LEN_16); 

    DelayMs(300);
    return WSL305S_AT_SetATCMD(AT_NWKSKEY_SET, szNwksKey);
}


/**
 *AT指令设置AppsKey
 *@param[in]    pucValue               AppsKey字符串指针
 *@retval       ERROR_SUCCESS         表示指令执行成功
 *@retval       ERROR_FAILED          表示指令执行失败
*/
uint8_t WSL305S_AT_SetAppsKey(char *pucValue)
{
    char    szAppsKey[HEX_LEN_16 * 3] = {0};

    wsl305s_Arr2Str(szAppsKey, pucValue, HEX_LEN_16);

    DelayMs(300);
    return WSL305S_AT_SetATCMD(AT_APPSKEY_SET, szAppsKey);
}


/**
 *AT指令设置DevAddr
 *@param[in]    uiDevAddr              DevAddr字符串指针
 *@retval       ERROR_SUCCESS         表示指令执行成功
 *@retval       ERROR_FAILED          表示指令执行失败
*/
uint8_t WSL305S_AT_SetDevAddr(uint32_t uiDevAddr)
{
    char aucAddr[16] = {0};

    snprintf(aucAddr, 16, "%02X:%02X:%02X:%02X", 
               (uint8_t)((uiDevAddr >> 24) & 0xFF), (uint8_t)((uiDevAddr >> 16) & 0xFF),
               (uint8_t)((uiDevAddr >> 8) & 0xFF), (uint8_t)(uiDevAddr & 0xFF));
    return WSL305S_AT_SetATCMD(AT_DADDR_SET, aucAddr);
}


/**
 *AT指令设置CFM模式
 *@param[in]    bConfirm               true为需要确认消息模式，false为不需要确认消息模式
 *@retval       ERROR_SUCCESS         表示指令执行成功
 *@retval       ERROR_FAILED          表示指令执行失败
*/
uint8_t WSL305S_AT_SetCFMMode(bool bConfirm)
{
    char *pcValue = "0";
    
    if (true == bConfirm)
    {
        pcValue = "1";
    }
    
    return WSL305S_AT_SetATCMD(AT_CFM_SET, pcValue);
}


/**
 *触发入网
 *@param[in]                         无
 *@retval       ERROR_SUCCESS       表示指令执行成功
 *@retval       ERROR_FAILED        表示指令执行失败
*/
uint8_t WSL305S_AT_Join(void)
{
    DEBUG_PRINT("WSL305S_AT_Join\r\n");
    
    /* 发送入网AT指令 */
    return WSL305S_AT_SetATCMD(AT_JOIN_RUN, NULL);
}


/**
 *从模组送到云端的报文数据发送
 *@param[in]    bConfirm               发送报文种类，true为Confirm,flase为Unconfirm
 *@param[in]    ucFPort                数据发送端口号
 *@param[in]    pData                  待发送的数据字符串指针
 *@param[in]    ucDataSize             待发送的数据字符串长度
 *@retval       ERROR_SUCCESS         表示指令执行成功
 *@retval       ERROR_FAILED          表示指令执行失败
*/
uint8_t WSL305S_AT_Send(bool bConfirm, 
                              uint8_t ucFPort, char *pData, uint8_t ucDataSize)
{
    uint8_t ucRet = ERROR_SUCCESS;
    char acData[WSL305S_MAX_SENDB_LEN + 1] = "";
    char acTemp[8] = "";
    uint16_t usLen = 0;

    if ((ucDataSize > WSL305S_MAX_DATA_LEN) || 
        (g_bJoined == false) || 
        (g_bUnconfirmSend == true) ||
        (g_bPktConfirmed == false))
    {
        return ERROR_FAILED;
    }

    if (bConfirm != g_bConfirm)
    {
        ucRet = WSL305S_AT_SetCFMMode(bConfirm);
        if (ucRet == ERROR_SUCCESS)
        {
            g_bConfirm = bConfirm;
        }
        Delay(1);  /* 设置报文模式后，延迟一段时间，否则模组发包容易失败 */
    }

    if (ucRet == ERROR_SUCCESS)
    {
        usLen = snprintf(acData, WSL305S_MAX_SENDB_LEN + 1, "%d:", ucFPort);
        for (int i = 0; i < ucDataSize; i++)
        {
            sprintf(acTemp, "%02X", pData[i]);
            memcpy(acData + usLen, acTemp, 2);
            usLen += 2;
        }
        acData[usLen] = 0;
        ucRet = wsl305s_SendData(acData);
        if (ucRet == ERROR_SUCCESS)
        {
            if (true == bConfirm)
            {
                g_bPktConfirmed = false;
                TimerSetValue( &g_stConfirmTimer, ((g_ucRx1Delay + 1 + 2) * 8) * 1000);
                TimerStart( &g_stConfirmTimer );
            }
            else
            {        
                g_bUnconfirmSend = true;
            }

            /* 发包后，等待模组空口发包 */
            DelayMs(500 + (ucDataSize / 10) * 200);
        }
    }

    return ucRet;
}


/**
 *获取云端下发到模组的数据
 *@param[in]    pcData               获取数据的缓存区指针
 *@param[in]    ucBufferLen          获取数据的缓存区长度
 *@param[in]    pcFPort              获取数据端口号
 *@retval       ucDataCopy           复制到缓冲区的数据长度
*/
uint8_t WSL305S_AT_GetData(char *pcData, uint8_t ucBufferLen, uint8_t *pcFPort)
{
    char aucCmdData[WSL305S_MAX_RX_LEN] = {0};
    char aucData[WSL305S_MAX_DATA_LEN] = {0};
    uint16_t usDatalen = 0;
    uint16_t usIndex = 0;
    unsigned int uiFPort = 0;
    unsigned int uiData = 0;
    uint8_t ucDataCopy = 0;
    char *pData = NULL;

    if (ERROR_SUCCESS == WSL305S_AT_GetATCMD(AT_RECVB_GET, aucCmdData, WSL305S_MAX_RX_LEN))
    {
        usDatalen = strlen(aucCmdData);
        if (0 < sscanf(aucCmdData, "%u:", &uiFPort))
        {
            pData = strstr(aucCmdData, ":");
            pData++;
            usIndex = pData - aucCmdData;
            if ((usDatalen - usIndex) % 2 == 0)
            {
                while (usDatalen - usIndex > 0)
                {
                    if ((true != wsl305s_IsHexFormat(aucCmdData[usIndex])) ||
                        (true != wsl305s_IsHexFormat(aucCmdData[usIndex+1])))
                    {
                        break;
                    }
                    sscanf(aucCmdData + usIndex, "%02x", &uiData);
                    aucData[ucDataCopy] = (char)uiData;
                    ucDataCopy++;
                    usIndex += 2;
                }

                if (usDatalen == usIndex)
                {
                    *pcFPort = (uint8_t)uiFPort;
                    ucDataCopy = (ucDataCopy > ucBufferLen) ? ucBufferLen : ucDataCopy;
                    memcpy(pcData, aucData, ucDataCopy);
//                    DelayMs(200);
                }
            }
        }       
    }
    return ucDataCopy;   
}


/**
 *获取从云端下发到模组的数据
 *@param[in]                 无                                             
*/
void WSL305S_AT_Running(void)
{
    bool bJoined = false;
    bool bPktConfirmed = false;
    bool bFPending = false;
    uint8_t ucFPort = 0;
    uint8_t ucRecvLen = 0;
    uint8_t ucIndex = 0;
    char auData[WSL305S_MAX_DATA_LEN] = {0};

    if (g_bJoined == false)
    {
        Delay(5 + 3); /* 等待模组收到join accept后去查询 */
        bJoined = WSL305S_AT_IsNetworkJoined();
        if (true == bJoined)
        {
            g_bJoined = true;
        }

        if (g_stHandler.Wsl305sMlmeConfirm != NULL)
        {
            g_stHandler.Wsl305sMlmeConfirm(bJoined);
        }
    }
    else
    {
        if (g_bUnconfirmSend == true)
        {
            /* 之前发送了unconfirm报文 */
            Delay(g_ucRx1Delay + 2);   /* 发完包后, 等待模组接收下行 */
            
            g_bUnconfirmSend = false;
            if (g_stHandler.Wsl305sMcpsConfirm != NULL)
            {
                g_stHandler.Wsl305sMcpsConfirm(false, ERROR_SUCCESS);
            }

            do
            {
                /* 查看模组是否接收到下行数据，查看云上是否有缓存 */
                DEBUG_PRINT("GetData try:%d\r\n",ucIndex);
                bFPending = WSL305S_AT_IsServerCached();
                ucRecvLen = WSL305S_AT_GetData(auData, WSL305S_MAX_DATA_LEN, &ucFPort);
                ucIndex++;  

                /* 模组和MCU可能存在时间上的误差，多次查询，每次查询间隔一段时间，提供健壮性 */
                Delay(2);  
            }while ((ucIndex < 3) && (ucRecvLen == 0));
        }
        else if ((g_bConfirm == true) && (g_bPktConfirmed == false))
        {
            /* 之前发送了confirm报文 */
            Delay(g_ucRx1Delay + 2);   /* 发完包后, 等待模组接收下行 */
            do
            {
                DEBUG_PRINT("GetData try:%d\r\n",ucIndex);
                bFPending = WSL305S_AT_IsServerCached();
                ucRecvLen = WSL305S_AT_GetData(auData, WSL305S_MAX_DATA_LEN, &ucFPort);
                ucIndex++; 

                /* 模组和MCU可能存在时间上的误差，多次查询，每次查询间隔一段时间，提供健壮性 */
                Delay(2);
                if (false == g_bPktConfirmed)
                {
                    bPktConfirmed = WSL305S_AT_IsPktConfirmed();
                    if (true == bPktConfirmed)
                    {
                        TimerStop( &g_stConfirmTimer );
                        g_bPktConfirmed = true;
                        if (g_stHandler.Wsl305sMcpsConfirm != NULL)
                        {
                            g_stHandler.Wsl305sMcpsConfirm(true, ERROR_SUCCESS);
                        }
                    }
                }
            }while ((ucIndex < 3) && (ucRecvLen == 0));
        }

        if ((bFPending == true) || (ucRecvLen > 0))
        {
            if (g_stHandler.Wsl305sMcpsIndication != NULL)
            {
                g_stHandler.Wsl305sMcpsIndication(bFPending, ucFPort, auData, ucRecvLen);
            }
        }
    }
    return;
}


/**
 *关闭数据收发串口
 *@param[in]                 无                                             
*/
void WSL305S_AT_CloseUart(void)
{
    UartClose(g_pstUartSerial);
    g_pstUartSerial = NULL;

    return;
}


/**
 *打开数据收发串口
 *@param[in]                 无                                             
*/
uint8_t WSL305S_AT_OpenUart(void)
{
    Uart_t *pstUart = NULL;
    uint8_t ucRet = ERROR_FAILED;

    pstUart = UartOpen(UART_SERIAL, RX_TX, 9600, UART_8_BIT, 
                          UART_1_STOP_BIT, NO_PARITY, NO_FLOW_CTRL);
    if (pstUart != NULL)
    {
        g_pstUartSerial = pstUart;
        ucRet = ERROR_SUCCESS;
    }

    return ucRet;
}


/**
 *Confirm报文超时定时器初始化
 *@param[in]           定时器超时事件处理回调函数指针                                                   
*/
void WSL305S_AT_Init(WSL305S_CALLBACK_S *pHandler)
{
    g_stHandler = *pHandler;

    TimerInit( &g_stConfirmTimer, wsl305s_OnConfirmTimerEvent );
    return;
}



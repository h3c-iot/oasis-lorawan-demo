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
#include "timer.h"

static Uart_t *g_pstUartSerial = NULL;

#define FQ_GRP_NUM                  8                           /* 每组信道的信道数 */
#define AT_CMDLEN_MAX               128                        /* AT指令发送缓存 */

/* KEY 定义 */                
#define ENTER_KEY                   0x0D                        /* ENTER按键 ASCII码*/
#define RETURN_KEY                  0x0A                        /* 回车按键 ASCII码*/

#define WSL305S_FQ_STR_LENTH        9                           /* 信道频率字符串长度 */
#define WSL305S_TXFQ_CMD_LEN        10
#define WSL305S_CN470_FREQ_START    470300000U       /* 0号信道频率 */
#define WSL305S_LORA_BANDWIDTH      200000U

#define WSL305S_MAX_DATA_LEN        242
#define WSL305S_MAX_ATCMD_LEN       20
#define WSL305S_MAX_SENDB_LEN       ((WSL305S_MAX_DATA_LEN * 2) + 4)
#define WSL305S_MAX_TX_LEN          (WSL305S_MAX_SENDB_LEN + WSL305S_MAX_ATCMD_LEN)
#define WSL305S_MAX_RX_LEN          ((WSL305S_MAX_DATA_LEN * 2) + 12)

#define HEX_LEN_16                  16
#define HEX_LEN_8                   8

bool g_bConfirm = false;
bool g_bPktConfirmed = true;
bool g_bUnconfirmSend = false;
bool g_bJoined = false;
uint8_t g_ucRx1Delay = 1;
WSL305S_CALLBACK_S g_stHandler = {NULL, };
static TimerEvent_t g_stConfirmTimer;


static void wsl305s_OnConfirmTimerEvent( void* context )
{
    TimerStop( &g_stConfirmTimer );

    g_bPktConfirmed = true;
    if (g_stHandler.Wsl305sMcpsConfirm != NULL)
    {
        g_stHandler.Wsl305sMcpsConfirm(true, AT_CMD_FAILUE);
    }

    return;
}

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

static uint8_t wsl305s_SendATCMD(const char *pucATCmd,
                                      char *pucData)
{

    char aucTxBuffer[WSL305S_MAX_TX_LEN];
    uint8_t ucTxIndex   = 0; 
    uint16_t usDataLen = 0;
    uint8_t ucRet = AT_CMD_SUCCESS;

    if ((pucATCmd == NULL) || (g_pstUartSerial == NULL))
    {
        return AT_CMD_FAILUE;
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
    printf("aucTxBuffer ==> %s",aucTxBuffer);
    if (0 != UartPutBuffer(g_pstUartSerial, (uint8_t *)aucTxBuffer, ucTxIndex))
    {
        ucRet = AT_CMD_FAILUE;
    }
    
    return ucRet;
}

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

/* 在特定端口上发送字符串数据 */
static uint8_t wsl305s_SendData(char *pucData)
{
    uint8_t ucRet = AT_CMD_FAILUE;
    
    /* 发送数据发送AT指令 */
    if (AT_CMD_SUCCESS == WSL305S_AT_SetATCMD(AT_SENDB_RUN, pucData))
    {
        ucRet = AT_CMD_SUCCESS;
    }

    return ucRet;
}

static void  wsl305s_Arr2Str(char *szArrData, char *pucPreArr, uint8_t uc8ArrLen)
{
    for (int i = 0; i < uc8ArrLen; i++)
    {
        sprintf(szArrData + i*3, "%02X:", pucPreArr[i]);
    }   
    
    szArrData[uc8ArrLen * 3 - 1] = 0;
    return;
}


/* 设置指令 */
uint8_t WSL305S_AT_SetATCMD(const char *pucATCMD ,char *pucValue)
{
    char aucCmdData[AT_CMDLEN_MAX + 1] = "";
    uint8_t ucRet  = AT_CMD_FAILUE;
    uint8_t ucRecvLen = 0;

    WSL305S_AT_OpenUart();
    DelayMs(10);

    /* 发送DevEUI设置指令 */
    if (AT_CMD_SUCCESS == wsl305s_SendATCMD(pucATCMD, pucValue))
    {
        DelayMs(100);
        
        ucRecvLen = wsl305s_RecvUart(aucCmdData, AT_CMDLEN_MAX);
        if (ucRecvLen != 0)
        {
            aucCmdData[ucRecvLen] = 0;
            if(strstr(aucCmdData, AT_STATUS_OK) != NULL)
            {
                printf("Send Ok, Data:[%s%s]\r\n", pucATCMD, ((pucValue == NULL) ? "" : pucValue));
                ucRet = AT_CMD_SUCCESS;
            }
            else
            {
                printf("Send failed, Receive Data <==\r\n%s\r\n", aucCmdData);
            }
        }
    }

    WSL305S_AT_CloseUart();

    return ucRet;
}

/* 查询参数指令 */
uint8_t WSL305S_AT_GetATCMD(const char *pucATCMD, char *pucRetValue, uint16_t usBuffLen)
{
    char aucCmdData[WSL305S_MAX_RX_LEN + 1] = {0};
    char *pucDataEnd = NULL;
    uint16_t usGetLen  = 0;
    uint16_t usRecvLen = 0;
    uint8_t ucRet = AT_CMD_FAILUE;

    WSL305S_AT_OpenUart();
    DelayMs(10);

    /* 发送DevEUI设置指令 */
    if (AT_CMD_SUCCESS == wsl305s_SendATCMD(pucATCMD, NULL))
    {
        DelayMs(200);
        
        usRecvLen = wsl305s_RecvUart(aucCmdData, WSL305S_MAX_RX_LEN);
        if (usRecvLen != 0)
        {
            aucCmdData[usRecvLen] = 0;
            if (strstr(aucCmdData, AT_STATUS_OK) != NULL)
            {
                printf("Send Ok, Data==>%s\r\nReceive Data<==\r\n%s\r\n", pucATCMD, aucCmdData);
                
                pucDataEnd = strstr(aucCmdData, "\r\n");
                if (pucDataEnd != NULL)
                {
                    usGetLen = pucDataEnd - aucCmdData;
                    usGetLen = (usGetLen > (usBuffLen-1)) ? (usBuffLen-1) : usGetLen;
                    if (usGetLen > 0)
                    {
                        memcpy(pucRetValue, aucCmdData, usGetLen);
                        *(pucRetValue + usGetLen) = '\0';
                        ucRet = AT_CMD_SUCCESS;
                    }
                }
            }
            else
            {
                printf("Send failed, Receive Data<==\r\n%s\r\n", aucCmdData);
            }
        }
    }

    WSL305S_AT_CloseUart();
    
    return ucRet;
}


/* 模组复位重启 */
void WSL305S_AT_ResetModule(void)
{
    printf("@ATZ\r\n");

    WSL305S_AT_OpenUart();
    
    /* 发送AT重启模组 */
    (void)wsl305s_SendATCMD(AT_MCU_RESET, NULL);

    DelayMs(100);
    WSL305S_AT_CloseUart();
    
    return;
}


/* 设置发送某信道组的1-8信道的频率，FqGrpNum为 1-12 */
uint8_t WSL305S_AT_SetChannelGroup(uint8_t ucChnlGrp)
{
    uint8_t  ucRet = AT_CMD_SUCCESS;
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

    printf("WSL305S_AT_SetChannelGroup\r\n");   

    auiLoraTxFrq[0] = WSL305S_CN470_FREQ_START + WSL305S_LORA_BANDWIDTH * FQ_GRP_NUM * (ucChnlGrp - 1); 
    for(int i = 0; i < 8; i++)
    {
        auiLoraTxFrq[i] = auiLoraTxFrq[0] + WSL305S_LORA_BANDWIDTH * i; 
        sprintf(aucLoraFrqGrp[i], "%ld", auiLoraTxFrq[i]);
//        FQ_NumtoStr(auiLoraTxFrq[i], (uint8_t *)(aucLoraFrqGrp[i]));
        if (AT_CMD_SUCCESS != WSL305S_AT_SetATCMD(aucATCmdTxFqGp[i], aucLoraFrqGrp[i]))
        {
            ucRet = AT_CMD_FAILUE;
            break;
        }
    }
    
    return ucRet;
}


/* 保存配置 */
uint8_t WSL305S_AT_SaveConfig(void)
{
    printf("WSL305S_AT_SaveConfig\r\n"); 
    
    /* 发送参数保存AT指令 */
    return WSL305S_AT_SetATCMD(AT_SAVE_RUN, NULL);
}

/* AT指令通信连接状态检查 */
uint8_t WSL305S_AT_CheckBoard(void)
{
    printf("WSL305S_AT_CheckBoard\r\n");
    
    /* 发送AT指令连接状态查询 */
    return WSL305S_AT_SetATCMD(AT_CONNECT_CHECK, NULL);
}

/* 判断是否入网成功 */
bool WSL305S_AT_IsNetworkJoined(void)
{
    char aucCmdData[AT_CMDLEN_MAX] = {0};
    bool bJoined  = false;

    printf("WSL305S_AT_IsNetworkJoined\r\n");
    if ((AT_CMD_SUCCESS == WSL305S_AT_GetATCMD(AT_NJM_GET, aucCmdData, AT_CMDLEN_MAX)) && 
        (strstr(aucCmdData, NJM_SET_ABP) != NULL))
    {
        bJoined = true;
    }
    
    /* 发送入网AT指令 */
    if ((bJoined == false) &&
        (AT_CMD_SUCCESS == WSL305S_AT_GetATCMD(AT_NJS_GET, aucCmdData, AT_CMDLEN_MAX)) &&
        (strstr(aucCmdData, JOIN_SUCCEED) != NULL))
    {
        bJoined = true;
    }
    
    return bJoined;
}

bool WSL305S_AT_IsPktConfirmed(void)
{
    char aucCmdData[AT_CMDLEN_MAX] = {0};
    bool bConfirmed  = false;

    printf("WSL305S_AT_IsPktConfirmed\r\n");
    if ((AT_CMD_SUCCESS != WSL305S_AT_GetATCMD(AT_CFS_GET, aucCmdData, AT_CMDLEN_MAX)) && 
        (strstr(aucCmdData, CFS_STATUS_OK) != NULL))
    {
        bConfirmed = true;
    }
    
    return bConfirmed;
}

bool WSL305S_AT_IsServerCached(void)
{
    char aucCmdData[AT_CMDLEN_MAX] = {0};
    bool bFPending  = false;

    printf("WSL305S_AT_IsServerCached\r\n");
    if ((AT_CMD_SUCCESS != WSL305S_AT_GetATCMD(AT_FPS_GET, aucCmdData, AT_CMDLEN_MAX)) && 
        (strstr(aucCmdData, FPS_PENDING) != NULL))
    {
        bFPending = true;
    }
    
    return bFPending;
}

uint8_t WSL305S_AT_SetDR(uint8_t ucDR)
{
    char ucValue = 0;

    ucValue = '0' + ucDR;
    return WSL305S_AT_SetATCMD(AT_DR_SET, &ucValue);
}

uint8_t WSL305S_AT_SetRX1Delay(uint8_t ucRX1Delay)
{
    char    szRX1Delay[8] = {0};

    g_ucRx1Delay = ucRX1Delay;

    sprintf(szRX1Delay, "%d", ucRX1Delay);
    return WSL305S_AT_SetATCMD(AT_RX1DL_SET, szRX1Delay);
}

uint8_t WSL305S_AT_SetAppKey(char *pucValue)
{
    char    szAppKey[HEX_LEN_16 * 3] = {0};

    wsl305s_Arr2Str(szAppKey, pucValue, HEX_LEN_16);

    DelayMs(200);
    return WSL305S_AT_SetATCMD(AT_APPKEY_SET, szAppKey);
}

uint8_t WSL305S_AT_SetDevEUI(char *pucValue)
{
    char    szDevEUI[HEX_LEN_8 * 3] = {0};
       
    wsl305s_Arr2Str(szDevEUI, pucValue, HEX_LEN_8); 

    DelayMs(300);
    return WSL305S_AT_SetATCMD(AT_DEUI_SET, szDevEUI);
}

uint8_t WSL305S_AT_SetAppEUI(char *pucValue)
{
    char    szAppEUI[HEX_LEN_8 * 3] = {0};
       
    wsl305s_Arr2Str(szAppEUI, pucValue, HEX_LEN_8); 

    DelayMs(300);
    return WSL305S_AT_SetATCMD(AT_APPEUI_SET, szAppEUI);
}

uint8_t WSL305S_AT_SetNwksKey(char *pucValue)
{
    char    szNwksKey[HEX_LEN_16 * 3] = {0};
       
    wsl305s_Arr2Str(szNwksKey, pucValue, HEX_LEN_16); 

    DelayMs(300);
    return WSL305S_AT_SetATCMD(AT_NWKSKEY_SET, szNwksKey);
}

uint8_t WSL305S_AT_SetAppsKey(char *pucValue)
{
    char    szAppsKey[HEX_LEN_16 * 3] = {0};

    wsl305s_Arr2Str(szAppsKey, pucValue, HEX_LEN_16);

    DelayMs(300);
    return WSL305S_AT_SetATCMD(AT_APPSKEY_SET, szAppsKey);
}

uint8_t WSL305S_AT_SetDevAddr(uint32_t uiDevAddr)
{
    char aucAddr[16] = {0};

    snprintf(aucAddr, 16, "%02X:%02X:%02X:%02X", 
               (uint8_t)((uiDevAddr >> 24) & 0xFF), (uint8_t)((uiDevAddr >> 16) & 0xFF),
               (uint8_t)((uiDevAddr >> 8) & 0xFF), (uint8_t)(uiDevAddr & 0xFF));
    return WSL305S_AT_SetATCMD(AT_DADDR_SET, aucAddr);
}

uint8_t WSL305S_AT_SetCFMMode(bool bConfirm)
{
    char *pcValue = "0";
    
    if (true == bConfirm)
    {
        pcValue = "1";
    }
    
    return WSL305S_AT_SetATCMD(AT_CFM_SET, pcValue);
}

/* 触发入网 */
uint8_t WSL305S_AT_Join(void)
{
    printf("WSL305S_AT_Join\r\n");
    
    /* 发送入网AT指令 */
    return WSL305S_AT_SetATCMD(AT_JOIN_RUN, NULL);
}

uint8_t WSL305S_AT_Send(bool bConfirm, 
                              uint8_t ucFPort, char *pData, uint8_t ucDataSize)
{
    uint8_t ucRet = AT_CMD_SUCCESS;
    char acData[WSL305S_MAX_SENDB_LEN + 1] = "";
    char acTemp[8] = "";
    uint16_t usLen = 0;

    if ((ucDataSize > WSL305S_MAX_DATA_LEN) || 
        (g_bJoined == false) || 
        (g_bUnconfirmSend == true) ||
        (g_bPktConfirmed == false))
    {
        return AT_CMD_FAILUE;
    }

    if (bConfirm != g_bConfirm)
    {
        ucRet = WSL305S_AT_SetCFMMode(bConfirm);
        if (ucRet == AT_CMD_SUCCESS)
        {
            g_bConfirm = bConfirm;
        }
    }

    if (ucRet == AT_CMD_SUCCESS)
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
        if (ucRet == AT_CMD_SUCCESS)
        {
            if (true == bConfirm)
            {
                g_bPktConfirmed = false;
                TimerSetValue( &g_stConfirmTimer, (g_ucRx1Delay + 1 + 2) * 8);
                TimerStart( &g_stConfirmTimer );
            }
            else
            {
                g_bUnconfirmSend = true;
            }
        }
    }

    return ucRet;
}

uint8_t WSL305S_AT_GetData(char *pcData, uint8_t ucBufferLen, uint8_t *pcFPort)
{
    char aucCmdData[WSL305S_MAX_RX_LEN] = {0};
    char aucData[WSL305S_MAX_DATA_LEN] = {0};
    uint16_t usDatalen = 0;
    uint16_t usIndex = 0;
    unsigned int uiFPort = 0;
    uint8_t ucDataCopy = 0;
    char *pData = NULL;

    if (AT_CMD_SUCCESS == WSL305S_AT_GetATCMD(AT_RECVB_GET, aucCmdData, WSL305S_MAX_RX_LEN))
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
                    sscanf(aucCmdData + usIndex, "%hhx", (unsigned char *)&aucData[ucDataCopy]);
                    ucDataCopy++;
                    usIndex += 2;
                }

                if (usDatalen == usIndex)
                {
                    *pcFPort = (uint8_t)uiFPort;
                    ucDataCopy = (ucDataCopy > ucBufferLen) ? ucBufferLen : ucDataCopy;
                    memcpy(pcData, aucData, ucDataCopy);
                }
            }
        }
    }
    
    return ucDataCopy;
}

void WSL305S_AT_Running(void)
{
    bool bJoined = false;
    bool bPktConfirmed = false;
    bool bFPending = false;
    uint8_t ucFPort = 0;
    uint8_t ucRecvLen = 0;
    char auData[WSL305S_MAX_DATA_LEN] = {0};

    if (g_bJoined == false)
    {
        Delay(5 + 1);
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
        Delay(g_ucRx1Delay + 1);
        if (g_bUnconfirmSend == true)
        {
            g_bUnconfirmSend = false;
            if (g_stHandler.Wsl305sMcpsConfirm != NULL)
            {
                g_stHandler.Wsl305sMcpsConfirm(false, AT_CMD_SUCCESS);
            }
            bFPending = WSL305S_AT_IsServerCached();
            ucRecvLen = WSL305S_AT_GetData(auData, WSL305S_MAX_DATA_LEN, &ucFPort);
        }
        else if ((g_bConfirm == true) && (g_bPktConfirmed == false))
        {
            bPktConfirmed = WSL305S_AT_IsPktConfirmed();
            if (true == bPktConfirmed)
            {
                TimerStop( &g_stConfirmTimer );
                g_bPktConfirmed = true;
                if (g_stHandler.Wsl305sMcpsConfirm != NULL)
                {
                    g_stHandler.Wsl305sMcpsConfirm(true, AT_CMD_SUCCESS);
                }
            }
            bFPending = WSL305S_AT_IsServerCached();
            ucRecvLen = WSL305S_AT_GetData(auData, WSL305S_MAX_DATA_LEN, &ucFPort);
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

void WSL305S_AT_CloseUart(void)
{
    UartClose(g_pstUartSerial);
    g_pstUartSerial = NULL;

    return;
}

uint8_t WSL305S_AT_OpenUart(void)
{
    Uart_t *pstUart = NULL;
    uint8_t ucRet = AT_CMD_FAILUE;

    pstUart = UartOpen(UART_SERIAL, RX_TX, 9600, UART_8_BIT, 
                          UART_1_STOP_BIT, NO_PARITY, NO_FLOW_CTRL);
    if (pstUart != NULL)
    {
        g_pstUartSerial = pstUart;
        ucRet = AT_CMD_SUCCESS;
    }

    return ucRet;
}

void WSL305S_AT_Init(WSL305S_CALLBACK_S *pHandler)
{
    g_stHandler = *pHandler;

    TimerInit( &g_stConfirmTimer, wsl305s_OnConfirmTimerEvent );
    return;
}



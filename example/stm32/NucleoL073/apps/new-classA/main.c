/**
* @file        main.c
*
* @copyright   Copyright (C) 2019, New H3C Technologies Co., Ltd.            
*              License: Revised BSD License 
*              ,see LICENSE.TXT file included in the project.
*
* @brief       main new Class A example
*                                                                                   
* @code        
*                          Author : wangzhen
*
*                          Project Code: example
*
*                          Module Name: main    
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
#include "utilities.h"
#include "board.h"
#include "gpio.h"
#include "delay.h"
#include "Commissioning.h"
#include "NvmCtxMgmt.h"
#include "ladapter.h"

/**
 * 发送应用数据的周期 10000 单位:ms
 */
#define APP_TX_DUTYCYCLE                            60000

#define THIS_IS(para)   1


/*!
 * LoRaWAN application port
 */
#define LORAWAN_APP_PORT128                            128

#if( ABP_ACTIVATION_LRWAN_VERSION == ABP_ACTIVATION_LRWAN_VERSION_V10x )
static uint8_t GenAppKey[] = LORAWAN_GEN_APP_KEY;
#else
static uint8_t AppKey[] = LORAWAN_APP_KEY;
#endif

/**
 * 网络根密钥
 */
static uint8_t NwkKey[] = LORAWAN_NWK_KEY;

#if( OVER_THE_AIR_ACTIVATION == 0 )

/**
 * 转发网络会话完整性密钥
 */
static uint8_t FNwkSIntKey[] = LORAWAN_F_NWK_S_INT_KEY;

/**
 * 服务网络会话完整性密钥
 */
static uint8_t SNwkSIntKey[] = LORAWAN_S_NWK_S_INT_KEY;

/**
 * 网络会话加密密钥
 */
static uint8_t NwkSEncKey[] = LORAWAN_NWK_S_ENC_KEY;

/**
 * 应用会话密钥
 */
static uint8_t AppSKey[] = LORAWAN_APP_S_KEY;

/**
 * 设备地址
 */
static uint32_t DevAddr = LORAWAN_DEVICE_ADDRESS;

#endif


/**
 * 定期发送应用数据的定时器
 */
static TimerEvent_t TxNextPacketTimer;

/**
 * 表示是否需要上行新的报文
 */
static bool NextTx = true;

/**
 * 表示是否入网
 */
static bool g_bJoined = false;

/**
 * 表示是否处于报文发送,接收，或者Mac处理周期内
 */
static uint8_t g_bSending = false;

/**
 * 表示发送是否需要确认的报文
 */
static bool g_bConfirm = false;

 /**
 * @brief   以hex格式打印输入的缓存.
 *
 * @param   [输入] unsigned char*类型的指针,存放打印的信息.
 *
 * @param   [输入] uinsined char类型的打印的长度
 *
 */
static void PrintHexBuffer( uint8_t *buffer, uint8_t size )
{
    uint8_t newline = 0;

    for( uint8_t i = 0; i < size; i++ )
    {
        if( newline != 0 )
        {
            printf( "\r\n" );
            newline = 0;
        }

        printf( "%02X ", buffer[i] );

        if( ( ( i + 1 ) % 16 ) == 0 )
        {
            newline = 1;
        }
    }
    printf( "\r\n" );
}

/*************************** Join Function**************************/
#if THIS_IS("LORA_JOIN")
typedef void ( *LORA_JOINProcess_PF )( bool bJoined);

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
LORA_JOINProcess_PF g_pfJoinProcess = NULL;

/**
* @brief   开始入网流程.
*
* @param   [输入] uinsined char类型的入网开始探测的信道组
*
* @param   [输入] LORA_JOINProcess_PF类型的指针,存放入网处理函数的地址.
*
*/
static void LoRaJoinStart(uint8_t ucChnlGrp, LORA_JOINProcess_PF pfLoRaJoinProcess)
{
    if (g_bJoining == true)
    {
        return;
    }

    printf("Start Join survey\r\n");
    (void)LADAPTER_SetJoinChannelGroup(ucChnlGrp);
    g_ucGrp = ucChnlGrp;
    g_ucChannelRetry = 0;
    g_pfJoinProcess = pfLoRaJoinProcess;
   

#if defined( LORA_MODULE_WSL30X )
    LADAPTER_SetNetwork(LADAPTER_NWKTYPE_OTAA);
#endif

    if (LADAPTER_SUCCES == LADAPTER_Join())
    {
        g_bJoining = true;
    }

    return;
}

/**
* @brief   开始入网流程.
*
* @param   [输入] uinsined char类型的入网结果
*          ,0表示入网成功
*          ,非0表示入网失败
*/
static void LoRaJoinChannelSurvey(uint8_t ucJoinResult)
{
    bool bReJoin = false;
    
    if( ucJoinResult == LADAPTER_SUCCES)
    {
        printf("Join success, group[%d]\r\n", g_ucGrp);
        LADAPTER_SetWorkChannelGroup(g_ucGrp);
        g_bJoining = false;
        if (g_pfJoinProcess != NULL)
        {
            g_pfJoinProcess(true);
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
            printf("Join retry, group[%d], channel retry[%d], group retry[%d]\r\n", 
                      g_ucGrp, g_ucChannelRetry, g_ucGrpRetry);
#if !defined( LORA_MODULE_WSL30X )
            (void)LADAPTER_Join( );
#endif 
            g_bJoining = true;
        }
        else
        {
            g_bJoining = false;
            if (g_pfJoinProcess != NULL)
            {
                g_pfJoinProcess(false);
            }
        }
    }

    return;
}


/*************************** Join END**************************/
#endif

#if THIS_IS("LORA_SEND")

#define LORA_MAX_CACHE_LEN   10
#define LORA_MAX_DATA_LEN    255

typedef struct tagPKTNode
{
    uint8_t ucFPort;
    bool bConfirm;
    uint8_t ucLen;
    char aucData[LORA_MAX_DATA_LEN];
}PKT_NODE_S;

/**
* Cache数组结构体进出
*/
typedef struct tagPKTCache
{
    uint8_t ucIn;
    uint8_t ucOut;
    uint8_t ucCnt;
    PKT_NODE_S astDataList[LORA_MAX_CACHE_LEN];
}PKT_CACHE_S;

/**
* 存放发送报文的数组结构体
*/
static PKT_CACHE_S g_stPktCache = {0};

/**
* 发送Cache数组待确认的报文.
*/
static void LoRaTriggerSend(void)
{
    PKT_NODE_S *pstNode = NULL;

    if ((g_bSending == false) && (g_stPktCache.ucCnt != 0))
    {
        pstNode = &g_stPktCache.astDataList[g_stPktCache.ucOut];
        printf("Trigger send packet, index[%d], bConfirm[%d], FPort[%d]\r\n", 
                        g_stPktCache.ucOut, pstNode->bConfirm, pstNode->ucFPort);
        if (NULL != LADAPTER_Send((LADAPTER_PKTSTYPE_E)pstNode->bConfirm, pstNode->ucFPort, 
                                            pstNode->aucData, pstNode->ucLen))
        {
            printf("Send success, index[%d], bConfirm[%d], FPort[%d]\r\n", 
                        g_stPktCache.ucOut, pstNode->bConfirm, pstNode->ucFPort);
            g_bSending = true;
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
* @param   [输入] unsigned char 类型的发送端口.
*
* @param   [输入] char* 类型的指针,指向发送数据的缓存.
*
* @param   [输入] unsigned char 类型的发送数据长度.
*
* @return  char类型ucIndex 表示缓存在Cache数组中发送信息的索引
*
*/
static int8_t LoRaSendPkt(bool bConfirm, uint8_t ucFPort, char *pcData, uint8_t ucLen)
{
    PKT_NODE_S *pstNode = NULL;
    uint8_t ucIndex = 0;
    
    if (g_stPktCache.ucCnt == LORA_MAX_CACHE_LEN)
    {
        /* buffer is full */
        return -1;
    }

    pstNode = &g_stPktCache.astDataList[g_stPktCache.ucIn];
    pstNode->bConfirm = bConfirm;
    pstNode->ucLen = ucLen;
    pstNode->ucFPort = ucFPort;
    memcpy(pstNode->aucData, pcData, ucLen);
    
    LoRaTriggerSend();

    ucIndex = g_stPktCache.ucIn;
    g_stPktCache.ucIn = (g_stPktCache.ucIn + 1) % LORA_MAX_CACHE_LEN;
    g_stPktCache.ucCnt++;

    return ucIndex;
}

/**
* @brief   应用Fpendding处理函数.
*
* @param   [输入]   bool类型变量
*          ,bFPending = true 表示下行报文Fpending标记位为1 
*          ,bFPending = false 表示下行报文Fpending标记位为0.
*
*/
static void LoRaReceiveFPending(bool bFPending)
{
    char aucData[8] = {0x00, };
    
    if (g_stPktCache.ucCnt == 0)
    {
        printf("Receive FPending, Send pull packet\r\n");
        LoRaSendPkt(false, 3, aucData, 1);
    }
    
    return;
}

/**
* @brief   应用Mcps确认处理函数.
*
* @param   [输入]   bool类型变量
*          ,bConfirmePkt = true 表示发送的需要确认的报文 
*          ,bConfirmePkt = false 表示发送的是不需要确认的报文.
*
* @param   [输入]   unsigned char类型的确认处理结果
*          ,ucResult = LADAPTER_SUCCES 表示确认处理成功
*          ,ucResult = LADAPTER_FAILED 表示确认处理失败
*
*/
static void LoRaMcpsConfirm(uint8_t ucResult)
{
    PKT_NODE_S *pstNode = NULL;
    
    if ((ucResult == LADAPTER_SUCCES) && (g_stPktCache.ucCnt != 0))
    {
        pstNode = &g_stPktCache.astDataList[g_stPktCache.ucOut];
        
        printf("Receive packet confirm, index[%d], bConfirm[%d], FPort[%d]\r\n", 
                     g_stPktCache.ucOut, pstNode->bConfirm, pstNode->ucFPort);
        memset(&g_stPktCache.astDataList[g_stPktCache.ucOut], 0, sizeof(PKT_NODE_S));
        g_stPktCache.ucOut = (g_stPktCache.ucOut + 1) % LORA_MAX_CACHE_LEN;
        g_stPktCache.ucCnt--;
    }

    g_bSending = false;

    return;
}

#endif
/** @brief   设置网络配置,设置入网方式
 *           ,入网的网络参数配置。             
 */
static void SetNetConfig(void)
{
    uint8_t devEui[] = LORAWAN_DEVICE_EUI;
    uint8_t joinEui[] = LORAWAN_JOIN_EUI;

    /** Try to restore from NVM and query the mac if possible.*/
    if( NvmCtxMgmtRestore( ) == NVMCTXMGMT_STATUS_SUCCESS )
    {
        printf( "\r\n###### ===== CTXS RESTORED ==== ######\r\n\r\n" );
    }
    else
    {
#if( OVER_THE_AIR_ACTIVATION == 0 )
        /** Tell the MAC layer which network server version are we connecting too.*/
        (void)LADAPTER_SetABPVersion(ABP_ACTIVATION_LRWAN_VERSION);
#endif

#if( ABP_ACTIVATION_LRWAN_VERSION == ABP_ACTIVATION_LRWAN_VERSION_V10x )
        (void)LADAPTER_SetAPPKey(GenAppKey);
#else
        (void)LADAPTER_SetAPPKey(AppKey);
#endif
        (void)LADAPTER_SetNWKKey(NwkKey);

        /** Initialize LoRaMac device unique ID if not already defined in Commissioning.h*/
        if( ( devEui[0] == 0 ) && ( devEui[1] == 0 ) &&
            ( devEui[2] == 0 ) && ( devEui[3] == 0 ) &&
            ( devEui[4] == 0 ) && ( devEui[5] == 0 ) &&
            ( devEui[6] == 0 ) && ( devEui[7] == 0 ) )
        {
            BoardGetUniqueId( devEui );
        }
        (void)LADAPTER_SetDevEUI(devEui);
        (void)LADAPTER_SetJoinEUI(joinEui);

#if( OVER_THE_AIR_ACTIVATION == 0 )
        /** Choose a random device address if not already defined in Commissioning.h */
        if( DevAddr == 0 )
        {
            /** Random seed initialization */
            srand1( BoardGetRandomSeed( ) );

            /** Choose a random device address */
            DevAddr = randr( 0, 0x01FFFFFF );
        }
        (void)LADAPTER_SetNetID(LORAWAN_NETWORK_ID);
        (void)LADAPTER_SetDevAddr(DevAddr);
        (void)LADAPTER_SetFNWKKey(FNwkSIntKey);
        (void)LADAPTER_SetSNWKKey(SNwkSIntKey);
        (void)LADAPTER_SetNWKSKey(NwkSEncKey);
        (void)LADAPTER_SetAPPSKey(AppSKey);
#endif
        LADAPTER_SetClassMode(LADAPTER_CLASS_A);
        LADAPTER_SetADR(false);
        LADAPTER_SetDR(LADAPTER_DR_0);
        LADAPTER_SetTxPower(LADAPTER_PWRLEVEL_0);
        LADAPTER_SetRX1Delay(5);
    }
    return;
}


/**
* @brief   设置应用端口为128下行数据处理函数.
*
* @param   [输入]   char *类型指针，指向数据的缓存.
*
* @param   [输入]   unsigned char类型的数据长度.
*
*/
static void processPort128FRMPkt( char *pcData, uint8_t ucDataLen)
{
    printf( "RX PORT     : %d\r\n", LORAWAN_APP_PORT128 );

    if( ucDataLen != 0 )
    {
        printf( "RX DATA     : \r\n" );
        PrintHexBuffer( (uint8_t *)pcData, ucDataLen );
    }

    if (ucDataLen > 0)
    {
        switch (*pcData)
        {
            case 0x00:
                printf(" g_bConfirm = false\r\n");
                g_bConfirm = false;
                break;
            case 0x01:
                printf(" g_bConfirm = true\r\n");
                g_bConfirm = true;
                break;
            case 0x02:
                printf("Echo data\r\n");
                Delay(2);   /* 使用模组时，发送太快会失败*/
                LoRaSendPkt(g_bConfirm, LORAWAN_APP_PORT128, pcData+1, ucDataLen-1);
                break;
            default:
                break;
        }
    }
    return;
}

/**
* @brief   入网状态处理函数.
*
* @param   [输入]   bool类型变量.
*          ,bJoined = true 表示入网成功 
*          ,bJoined = false 表示入网失败.
*
*/
static void OnJoinProcess( bool bJoined)
{
    g_bJoined = bJoined;
}

/**
* @brief   超时发包事件,用于周期发送报文的定时器事件.
*
*/
static void OnTxNextPktTimerEvent( void* context )
{    
    char aucData[8] = {0x00, };
    
    TimerStop( &TxNextPacketTimer );

    if (true == g_bJoined)
    {
        if (g_stPktCache.ucCnt == 0)
        {
            printf("Next tx timeout, Send pull packet\r\n");
            LoRaSendPkt(false, 3, aucData, 1);
        }
    }
    else
    {
        LoRaJoinStart(1, OnJoinProcess);
    }

    NextTx = true;
    
    return;
}


/**
 * Main application entry point.
 */
int main( void )
{
    LADAPTER_HANDLER_S stHandler;
    LADAPTER_PKTHANDLER_S stPktHandler = {0};

    /** Board Init */
    BoardInitMcu( );
    BoardInitPeriph( );
    
    DelayMs(5000);
    
    /** Handler CallBack Init */
    stHandler.pfLAdapter_JoinServer = LoRaJoinChannelSurvey;
    stHandler.pfLAdapter_ReceiveFPending = LoRaReceiveFPending;
    stHandler.pfLAdapter_BoardGetBatteryLevel = NULL;
    stHandler.pfLAdapter_OnMacProcessNotify = NULL;
    stHandler.pfLAdapter_GetTemperatureLevel = NULL;
    LADAPTER_Init(&stHandler, 1);

    /** Net Config Set */
    SetNetConfig();

    stPktHandler.pfLAdapter_ProcFRMPktReceive = processPort128FRMPkt;
    stPktHandler.pfLAdapter_ProcMcpsConfirm = LoRaMcpsConfirm;
    LADAPTER_RegisterFRMPktProc(LORAWAN_APP_PORT128, &stPktHandler);

    stPktHandler.pfLAdapter_ProcFRMPktReceive = NULL;
    stPktHandler.pfLAdapter_ProcMcpsConfirm = LoRaMcpsConfirm;
    LADAPTER_RegisterFRMPktProc(3, &stPktHandler);

    /** Tx Timer Init */
    TimerInit( &TxNextPacketTimer, OnTxNextPktTimerEvent );

#if( OVER_THE_AIR_ACTIVATION == 0 ) /* ABP */
    LADAPTER_SetNetwork(LADAPTER_NWKTYPE_ABP);
    g_bJoined = true;
#else
    /** OTAA LoRa Net Start */
    LoRaJoinStart(1, OnJoinProcess);
#endif

    while( 1 )
    {
        /** Process Radio IRQ */
        if( Radio.IrqProcess != NULL )
        {
            Radio.IrqProcess( );
        }
        /** Processes the LoRaMac events */
        LADAPTER_Running();

        if( NextTx == true)
        {
            /** Schedule next packet transmission */
            TimerSetValue( &TxNextPacketTimer, APP_TX_DUTYCYCLE );
            TimerStart( &TxNextPacketTimer );
            NextTx = false;
        }

        if (g_bJoined == true)
        {
            LoRaTriggerSend();
        }
    }
}

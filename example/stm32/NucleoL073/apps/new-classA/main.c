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
#include "base.h"
#include "board.h"
#include "gpio.h"
#include "delay.h"
#include "Commissioning.h"
#include "NvmCtxMgmt.h"
#include "ladapter.h"
#include "oasisnet.h"
#include "oasis.h"


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
 * 表示是否入网
 */
static bool g_bJoined = false;

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
 * 表示是否处于报文发送,接收，或者Mac处理周期内
 */
static uint8_t g_bSending = false;


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
        if (OASIS_NETPKT_HANDLER_INVALID != OASISNET_SendPkt((OASIS_NETPKT_TYPE_E)pstNode->bConfirm, pstNode->ucFPort, 
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
* @brief   应用Mcps确认处理函数.
*
* @param   [输入]   LADAPTER_PKTHANDLER pPktHandler 指向报文句柄
*
* @param   [输入]   unsigned char类型的确认处理结果
*          ,ucResult = ERROR_SUCCESS 表示确认处理成功
*          ,ucResult = ERROR_FAILED 表示确认处理失败
*
*/
static void LoRaMcpsConfirm(OASIS_NETPKT_HANDLER hPktHandler, uint8_t ucResult)
{
    PKT_NODE_S *pstNode = NULL;
    
    if ((ucResult == ERROR_SUCCESS) && (g_stPktCache.ucCnt != 0))
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
        LADAPTER_SetDR(LADAPTER_DR_1);
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
static void processPort128FRMPkt(char *pcData, uint8_t ucDataLen)
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
    if (bJoined == true)
    {
        OASIS_SendConfigSYNCReq();
        OASIS_StartAliveTimer();
    }
    else
    {
        OASIS_StopAliveTimer();
        OASISNET_Join();
    }
    g_bJoined = bJoined;
    
    return;
}


/**
 * Main application entry point.
 */
int main( void )
{
    OASISNET_HANDLER_S stHandler;
    OASISNET_PKT_HANDLER_S stPktHandler = {0};
    OASIS_HANDLER_S stOasisHandler;
    
    /** Board Init */
    BoardInitMcu( );
    BoardInitPeriph( );

    printf("new-classa start.\r\n");
    DelayMs(5000);
    
    /** Handler CallBack Init */
    stHandler.pfOasisNet_BoardGetBatteryLevel = NULL;
    stHandler.pfOasisNet_GetTemperatureLevel = NULL;
    stHandler.pfOasisNet_JOINProcess = OnJoinProcess;
    OASISNET_Init(&stHandler, 1);

    stOasisHandler.pfOasis_ResetModule = NULL;
    OASIS_Init(&stOasisHandler);

    /** Net Config Set */
    SetNetConfig();

    stPktHandler.pfOasis_ProcFRMPktReceive = processPort128FRMPkt;
    stPktHandler.pfOasis_ProcMcpsConfirm = LoRaMcpsConfirm;
    OASISNET_RegisterFRMPktProc(LORAWAN_APP_PORT128, &stPktHandler);

#if( OVER_THE_AIR_ACTIVATION == 0 ) /* ABP */
    LADAPTER_SetNetwork(LADAPTER_NWKTYPE_ABP);
    g_bJoined = true;
#else
    /** OTAA LoRa Net Start */
#if defined( LORA_MODULE_WSL30X )
    LADAPTER_SetNetwork(LADAPTER_NWKTYPE_OTAA);
#endif
    OASISNET_Join();
#endif

    while( 1 )
    {
        /** Process Radio IRQ */
        if( Radio.IrqProcess != NULL )
        {
            Radio.IrqProcess( );
        }
        /** Processes the LoRaMac events */
        OASISNET_Run();

        if (g_bJoined == true)
        {
            LoRaTriggerSend();
        }
    }
}

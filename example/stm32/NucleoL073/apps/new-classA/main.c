/*******************************************************************************
*                                                                              *
*                Copyright (C) 2019, New H3C Technologies Co., Ltd.            *
*  License: Revised BSD License, see LICENSE.TXT file included in the project  *
*                                                                              *
*------------------------------------------------------------------------------*
*                                                                              *
*                              main.c                                        *
*                                                                              *
*  Project Code: oasis-sdk                                                     *
*   Module Name: main                                                          *
*  Date Created: 2019-10-24                                                    *
*        Author: yys1819                                                       *
*   Description: main example                                                  *
*                                                                              *
*------------------------------------------------------------------------------*
*  Modification History                                                        *
*  DATE        NAME             DESCRIPTION                                    *
*  ----------------------------------------------------------------------------*
*  YYYY-MM-DD                                                                  *
*                                                                              *
*******************************************************************************/


#include <stdio.h>
#include "utilities.h"
#include "board.h"
#include "gpio.h"
#include "delay.h"
#include "Commissioning.h"
#include "NvmCtxMgmt.h"
#include "ladapter.h"

/*!
 * Defines the application data transmission duty cycle. 5s, value in [ms].
 */
#define APP_TX_DUTYCYCLE                            5000


/*!
 * LoRaWAN application port
 */
#define LORAWAN_APP_PORT128                            128

#if( ABP_ACTIVATION_LRWAN_VERSION == ABP_ACTIVATION_LRWAN_VERSION_V10x )
static uint8_t GenAppKey[] = LORAWAN_GEN_APP_KEY;
#else
static uint8_t AppKey[] = LORAWAN_APP_KEY;
#endif
static uint8_t NwkKey[] = LORAWAN_NWK_KEY;

#if( OVER_THE_AIR_ACTIVATION == 0 )

static uint8_t FNwkSIntKey[] = LORAWAN_F_NWK_S_INT_KEY;
static uint8_t SNwkSIntKey[] = LORAWAN_S_NWK_S_INT_KEY;
static uint8_t NwkSEncKey[] = LORAWAN_NWK_S_ENC_KEY;
static uint8_t AppSKey[] = LORAWAN_APP_S_KEY;

/*!
 * Device address
 */
static uint32_t DevAddr = LORAWAN_DEVICE_ADDRESS;

#endif


/*!
 * Timer to handle the application data transmission duty cycle
 */
static TimerEvent_t TxNextPacketTimer;

/*!
 * Indicates if a new packet can be sent
 */
static bool NextTx = true;
static bool g_bJoining = false;

static uint8_t g_ucGrp = 1;
static uint8_t g_ucChannelRetry = 0;
static uint8_t g_ucGrpRetry = 0;

/*!
 * Prints the provided buffer in HEX
 * 
 * \param buffer Buffer to be printed
 * \param size   Buffer size to be printed
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



/***************************Function**************************/

static void LoRaJoinStart(uint8_t ucChnlGrp)
{
    if (g_bJoining == true)
    {
        return;
    }
    
    (void)LADAPTER_SetJoinChannelGroup(ucChnlGrp);
    g_ucGrp = ucChnlGrp;
    g_ucChannelRetry = 0;
    
#if( OVER_THE_AIR_ACTIVATION == 0 ) /* ABP */
    LADAPTER_SetNetwork(LADAPTER_NWKTYPE_ABP);
#else /* OTAA */

#if defined( LORA_MODULE_WSL30X )
    LADAPTER_SetNetwork(LADAPTER_NWKTYPE_OTAA);
#endif

    if (LADAPTER_SUCCES == LADAPTER_Join())
    {
        g_bJoining = true;
    }
    
#endif

    return;
}

static void LoRaJoinChannelSurvey(uint8_t ucJoinResult)
{
    bool bReJoin = false;
    
    if( ucJoinResult == LADAPTER_SUCCES)
    {
        LADAPTER_SetWorkChannelGroup(g_ucGrp);
        g_bJoining = false;
    }
    else
    {
        // Join was not successful. Try to join again
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
#if !defined( LORA_MODULE_WSL30X )
            (void)LADAPTER_Join( );
#endif 
            g_bJoining = true;          
        }
        else
        {
            g_bJoining = false;
        }
    }

    return;
}


static void SetNetConfig(void)
{
    uint8_t devEui[] = LORAWAN_DEVICE_EUI;
    uint8_t joinEui[] = LORAWAN_JOIN_EUI;

    // Try to restore from NVM and query the mac if possible.
    if( NvmCtxMgmtRestore( ) == NVMCTXMGMT_STATUS_SUCCESS )
    {
        printf( "\r\n###### ===== CTXS RESTORED ==== ######\r\n\r\n" );
    }
    else
    {
#if( OVER_THE_AIR_ACTIVATION == 0 )
        // Tell the MAC layer which network server version are we connecting too.
        (void)LADAPTER_SetABPVersion(ABP_ACTIVATION_LRWAN_VERSION);
#endif

#if( ABP_ACTIVATION_LRWAN_VERSION == ABP_ACTIVATION_LRWAN_VERSION_V10x )
        (void)LADAPTER_SetAPPKey(GenAppKey);
#else
        (void)LADAPTER_SetAPPKey(AppKey);
#endif
        (void)LADAPTER_SetNWKKey(NwkKey);

        // Initialize LoRaMac device unique ID if not already defined in Commissioning.h
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
        // Choose a random device address if not already defined in Commissioning.h
        if( DevAddr == 0 )
        {
            // Random seed initialization
            srand1( BoardGetRandomSeed( ) );

            // Choose a random device address
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

/*!
 * \brief Function executed on TxNextPacket Timeout event
 */
static void OnTxNextPktTimerEvent( void* context )
{    
    TimerStop( &TxNextPacketTimer );

    if (true != LADAPTER_IsNetworkJoined())
    {
        // Network not joined yet. Try to join again
        LoRaJoinStart(1);
    }
    else
    {
        LADAPTER_Send(MCPS_UNCONFIRMED, LORAWAN_APP_PORT128, "123", 1);
        NextTx = true;
    }
    return;
}

static void processPort128FRMPkt( char *pcData, uint8_t ucDataLen, bool bFPending )
{
    printf( "RX PORT     : %d\r\n", LORAWAN_APP_PORT128 );

    if( ucDataLen != 0 )
    {
        printf( "RX DATA     : \r\n" );
        PrintHexBuffer( (uint8_t *)pcData, ucDataLen );
    }
    printf( "FPending    : %d\r\n", bFPending );
}

/**
 * Main application entry point.
 */
int main( void )
{
    LADAPTER_HANDLER_S stHandler;

    BoardInitMcu( );
    BoardInitPeriph( );
    
    DelayMs(5000);
    // Handler CallBack Init
    stHandler.pfLAdapter_JoinServer = LoRaJoinChannelSurvey;
    stHandler.pfLAdapter_BoardGetBatteryLevel = NULL;
    stHandler.pfLAdapter_OnMacProcessNotify = NULL;
    stHandler.pfLAdapter_GetTemperatureLevel = NULL;
    LADAPTER_Init(&stHandler);

    // Net Config Init
    SetNetConfig();

    LADAPTER_FRMPktRegister(LORAWAN_APP_PORT128, processPort128FRMPkt);

    // Tx Timer Init
    TimerInit( &TxNextPacketTimer, OnTxNextPktTimerEvent );

    // LoRa Net Start
    LoRaJoinStart(1);

    while( 1 )
    {
        // Process Radio IRQ
        if( Radio.IrqProcess != NULL )
        {
            Radio.IrqProcess( );
        }
        // Processes the LoRaMac events
        LADAPTER_Running();

        if( NextTx == true)
        {
            // Schedule next packet transmission
            TimerSetValue( &TxNextPacketTimer, APP_TX_DUTYCYCLE );
            TimerStart( &TxNextPacketTimer );
            NextTx = false;
        }
    }
}

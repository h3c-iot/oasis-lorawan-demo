/*!
 * \file      uart.c
 *
 * \brief     UART driver implementation
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \code
 *                ______                              _
 *               / _____)             _              | |
 *              ( (____  _____ ____ _| |_ _____  ____| |__
 *               \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 *               _____) ) ____| | | || |_| ____( (___| | | |
 *              (______/|_____)_|_|_| \__)_____)\____)_| |_|
 *              (C)2013-2017 Semtech
 *
 * \endcode
 *
 * \author    Miguel Luis ( Semtech )
 *
 * \author    Gregory Cristian ( Semtech )
 */
#include <string.h>
#include <stdlib.h>
#include "uart-board.h"
#include "uart.h"

void UartInit( Uart_t *obj, UartId_t uartId, PinNames tx, PinNames rx )
{
    if( obj->IsInitialized == false )
    {
        obj->IsInitialized = true;
        UartMcuInit( obj, uartId, tx, rx );
    }
}

void UartConfig( Uart_t *obj, UartMode_t mode, uint32_t baudrate, WordLength_t wordLength, StopBits_t stopBits, Parity_t parity, FlowCtrl_t flowCtrl )
{
    UartMcuConfig( obj, mode, baudrate, wordLength, stopBits, parity, flowCtrl );
}

void UartDeInit( Uart_t *obj )
{
    obj->IsInitialized = false;
    UartMcuDeInit( obj );
}

uint8_t UartPutChar( Uart_t *obj, uint8_t data )
{
     return UartMcuPutChar( obj, data );
}

uint8_t UartGetChar( Uart_t *obj, uint8_t *data )
{
    return UartMcuGetChar( obj, data );
}

uint8_t UartPutBuffer( Uart_t *obj, uint8_t *buffer, uint16_t size )
{
    return UartMcuPutBuffer( obj, buffer, size );
}

uint8_t UartGetBuffer( Uart_t *obj, uint8_t *buffer, uint16_t size, uint16_t *nbReadBytes )
{
    return UartMcuGetBuffer( obj, buffer, size, nbReadBytes );
}

/*==================== add by wangzhen ===================*/
typedef struct tagUartDriver
{
    Uart_t *pstUart;
    PinNames enPinTx;
    PinNames enPinRx;
}UART_DRIVER_S;

static UART_DRIVER_S *g_astUartList[UART_MAX] = {NULL, };

void *UartRegister( Uart_t *pstUart, UartId_t enUartId, PinNames enPinTx, PinNames enPinRx )
{
    UART_DRIVER_S *pstUartDriver = NULL;

    if (g_astUartList[enUartId] == NULL)
    {
        pstUartDriver = (UART_DRIVER_S *)malloc(sizeof(UART_DRIVER_S));
        if (pstUartDriver != NULL)
        {
            g_astUartList[enUartId] = pstUartDriver;
            g_astUartList[enUartId]->pstUart = pstUart;
            g_astUartList[enUartId]->enPinRx = enPinRx;
            g_astUartList[enUartId]->enPinTx = enPinTx;
        }
    }

    return (void *)pstUartDriver;
}

void UartDeRegister(UartId_t enUartId)
{
    if (g_astUartList[enUartId] != NULL)
    {
        if (g_astUartList[enUartId]->pstUart != NULL)
        {
            UartDeInit(g_astUartList[enUartId]->pstUart);
        }
        
        free(g_astUartList[enUartId]);
    }

    return;
}

Uart_t *UartOpen(UartId_t enUartId, UartMode_t mode, uint32_t baudrate, 
                    WordLength_t wordLength, StopBits_t stopBits, 
                    Parity_t parity, FlowCtrl_t flowCtrl)
{
    Uart_t *pstUart = NULL;
    UART_DRIVER_S *pstUartDriver = g_astUartList[enUartId];
    
    if (pstUartDriver != NULL)
    {
        pstUart = pstUartDriver->pstUart;
        if (pstUart != NULL)
        {
            UartInit(pstUart, enUartId, pstUartDriver->enPinTx, pstUartDriver->enPinRx);

            UartConfig(pstUart, mode, baudrate, wordLength, stopBits, parity, flowCtrl);
        }
    }
    
    return pstUart;
}

void UartClose(Uart_t *pstUart)
{
    if (pstUart != NULL)
    {
        FifoFlush(&(pstUart->FifoRx));
        FifoFlush(&(pstUart->FifoTx));
        
        UartDeInit(pstUart);
    }
    return;
}
/*======================== end  ===============================*/


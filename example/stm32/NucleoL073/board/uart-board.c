/*!
 * \file      uart-board.c
 *
 * \brief     Target board UART driver implementation
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
 *
 * \author    wangzhen (H3C) modified, 2019/10/23
 */
#include "stm32l0xx.h"
#include "utilities.h"
#include "board.h"
#include "uart-board.h"

/*!
 * Number of times the UartPutBuffer will try to send the buffer before
 * returning ERROR
 */
#define TX_BUFFER_RETRY_COUNT                       10

static UART_HandleTypeDef Uart4Handle;
static UART_HandleTypeDef Uart2Handle;

uint8_t Uart4RxData = 0;
uint8_t Uart4TxData = 0;

uint8_t Uart2RxData = 0;
uint8_t Uart2TxData = 0;

extern Uart_t Uart4;
extern Uart_t Uart2;

void UartMcuInit( Uart_t *obj, UartId_t uartId, PinNames tx, PinNames rx )
{
    obj->UartId = uartId;
    
    if( uartId == UART_USB_CDC )
    {
#if defined( USE_USB_CDC )
        UartUsbInit( obj, uartId, NC, NC );
#endif
    }
    else if (uartId == UART_DBG)
    {
        __HAL_RCC_USART2_FORCE_RESET( );
        __HAL_RCC_USART2_RELEASE_RESET( );
        __HAL_RCC_USART2_CLK_ENABLE( );

        GpioInit( &obj->Tx, tx, PIN_ALTERNATE_FCT, PIN_PUSH_PULL, PIN_PULL_UP, GPIO_AF4_USART2 );
        GpioInit( &obj->Rx, rx, PIN_ALTERNATE_FCT, PIN_PUSH_PULL, PIN_PULL_UP, GPIO_AF4_USART2 );
    }
    else
    {
        __HAL_RCC_USART4_FORCE_RESET( );
        __HAL_RCC_USART4_RELEASE_RESET( );
        __HAL_RCC_USART4_CLK_ENABLE( );

        GpioInit( &obj->Tx, tx, PIN_ALTERNATE_FCT, PIN_PUSH_PULL, PIN_PULL_UP, GPIO_AF6_USART4 );
        GpioInit( &obj->Rx, rx, PIN_ALTERNATE_FCT, PIN_PUSH_PULL, PIN_PULL_UP, GPIO_AF6_USART4 );
    }
    return;
}

void UartMcuConfig( Uart_t *obj, UartMode_t mode, uint32_t baudrate, WordLength_t wordLength, StopBits_t stopBits, Parity_t parity, FlowCtrl_t flowCtrl )
{
    UART_HandleTypeDef *pUartHandle = NULL;
    if( obj->UartId == UART_USB_CDC )
    {
#if defined( USE_USB_CDC )
        UartUsbConfig( obj, mode, baudrate, wordLength, stopBits, parity, flowCtrl );
#endif
    }
    else
    {
        if (obj->UartId == UART_SERIAL)
        {
            pUartHandle = &Uart4Handle;
            pUartHandle->Instance = USART4;
        }
        else
        {
            pUartHandle = &Uart2Handle;
            pUartHandle->Instance = USART2;
        }
        
        pUartHandle->Init.BaudRate = baudrate;

        if( mode == TX_ONLY )
        {
            if( obj->FifoTx.Data == NULL )
            {
                assert_param( FAIL );
            }
            pUartHandle->Init.Mode = UART_MODE_TX;
        }
        else if( mode == RX_ONLY )
        {
            if( obj->FifoRx.Data == NULL )
            {
                assert_param( FAIL );
            }
            pUartHandle->Init.Mode = UART_MODE_RX;
        }
        else if( mode == RX_TX )
        {
            if( ( obj->FifoTx.Data == NULL ) || ( obj->FifoRx.Data == NULL ) )
            {
                assert_param( FAIL );
            }
            pUartHandle->Init.Mode = UART_MODE_TX_RX;
        }
        else
        {
            assert_param( FAIL );
        }

        if( wordLength == UART_8_BIT )
        {
            pUartHandle->Init.WordLength = UART_WORDLENGTH_8B;
        }
        else if( wordLength == UART_9_BIT )
        {
            pUartHandle->Init.WordLength = UART_WORDLENGTH_9B;
        }

        switch( stopBits )
        {
        case UART_2_STOP_BIT:
            pUartHandle->Init.StopBits = UART_STOPBITS_2;
            break;
        case UART_1_5_STOP_BIT:
            pUartHandle->Init.StopBits = UART_STOPBITS_1_5;
            break;
        case UART_1_STOP_BIT:
        default:
            pUartHandle->Init.StopBits = UART_STOPBITS_1;
            break;
        }

        if( parity == NO_PARITY )
        {
            pUartHandle->Init.Parity = UART_PARITY_NONE;
        }
        else if( parity == EVEN_PARITY )
        {
            pUartHandle->Init.Parity = UART_PARITY_EVEN;
        }
        else
        {
            pUartHandle->Init.Parity = UART_PARITY_ODD;
        }

        if( flowCtrl == NO_FLOW_CTRL )
        {
            pUartHandle->Init.HwFlowCtl = UART_HWCONTROL_NONE;
        }
        else if( flowCtrl == RTS_FLOW_CTRL )
        {
            pUartHandle->Init.HwFlowCtl = UART_HWCONTROL_RTS;
        }
        else if( flowCtrl == CTS_FLOW_CTRL )
        {
            pUartHandle->Init.HwFlowCtl = UART_HWCONTROL_CTS;
        }
        else if( flowCtrl == RTS_CTS_FLOW_CTRL )
        {
            pUartHandle->Init.HwFlowCtl = UART_HWCONTROL_RTS_CTS;
        }

        pUartHandle->Init.OverSampling = UART_OVERSAMPLING_16;

        if( HAL_UART_Init( pUartHandle ) != HAL_OK )
        {
            assert_param( FAIL );
        }

        if (obj->UartId == UART_SERIAL)
        {
            HAL_NVIC_SetPriority( USART4_5_IRQn, 1, 0 );
            HAL_NVIC_EnableIRQ( USART4_5_IRQn );

            /* Enable the UART Data Register not empty Interrupt */
            HAL_UART_Receive_IT( pUartHandle, &Uart4RxData, 1 );
        }
        else
        {
            HAL_NVIC_SetPriority( USART2_IRQn, 1, 0 );
            HAL_NVIC_EnableIRQ( USART2_IRQn );

            /* Enable the UART Data Register not empty Interrupt */
            HAL_UART_Receive_IT( pUartHandle, &Uart2RxData, 1 );
        }
    }
}

void UartMcuDeInit( Uart_t *obj )
{
    if( obj->UartId == UART_USB_CDC )
    {
#if defined( USE_USB_CDC )
        UartUsbDeInit( obj );
#endif
    }
    else if (obj->UartId == UART_DBG)
    {
        __HAL_RCC_USART2_FORCE_RESET( );
        __HAL_RCC_USART2_RELEASE_RESET( );
        __HAL_RCC_USART2_CLK_DISABLE( );

        GpioInit( &obj->Tx, obj->Tx.pin, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
        GpioInit( &obj->Rx, obj->Rx.pin, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    }
    else
    {
        __HAL_RCC_USART4_FORCE_RESET( );
        __HAL_RCC_USART4_RELEASE_RESET( );
        __HAL_RCC_USART4_CLK_DISABLE( );

        GpioInit( &obj->Tx, obj->Tx.pin, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
        GpioInit( &obj->Rx, obj->Rx.pin, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    }
}

uint8_t UartMcuPutChar( Uart_t *obj, uint8_t data )
{
    UART_HandleTypeDef *pUartHandle = NULL;
    
    if( obj->UartId == UART_USB_CDC )
    {
#if defined( USE_USB_CDC )
        return UartUsbPutChar( obj, data );
#else
        return 255; // Not supported
#endif
    }
    else
    {
        if (obj->UartId == UART_SERIAL)
        {
            pUartHandle = &Uart4Handle;
        }
        else
        {
            pUartHandle = &Uart2Handle;
        }
        
        CRITICAL_SECTION_BEGIN( );

        if( IsFifoFull( &obj->FifoTx ) == false )
        {
            FifoPush( &obj->FifoTx, data );

            // Trig UART Tx interrupt to start sending the FIFO contents.
            __HAL_UART_ENABLE_IT( pUartHandle, UART_IT_TC );

            CRITICAL_SECTION_END( );
            return 0; // OK
        }
        CRITICAL_SECTION_END( );
        return 1; // Busy
    }
}

uint8_t UartMcuGetChar( Uart_t *obj, uint8_t *data )
{
    if( obj->UartId == UART_USB_CDC )
    {
#if defined( USE_USB_CDC )
        return UartUsbGetChar( obj, data );
#else
        return 255; // Not supported
#endif
    }
    else
    {
        CRITICAL_SECTION_BEGIN( );

        if( IsFifoEmpty( &obj->FifoRx ) == false )
        {
            *data = FifoPop( &obj->FifoRx );
            CRITICAL_SECTION_END( );
            return 0;
        }
        CRITICAL_SECTION_END( );
        return 1;
    }
}

uint8_t UartMcuPutBuffer( Uart_t *obj, uint8_t *buffer, uint16_t size )
{
    if( obj->UartId == UART_USB_CDC )
    {
#if defined( USE_USB_CDC )
        return UartUsbPutBuffer( obj, buffer, size );
#else
        return 255; // Not supported
#endif
    }
    else
    {
        uint8_t retryCount;
        uint16_t i;

        for( i = 0; i < size; i++ )
        {
            retryCount = 0;
            while( UartPutChar( obj, buffer[i] ) != 0 )
            {
                retryCount++;

                // Exit if something goes terribly wrong
                if( retryCount > TX_BUFFER_RETRY_COUNT )
                {
                    return 1; // Error
                }
            }
        }
        return 0; // OK
    }
}

uint8_t UartMcuGetBuffer( Uart_t *obj, uint8_t *buffer, uint16_t size, uint16_t *nbReadBytes )
{
    uint16_t localSize = 0;

    while( localSize < size )
    {
        if( UartGetChar( obj, buffer + localSize ) == 0 )
        {
            localSize++;
        }
        else
        {
            break;
        }
    }

    *nbReadBytes = localSize;

    if( localSize == 0 )
    {
        return 1; // Empty
    }
    return 0; // OK
}

void HAL_UART_TxCpltCallback( UART_HandleTypeDef *handle )
{
    Uart_t *pUart = NULL;
    uint8_t *pTxData = NULL;

    if (handle->Instance == USART4)
    {
        pUart = &Uart4;
        pTxData = &Uart4TxData;
    }
    else
    {
        pUart = &Uart2;
        pTxData = &Uart2TxData;
    }
    
    if( IsFifoEmpty( &(pUart->FifoTx) ) == false )
    {
        *pTxData = FifoPop( &(pUart->FifoTx) );
        //  Write one byte to the transmit data register
        HAL_UART_Transmit_IT( handle, pTxData, 1 );
    }

    if( pUart->IrqNotify != NULL )
    {
        pUart->IrqNotify( UART_NOTIFY_TX );
    }
}

void HAL_UART_RxCpltCallback( UART_HandleTypeDef *handle )
{
    Uart_t *pUart = NULL;
    uint8_t *pRxData = NULL;

    if (handle->Instance == USART4)
    {
        pUart = &Uart4;
        pRxData = &Uart4RxData;
    }
    else
    {
        pUart = &Uart2;
        pRxData = &Uart2RxData;
    }
    
    if( IsFifoFull( &(pUart->FifoRx) ) == false )
    {
        // Read one byte from the receive data register
        FifoPush( &(pUart->FifoRx), *pRxData );
    }

    if( pUart->IrqNotify != NULL )
    {
        pUart->IrqNotify( UART_NOTIFY_RX );
    }

    HAL_UART_Receive_IT( handle, pRxData, 1 );
}

void HAL_UART_ErrorCallback( UART_HandleTypeDef *handle )
{
    uint8_t *pRxData = NULL;

    if (handle->Instance == USART4)
    {
        pRxData = &Uart4RxData;
    }
    else
    {
        pRxData = &Uart2RxData;
    }
    
    HAL_UART_Receive_IT( handle, pRxData, 1 );
}

void USART2_IRQHandler( void )
{
    HAL_UART_IRQHandler( &Uart2Handle );
}

void USART4_5_IRQHandler( void )
{
    HAL_UART_IRQHandler( &Uart4Handle );
}


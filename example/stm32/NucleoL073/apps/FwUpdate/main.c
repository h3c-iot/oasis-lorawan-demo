/*******************************************************************************
*                                                                              *
*                Copyright (C) 2019, New H3C Technologies Co., Ltd.            *
*  License: Revised BSD License, see LICENSE.TXT file included in the project  *
*                                                                              *
*------------------------------------------------------------------------------*
*                                                                              *
*                              main.c                                          *
*                                                                              *
*  Project Code: oasis-sdk                                                     *
*   Module Name: Lora Module WSL305S                                           *
*  Date Created: 2019-10-24                                                    *
*        Author: hxb18134                                                      *
*   Description: adapt to wsl305s module firmeware update                      *
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
#include "board.h"
#include "board-config.h"
#include "gpio.h"
#include "uart.h"
#include "delay.h"
#include "xmodem_1k_master.h"



/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define TEST_XMODEM_SEND

/* FLASH ADDRESS */
#define FLASH_BASE                  (0x08000000UL) /*!< FLASH base address in the alias region */ 
#define FLASH_PAGE_SIZE             ((uint32_t)128U)  /*!< FLASH Page Size in bytes */
#define FLASH_USER_START_ADDR       (FLASH_BASE + FLASH_PAGE_SIZE * 256)
#define FLASH_USER_END_ADDR         (FLASH_USER_START_ADDR + FLASH_PAGE_SIZE * 32)

extern Uart_t Uart4;

/* Private functions ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/



/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
#ifdef TEST_XMODEM_RECEIVE
int main(void)
{
    int st;

    printf ("Send data using the xmodem protocol from your terminal emulator now...\n");
    /* the following should be changed for your environment:
       0x30000 is the download address,
       65536 is the maximum size to be written at this address
     */
    st = xmodemReceive((char *)0x8008000, 65536);
    if (st < 0) {
        printf ("Xmodem receive error: status: %d\n", st);
    }
    else  {
        printf ("Xmodem successfully received %d bytes\n", st);
    }

    return 0;
}
#endif

#ifdef TEST_XMODEM_SEND
int main(void)
{
    unsigned char *src_data;
    int src_size;
    int st;
    uint8_t Rxdata;

    BoardInitMcu( );

    printf ("Prepare your terminal emulator to receive data now...\r\n");
    
    /* 版本数据FLASH起始地址 */
    src_data = (unsigned char *)FLASH_USER_START_ADDR;
    /* 版本文件字节数 */
    src_size = 53584;

    /* 上电后发送"U",是模组进入BOOT模式             */
    printf("enter boot mode\r\n");
    UartPutChar(&Uart4, (uint8_t)BOOT_ENTER);
    DelayMs(1000);
    while(UartGetChar(&Uart4, &Rxdata) == 0)
    {
    }

    DelayMs(1000);
    printf("start upgrade\r\n");
    UartPutChar(&Uart4, (uint8_t)UPGRADE_START);
    
    st = xmodemTransmit(src_data, src_size);
    if (st < 0) {
        printf ("Xmodem transmit error: status: %d\r\n", st);
    }
    else  {
        printf ("Xmodem successfully transmitted %d bytes\r\n", st);
    }

    return 0;
}
#endif


#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

/*******************************************************************************
*                                                                              *
*                Copyright (C) 2019, New H3C Technologies Co., Ltd.            *
*  License: Revised BSD License, see LICENSE.TXT file included in the project  *
*                                                                              *
*------------------------------------------------------------------------------*
*                                                                              *
*                               xmodem_1k_master.C                             *
*                                                                              *
*  Project Code: oasis-sdk                                                     *
*   Module Name: Lora Module WSL305S                                           *
*  Date Created: 2019-10-28                                                    *
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
#include "uart.h"
#include "delay.h"
#include "crc16.h"
#include "xmodem_1k_master.h"


extern Uart_t Uart4;

static int check(int crc, const unsigned char *buf, int sz)
{
    if (crc) {
        unsigned short crc = crc16_ccitt(buf, sz);
        unsigned short tcrc = (buf[sz]<<8)+buf[sz+1];
        if (crc == tcrc)
            return 1;
    }
    else {
        int i;
        unsigned char cks = 0;
        for (i = 0; i < sz; ++i) {
            cks += buf[i];
        }
        if (cks == buf[sz])
        return 1;
    }

    return 0;
}

static uint8_t _inbyte(uint32_t time_ms)
{
    uint8_t Rxdata;
    
    DelayMs(time_ms);
    if(UartGetChar(&Uart4, &Rxdata) == 0)
    {
        printf("UartGetChar:0x%x\r\n",Rxdata);
        return Rxdata;
    }
    return 0;    
}

static uint8_t _outbyte(unsigned char out_data)
{   
    printf("0x%x\r\n",out_data);
    return UartPutChar(&Uart4, out_data);
}


static void flushinput(void)
{
    printf("flushinput\r\n");
    /* 延时1.5s后判断 */
    while (_inbyte(((DLY_1S)*3)>>1) >= 0)
    {
    }
    return;
}

int xmodemReceive(unsigned char *dest, int destsz)
{
    unsigned char xbuff[1030]; /* 1024 for XModem 1k + 3 head chars + 2 crc + nul */
    unsigned char *p;
    int bufsz, crc = 0;
    unsigned char trychar = 'C';
    unsigned char packetno = 1;
    int i, c, len = 0;
    int retry, retrans = MAXRETRANS;

    for(;;) {
        for( retry = 0; retry < 16; ++retry) {
            if (trychar) _outbyte(trychar);
            if ((c = _inbyte((DLY_1S)<<1)) >= 0) {
                printf("%c\r\n",c);
                switch (c) {
                case SOH:
                    bufsz = 128;
                    goto start_recv;
                case STX:
                    bufsz = 1024;
                    goto start_recv;
                case EOT:
                    flushinput();
                    _outbyte(ACK);
                    return len; /* normal end */
                case CAN:
                    if ((c = _inbyte(DLY_1S)) == CAN) {
                        flushinput();
                        _outbyte(ACK);
                        return -1; /* canceled by remote */
                    }
                    break;
                default:
                    break;
                }
            }
        }
        if (trychar == 'C') { trychar = NAK; continue; }
        flushinput();
        _outbyte(CAN);
        _outbyte(CAN);
        _outbyte(CAN);
        return -2; /* sync error */

    start_recv:
        if (trychar == 'C') crc = 1;
        trychar = 0;
        p = xbuff;
        *p++ = c;
        for (i = 0;  i < (bufsz+(crc?1:0)+3); ++i) {
            if ((c = _inbyte(DLY_1S)) < 0) goto reject;
            *p++ = c;
        }

        if (xbuff[1] == (unsigned char)(~xbuff[2]) && 
            (xbuff[1] == packetno || xbuff[1] == (unsigned char)packetno-1) &&
            check(crc, &xbuff[3], bufsz)) {
            if (xbuff[1] == packetno)   {
                register int count = destsz - len;
                if (count > bufsz) count = bufsz;
                if (count > 0) {
                    memcpy(&dest[len], &xbuff[3], count);
                    len += count;
                }
                ++packetno;
                retrans = MAXRETRANS+1;
            }
            if (--retrans <= 0) {
                flushinput();
                _outbyte(CAN);
                _outbyte(CAN);
                _outbyte(CAN);
                return -3; /* too many retry error */
            }
            _outbyte(ACK);
            continue;
        }
    reject:
        flushinput();
        _outbyte(NAK);
    }
}

int xmodemTransmit(unsigned char *src, int srcsz)
{
    unsigned char xbuff[1030]; /* 1024 for XModem 1k + 3 head chars + 2 crc + nul */
    int bufsz, crc = -1;
    unsigned char packetno = 1;
    int i, c, len = 0;
    int retry;  
    
    for(;;) {
        for( retry = 0; retry < 16; ++retry) {
            if ((c = _inbyte((DLY_1S)<<1)) >= 0) {
                switch (c) {
                case 'C':
                    crc = 1;
                    printf("start_trans\r\n");
                    goto start_trans;
                case NAK:
                    crc = 0;
                    goto start_trans;
                case CAN:
                    if ((c = _inbyte(DLY_1S)) == CAN) {
                        _outbyte(ACK);
                        flushinput();
                        return -1; /* canceled by remote */
                    }
                    break;
                default:
                    break;
                }
            }
        }
        _outbyte(CAN);
        _outbyte(CAN);
        _outbyte(CAN);
        flushinput();
        return -2; /* no sync */

        for(;;) {
        start_trans:
#ifdef TRANSMIT_XMODEM_1K
            xbuff[0] = STX; bufsz = 1024;
#else
            xbuff[0] = SOH; bufsz = 128;
#endif
            xbuff[1] = packetno;
            xbuff[2] = ~packetno;
            c = srcsz - len;
            if (c > bufsz) c = bufsz;
            if (c > 0) {
                memset (&xbuff[3], 0, bufsz);
                memcpy (&xbuff[3], &src[len], c);
                if (c < bufsz) xbuff[3+c] = CTRLZ;
                if (crc) {
                    unsigned short ccrc = crc16_ccitt(&xbuff[3], bufsz);
                    xbuff[bufsz+3] = (ccrc>>8) & 0xFF;
                    xbuff[bufsz+4] = ccrc & 0xFF;
                }
                else {
                    unsigned char ccks = 0;
                    for (i = 3; i < bufsz+3; ++i) {
                        ccks += xbuff[i];
                    }
                    xbuff[bufsz+3] = ccks;
                }
                for (retry = 0; retry < MAXRETRANS; ++retry) {
                    printf("retry:%d\r\n",retry);         
                    UartPutBuffer(&Uart4, xbuff, bufsz+4+(crc?1:0));
                
                    if ((c = _inbyte(DLY_1S * 2)) >= 0 ) {
                        printf("packetno:%d,rsp:%x,crc:%d\r\n",packetno,c,crc);
                        switch (c) {
                        case ACK:
                            ++packetno;
                            len += bufsz;
                            goto start_trans;
                        case CAN:
                            if ((c = _inbyte(DLY_1S)) == CAN) {
                                _outbyte(ACK);
                                flushinput();
                                return -1; /* canceled by remote */
                            }
                            break;
                        case NAK:
                        default:
                            break;
                        }
                    }
                }
                _outbyte(CAN);
                _outbyte(CAN);
                _outbyte(CAN);
                flushinput();
                return -4; /* xmit error */
            }
            else {
                for (retry = 0; retry < 10; ++retry) {
                    _outbyte(EOT);
                    if ((c = _inbyte((DLY_1S * 2))) == ACK) break;
                }
               // flushinput();
                return (c == ACK)?len:-5;
            }
        }
    }
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

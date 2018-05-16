/*
*********************************************************************************************************
*                                     MICRIUM BOARD SUPPORT SUPPORT
*
*                          (c) Copyright 2003-2009; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*               Knowledge of the source code may NOT be used to develop a similar product.
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                      MICRIUM BOARD SUPPORT PACKAGE
*                                         SERIAL (UART) INTERFACE
*
* Filename      : bsp_ser.c
* Version       : V1.00
* Programmer(s) : EHS
                  02-02-2011 gpc - Change to simple polled I/O, not requiring the OS.
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#define  BSP_SER_MODULE
#include <bsp.h>


/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/
#define USART_RXNE  0x20  // Rx not Empty Status Bit
#define USART_TXE   0x80  // Tx Empty Status Bit

/*
*********************************************************************************************************
*                                           LOCAL CONSTANTS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                          LOCAL DATA TYPES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            LOCAL TABLES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                     LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*********************************************************************************************************
**                                         GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                          BSP_Ser_Init()
*
* Description : Initialize a serial port for communication.
*
* Argument(s) : baud_rate           The desire RS232 baud rate.
*
* Return(s)   : none.
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_Ser_Init (CPU_INT32U  baud_rate)
{
    GPIO_InitTypeDef        gpio_init;
    USART_InitTypeDef       usart_init;
    USART_ClockInitTypeDef  usart_clk_init;

    usart_init.USART_BaudRate            = baud_rate;
    usart_init.USART_WordLength          = USART_WordLength_8b;
    usart_init.USART_StopBits            = USART_StopBits_1;
    usart_init.USART_Parity              = USART_Parity_No ;
    usart_init.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    usart_init.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;

    usart_clk_init.USART_Clock           = USART_Clock_Disable;
    usart_clk_init.USART_CPOL            = USART_CPOL_Low;
    usart_clk_init.USART_CPHA            = USART_CPHA_2Edge;
    usart_clk_init.USART_LastBit         = USART_LastBit_Disable;


    BSP_PeriphEn(BSP_PERIPH_ID_USART2);
    BSP_PeriphEn(BSP_PERIPH_ID_IOPD);
    BSP_PeriphEn(BSP_PERIPH_ID_AFIO);
    GPIO_PinRemapConfig(GPIO_Remap_USART2, ENABLE);
                                                                /* ----------------- SETUP USART2 GPIO ---------------- */
                                                                /* Configure GPIOD.5 as push-pull.                      */
    gpio_init.GPIO_Pin   = GPIO_Pin_5;
    gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
    gpio_init.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOD, &gpio_init);

                                                                /* Configure GPIOD.6 as input floating.                 */
    gpio_init.GPIO_Pin   = GPIO_Pin_6;
    gpio_init.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOD, &gpio_init);

                                                                /* ------------------ SETUP USART2 -------------------- */
    USART_Init(USART2, &usart_init);
    USART_ClockInit(USART2, &usart_clk_init);
    USART_Cmd(USART2, ENABLE);
}


/*
*********************************************************************************************************
*                                                BSP_Ser_Printf()
*
* Description : Formatted outout to the serial port.
*               This funcion reads a string from a serial port. This call blocks until a
*               character appears at the port and the last character is a Carriage
*               Return (0x0D).
*
* Argument(s) : Format string follwing the C format convention.
*
* Return(s)   : none.
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_Ser_Printf (CPU_CHAR *format, ...)
{
    static  CPU_CHAR  buffer[80 + 1];
            va_list   vArgs;


    va_start(vArgs, format);
    vsprintf((char *)buffer, (char const *)format, vArgs);
    va_end(vArgs);

    BSP_Ser_WrStr((CPU_CHAR*) buffer);
}


/*
*********************************************************************************************************
*                                                BSP_Ser_RdByte()
*
* Description : Receive a single byte.
*
* Argument(s) : none.
*
* Return(s)   : The received byte
*
* Caller(s)   : Application
*
* Note(s)     : (1) This functions blocks until a data is received.
*
*               (2) It can not be called from an ISR.
*********************************************************************************************************
*/

CPU_INT08U  BSP_Ser_RdByte (void)
{
    CPU_INT08U   rx_byte;

    CPU_REG32  status;

    while (!(USART2->SR & USART_RXNE))
      ;
    
    rx_byte = USART2->DR;                                    /* Read the data form the temporal register          */

    return (rx_byte);
}

/*
*********************************************************************************************************
*                                                BSP_Ser_RdStr()
*
* Description : This function reads a string from a UART.
*
* Argument(s) : p_str      A pointer to a buffer at which the string can be stored.
*
*               len         The size of the string that will be read.
*
* Return(s)   : none.
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_Ser_RdStr (CPU_CHAR    *p_str,
                     CPU_INT16U   len)
{
    CPU_CHAR     rx_data;
    CPU_CHAR     rx_buf_ix;

    rx_buf_ix = 0;
    p_str[0]  = 0;

    while (DEF_TRUE)
    {
        rx_data = BSP_Ser_RdByte();

        if ((rx_data == ASCII_CHAR_CARRIAGE_RETURN) ||          /* Is it '\r' or '\n' character  ?                    */
            (rx_data == ASCII_CHAR_LINE_FEED      )) {

            BSP_Ser_WrByte((CPU_INT08U)ASCII_CHAR_LINE_FEED);
            BSP_Ser_WrByte((CPU_INT08U)ASCII_CHAR_CARRIAGE_RETURN);

            p_str[rx_buf_ix] = 0;                              /* set the null character at the end of the string     */
            break;                                             /* exit the loop                                       */
        }

        if (rx_data == ASCII_CHAR_BACKSPACE) {                 /* Is backspace character                              */
            if (rx_buf_ix > 0) {
                BSP_Ser_WrByte((CPU_INT08U)ASCII_CHAR_BACKSPACE);
                BSP_Ser_WrByte((CPU_INT08U)ASCII_CHAR_SPACE);
                BSP_Ser_WrByte((CPU_INT08U)ASCII_CHAR_BACKSPACE);

                rx_buf_ix--;                                   /* Decrement the index                                 */
                p_str[rx_buf_ix] = 0;
            }
        }

        if (ASCII_IsPrint(rx_data)) {                           /* Is it a printable character ... ?                  */
            BSP_Ser_WrByte((CPU_INT08U)rx_data);        /* Echo-back                                          */
            p_str[rx_buf_ix] = rx_data;                         /* Save the received character in the buffer          */
            rx_buf_ix++;                                        /* Increment the buffer index                         */
            if (rx_buf_ix >= len) {
               rx_buf_ix = len;
            }
        }
    }

}


/*
*********************************************************************************************************
*                                                BSP_Ser_WrByte()
*
* Description : Writes a single byte to a serial port.
*
* Argument(s) : tx_byte     The character to output.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_Ser_WrByte(CPU_INT08U  c)
{
    CPU_REG32  status;

    while (!(USART2->SR &  USART_TXE))
      ;

    USART2->DR = c;
}

/*
*********************************************************************************************************
*                                                BSP_Ser_WrStr()
*
* Description : Transmits a string.
*
* Argument(s) : p_str      Pointer to the string that will be transmitted
*
* Caller(s)   : Application
*
* Return(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_Ser_WrStr (CPU_CHAR  *p_str)
{
    while ((*p_str) != (CPU_CHAR )0) {

        if (*p_str == ASCII_CHAR_LINE_FEED) {
            BSP_Ser_WrByte(ASCII_CHAR_CARRIAGE_RETURN);
            BSP_Ser_WrByte(ASCII_CHAR_LINE_FEED);
            p_str++;
        } else {
            BSP_Ser_WrByte(*p_str++);
        }
    }

}


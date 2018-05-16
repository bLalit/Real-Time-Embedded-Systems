/*--------------------------------SerlIODriver.c----------------------------------*/
/*Name: Lalit Bhat
Student ID:01671833
Program Number: 4
*/

#include "Includes.h"
#include "SerIODriver.h"
#include "BfrPair.h"
#include "assert.h"

/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/
#define USART_RXNE  0x20  // Rx not Empty Status Bit
#define USART_TXE   0x80  // Tx Empty Status Bit
#define USART_RXNEIE  0x20  // Rx Not Empty Interrupt Mask
#define USART_TXEIE   0x80  // Tx Empty Interrupt Mask

/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/

// If not already defined, use the default buffer size of 4.
#ifndef BfrSize
#define BfrSize 4
#endif

// Allocate the input buffer pair.
static BfrPair iBfrPair;
static CPU_INT08U iBfr0Space[BfrSize];
static CPU_INT08U iBfr1Space[BfrSize];

// Allocate the output buffer pair.
static BfrPair oBfrPair;
static CPU_INT08U oBfr0Space[BfrSize];
static CPU_INT08U oBfr1Space[BfrSize];

OS_SEM	openOBfrs;	 
OS_SEM	closedIBfrs;	



/*Initialize the RS232 I/O driver by initializing both iBfrPair and oBfrPair.
Unmask the Tx and the Rx. Also initialize the two semaphores openObfrs and 
closedIBfrs.*/
void InitSerIO(void)
{
  OS_ERR      err;


  BfrPairInit(&iBfrPair, iBfr0Space, iBfr1Space, BfrSize);
  BfrPairInit(&oBfrPair, oBfr0Space, oBfr1Space, BfrSize);
  
  USART2->CR1 |= (USART_TXEIE | USART_RXNEIE);

  OSSemCreate(&closedIBfrs, "Full Buffers", 0, &err);			/* -- Empty at start. */
  assert(err == OS_ERR_NONE);
  
  OSSemCreate(&openOBfrs, "Empty Bfrs", 1, &err);	/* -- BfrSize bytes available. */
  assert(err == OS_ERR_NONE);
}

/*If TXE = 0, just return. Otherwise, if the get bufer is not closed, mask the Tx interrupt
and return.
If TXE = 1 and the get buffer is closed, remove the next byte from the get buger and output
it to the Tx. If the get buffer is now open, post the semaphore openOBfrs.*/
void ServiceTx(void)
{
  OS_ERR      osErr;
  CPU_CHAR c;
  if (!(USART2->SR &  USART_TXE))
    return;
  
  if (!GetBfrClosed(&oBfrPair))
    {
    USART2->CR1 &= ~USART_TXEIE;
    return; 
    }
  c = GetBfrRemByte(&oBfrPair);
  USART2->DR = c;

  if (!GetBfrClosed(&oBfrPair))
    {
    OSSemPost(&openOBfrs, OS_OPT_POST_1, &osErr);
    assert(osErr==OS_ERR_NONE);
    }
}

void TxFlush(void)
{
  OS_ERR      osErr;

  ClosePutBfr(&oBfrPair);

  OSSemPend(&openOBfrs, 0, OS_OPT_PEND_BLOCKING, NULL, &osErr);
  assert(osErr==OS_ERR_NONE);

  BfrPairSwap(&oBfrPair);

  USART2->CR1 |= USART_TXEIE;
}

/*If the oBfrPair pu buffer is not closed, write one byte into the buffer,
unmask the Tx interrupt, and return txChar as the return value.
If, the buffer is closed pend on the semaphore openOBfrs and then swap the buffers.*/
CPU_INT16S PutByte(CPU_INT16S c)
{
  OS_ERR      osErr;

  if (PutBfrClosed(&oBfrPair))
    {
    OSSemPend(&openOBfrs, 0, OS_OPT_PEND_BLOCKING, NULL, &osErr);
    assert(osErr==OS_ERR_NONE);

    BfrPairSwap(&oBfrPair);
    }  
   
  PutBfrAddByte(&oBfrPair, c);
  
  USART2->CR1 |= USART_TXEIE;

  return c;
}

/*------------------------ServiceRx(void)------------------------*/
/*If PutbfrClosed is closed then post closedIBfrs*/

void ServiceRx(void)
{
  OS_ERR      osErr;

  if (!(USART2->SR & USART_RXNE))
  {
    CPU_INT08S c = USART2->SR & USART_RXNE;
    return;
  }

  if (PutBfrClosed(&iBfrPair))
    {
    USART2->CR1 &= ~USART_RXNEIE;
    return;
    }
    
  PutBfrAddByte(&iBfrPair, (CPU_INT16S) USART2->DR);

  if (PutBfrClosed(&iBfrPair))
    {
    OSSemPost(&closedIBfrs, OS_OPT_POST_1, &osErr);
    assert(osErr==OS_ERR_NONE);
    }

}

/*------------------GetByte()-----------*/
CPU_INT16S GetByte(void)
{
  OS_ERR osErr;		/* -- Semaphore error code */
  if(!GetBfrClosed(&iBfrPair))
  {
    /* Wait for receiver to fill the IBfr put buffer done and buffer not empty. */
    OSSemPend(&closedIBfrs, 0, OS_OPT_PEND_BLOCKING, NULL, &osErr);	
    assert(osErr==OS_ERR_NONE);
    BfrPairSwap(&iBfrPair); /*upon posting in serviceRx()swap the put buffer and getbuffer to start filling the data*/
  }		        
  CPU_INT16S c =GetBfrRemByte(&iBfrPair);/*extract the byte from getbuffer*/
  USART2->CR1 |= USART_RXNEIE;
  return c;
}
/*--------------- S e r P r i n t f ( ) ---------------

PURPOSE:
Provides functionality of standard C printf( ) function printing
to the RS232 Tx rather than to the screen.

INPUT PARAMETERS:
format  - Standard printf format string
...     - Zero or more parameters giving values to print according
          to the "format" format string
*/

void SerPrintf(CPU_CHAR *format, ...)
{
#define PrintfBfrSize 81
  
  CPU_CHAR  buffer[PrintfBfrSize];  //  SerPrintf( ) output string buffer
  va_list   vArgs;                  //  Used to interpret "format" parameter

  CPU_CHAR *ptr;                    // Pointer to output string

  // Generate formatted output in "buffer"
  va_start(vArgs, format);
  vsnprintf((char *)buffer, PrintfBfrSize, (char const *)format, vArgs);
  va_end(vArgs);

  // Output the SerPrintf( ) buffer
  for (ptr=buffer; *ptr!='\0';)
    PutByte(*ptr++);    
}

/*--------------- E x t I n t e r r u p t ( ) ---------------*/

/*
PURPOSE
External interrupt exception handler. 
Handle rx and tx interrupts.
*/
void ISR(void)
{
  /* Disable interrupts. */
  CPU_SR_ALLOC();
  OS_CRITICAL_ENTER();  
  
	/* Tell kernel we're in an ISR. */
	OSIntEnter();

  /* Enable interrupts. */
  OS_CRITICAL_EXIT();
	
  // Service rx interrupt.
  ServiceRx();

  // Service tx interrupt.
  ServiceTx();

	/* Give the O/S a chance to swap tasks. */
	OSIntExit ();
}

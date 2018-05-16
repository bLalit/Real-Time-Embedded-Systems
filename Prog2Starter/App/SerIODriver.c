/*-----------------------------------SerIODriver.c------------------------------------------*/
/*Name: Lalit Bhat
Student ID:01671833
Program Number: 2
*/

#include "SerIODriver.h"


#ifndef BfrSize
#define BfrSize 4 //Buffer size is set to 4
#endif

#define USART_TXE 0X80
#define USART_RXNE 0x20
#define TXEIE 0x80      //TXE interrupt enable
#define RXNEIE 0x20     //RXNE interrupt enable

static BfrPair iBfrPair; //Allocate the input buffer pair
static CPU_INT08U iBfr0Space[BfrSize];
static CPU_INT08U iBfr1Space[BfrSize];

static BfrPair oBfrPair; //Allocate the input buffer pair
static CPU_INT08U oBfr0Space[BfrSize];
static CPU_INT08U oBfr1Space[BfrSize];

/*Initialize the RS232 I.O driver by initializing both iBfrPair and oBfrPair*/
void InitSerIO(void)
{
  BfrPairInit(&iBfrPair,iBfr0Space,iBfr1Space,BfrSize);
  BfrPairInit(&oBfrPair,oBfr0Space,oBfr1Space,BfrSize);
  USART2->CR1=0x20AC;
  USART2->SR=0x00C0;
#define SETENA1 (*((CPU_INT32U *) 0x000E104)) //SETENA1 is memory mapped to this address
#define USART2ENA 0x00000040
  SETENA1 = USART2ENA;
}

/* If the oBfrPair put buffer is closed and the oBfrPair get buffer is not closed, swap the get buffer and the put buffer
If the oBfrPair put buffer is not full and write ine byte into the buffer abd return tcChar as the return value; 
if the buffer is full, return -1 indiacating falure.*/
CPU_INT16S PutByte(CPU_INT16S txChar)
{
  if (BfrPairSwappable(&oBfrPair))
  {
    BfrPairSwap(&oBfrPair);
  }
  if ((PutBfrClosed(&oBfrPair)))
  {
    return -1;
  }
    USART2->CR1 =  ((USART2->CR1)|TXEIE);
    return(PutBfrAddByte(&oBfrPair, txChar));
  
}

/* If TXE =1 and the oBfrPair get buffer is not empty, then output one byte to the UART Tx and retun*/
/*If TXE=0 or if get buffer is empty, just return*/
void ServiceTx(void)
{
  if(USART2->SR & USART_TXE)
    if(GetBfrClosed(&oBfrPair))
    {
      USART2->DR = (CPU_INT08U)GetBfrRemByte(&oBfrPair);
    } 
    else
    {
      USART2->CR1 = ((USART2->CR1)&(~TXEIE));
      return;
    }
  else
    return;
  
  
}
  
/*If the iBfrPair Put buffer is closed and the iBfrPair get buffer is not closed,
  swap the get buffer and put buffer*/
/*If the iBfrPair get buffer is not empth, remove and return the next byte from 
  the buffer. if the buffer is empty, returm -1 indicating failure*/
CPU_INT16S GetByte(void)
{
  if (BfrPairSwappable(&iBfrPair))
  {
    BfrPairSwap(&iBfrPair);
  }
  if (GetBfrClosed(&iBfrPair))
  {
    USART2->CR1 = ((USART2->CR1)|RXNEIE);
    return(GetBfrRemByte(&iBfrPair));
  }
  else
    return -1;
}

/*If RXNE = 1 and  the iBfrPair put buffer is not full, read a byte from 
  the UART Rx and add it to the put buffer.*/
/* If RXNE = 0 or the put buffer is full, just return.*/
void ServiceRx(void)
{
 if((USART2->SR & USART_RXNE))
  {
    if(!(PutBfrClosed(&iBfrPair)))
    {
      PutBfrAddByte(&iBfrPair, USART2 -> DR);
    }
    else
    {
      USART2->CR1 = ((USART2->CR1)&(~RXNEIE));
      return;
    }
  }
 else
   return;
}

/*Call ServiceRx() to handle Rx interrupts and then call ServiceTx() 
to handle Tx interrupts*/
void  SerialISR(void)
{
  ServiceRx();
  ServiceTx();
}
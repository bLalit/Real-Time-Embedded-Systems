/*=============== P k t P a r s e r . c ===============*/

/* Edited by: Lalit Bhat
Student ID: 01671833

BY:	George Cheney
		EECE472 / EECE572 Embedded Real Time Systems
		Electrical and Computer Engineering Dept.
		UMASS Lowell
*/

/*PURPOSE
This is a dummy Parser Task used to test the serial I/O driver module. In the final version 
it will become the Packet Parser Task.

DEMONSTRATES
Multitasking
Semaphores

CHANGES
03-02-2017  - Created for Spring 2017
*/
#include "PktParser.h"
#include "BfrPair.h"
#define DestinationAddr 1
#define SuspendTimeout 0
#define P1Char 0x03
#define P2Char 0xEF
#define P3Char 0xAF
#define NullChar 0
#define MinimumPacketLength 8
#define CheckSumVerify 0
#define SuspentTimeout 0
#define HeaderLength 4

//----- c o n s t a n t    d e f i n i t i o n s -----

#define ParserPrio 6 // Parser task priority

/* Size of the Process task stack */
#define	PARSER_STK_SIZE     256 

/*----- g l o b a l s -----*/

// Process Task Control Block
OS_TCB parserTCB;

/* Stack space for Process task stack */
CPU_STK                 parserStk[PARSER_STK_SIZE];

typedef enum {P1,P2,P3,PLEN,D,CSUM,ER} ParserState;


/*----- C r e a t e P a r s e r T a s k ( ) -----

PURPOSE
Create and initialize the Parser Task.
*/
CPU_VOID CreateParserTask(CPU_VOID)
{
	OS_ERR		osErr;/* -- OS Error code */

	/* Create Parser task. */	
  OSTaskCreate(&parserTCB,
               "Processing Task",
               ParsePkt, 
               NULL,
               ParserPrio,
               &parserStk[0],
               PARSER_STK_SIZE / 10,
               PARSER_STK_SIZE,
               0,
               0,
               0,
               (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
               &osErr);

  assert(osErr == OS_ERR_NONE);
}




/*-----------------------Edited by Lalit Bhat-------------------------------------*/

CPU_VOID ParsePkt(CPU_VOID *data)
{
  OS_ERR osErr;
  static ParserState ParseState = P1; //current parser state
  CPU_INT16S x;                       //next state
  CPU_INT08U CheckSum;
  CPU_INT08U i = 0;
  CPU_INT16S erst;
  
  for(;;)
  {
    if(PutBfrClosed(&payloadBfrPair))
    {
      OSSemPend(&openPayloadBfrs,SuspentTimeout,OS_OPT_PEND_BLOCKING,NULL,&osErr);
      assert(osErr==OS_ERR_NONE);
    }

   //Get the next byte from the packet
     x = GetByte();
    PktBfr *pktBfr = (PktBfr *) PutBfrAddr(&payloadBfrPair);
    
    switch (ParseState)
    {
    case P1:
      if(x == P1Char) 
      {
        ParseState = P2;
        CheckSum = x;
      }
      else
      {
        ParseState = ER;
        erst = -1;
        pktBfr->dataLen=erst;           //preambe 1 error
        ClosePutBfr(&payloadBfrPair);
      }
      break;
      
    case P2:
      if(x == P2Char)
      {
        ParseState = P3;
        CheckSum = CheckSum^x;
      }
      else
      {
        ParseState = ER;
        erst = -2;                     //preambe 2 error
        pktBfr->dataLen=erst;   
        ClosePutBfr(&payloadBfrPair);
      }
      break;
      
    case P3:
      if( x == P3Char)
      {
        ParseState = PLEN;
        CheckSum = CheckSum^x;
      }
      else
      {
        ParseState = ER;
        erst = -3;                      //preambe 3 error
        pktBfr->dataLen=erst;
        ClosePutBfr(&payloadBfrPair);
      }
      break;
      
    case PLEN:
      CheckSum = CheckSum^x;
      pktBfr->dataLen = (x-HeaderLength);
      
      if(x<MinimumPacketLength)
      {
        ParseState = ER;
        erst = -5;                      //packet length error
        pktBfr->dataLen=erst;
        ClosePutBfr(&payloadBfrPair);
      }
      else
      {
        ParseState = D;
      }
    
      break;
      
    case D:
      pktBfr->dat[i++] = x;
      CheckSum = CheckSum^x;
      if(i>=pktBfr->dataLen-1)
      {
        ParseState = CSUM;
      }
      break; 
      
    case CSUM:
      CheckSum = CheckSum^x;
      if(CheckSum==CheckSumVerify)
      {
        ParseState = P1;
        ClosePutBfr(&payloadBfrPair);
        OS_ERR osErr;
        OSSemPost(&closedPayloadBfrs,OS_OPT_POST_1,&osErr);
        assert(osErr==OS_ERR_NONE);
        i=0;
        
      }
      else
      {     
        ParseState = ER;
        erst = -4;                      //check sum error
        pktBfr->dataLen=erst;
        ClosePutBfr(&payloadBfrPair);
      }
      break;
      
    case ER:/*If the byte goes in the error state then the code looks for the correct P1Char and P2Char*/
       CheckSum=0;
        i=0;      
      if(x==P1Char)
      {
        CheckSum ^= x;
        x=GetByte();
        if(x==P2Char)
        {
          CheckSum = CheckSum^P2Char;
          ParseState = P3;
          }
        }
      break;
    }
     if(PutBfrClosed(&payloadBfrPair))
     {
      OSSemPost(&closedPayloadBfrs, OS_OPT_POST_1, &osErr);
      assert(osErr == OS_ERR_NONE);
     
    }
  }
}
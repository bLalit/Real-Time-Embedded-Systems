/*=============== P k t P a r s e r . c ===============*/

/* Edited by: Lalit Bhat
Student ID: 01671833
FINAL PROJECT

BY:	George Cheney
		EECE472 / EECE572 Embedded Real Time Systems
		Electrical and Computer Engineering Dept.
		UMASS Lowell
*/
/*
PURPOSE
This is a dummy Parser Task used to test the serial I/O driver module. In the final version 
it will become the Packet Parser Task.

DEMONSTRATES
Multitasking
Semaphores

CHANGES
03-02-2017  - Created for Spring 2017
*/
#include "PktParser.h"
#include "includes.h"
#include "Globals.h"
#include "MemMgr.h"


#define DestinationAddr 1
//#define SuspendTimeout 0
#define P1Char 0x03
#define P2Char 0xAF
#define P3Char 0xEF
#define NullChar 0
#define MinimumPacketLength 8
#define CheckSumVerify 0
#define SuspentTimeout 0
#define HeaderLength 5

//----- c o n s t a n t    d e f i n i t i o n s -----

#define ParserPrio 6 // Parser task priority

/* Size of the Process task stack */
#define	PARSER_STK_SIZE     256 

/*----- g l o b a l s -----*/

// Process Task Control Block
static OS_TCB parserTCB;
OS_Q ParserQueue;
OS_Q FramerQueue;

//Que *iBfr = NULL; //parser test


/* Stack space for Process task stack */
static CPU_STK                 parserStk[PARSER_STK_SIZE];

typedef enum {P1,P2,P3,PLEN,D,CSUM,ER} ParserState;


/*----- C r e a t e P a r s e r T a s k ( ) -----

PURPOSE
Create and initialize the Parser Task.
*/
CPU_VOID CreateParserTask(CPU_VOID)
{
	OS_ERR osErr;/* -- OS Error code */

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
               100,
               0,
               (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
               &osErr);
  assert(osErr == OS_ERR_NONE);
  
  OSQCreate(&ParserQueue,"Parser Queue",PoolSize,&osErr);
  assert(osErr == OS_ERR_NONE);
  
  OSQCreate(&FramerQueue,"Parser Queue",PoolSize,&osErr);
  assert(osErr == OS_ERR_NONE);
}




/*-----------------------Edited by Lalit Bhat-------------------------------------*/

CPU_VOID ParsePkt(CPU_VOID *data)
{
  Que *iBfr = NULL; //comment for test
  
  OS_ERR osErr;
  static ParserState ParseState = P1; //current parser state
  CPU_INT16S x;                       //next state
  CPU_INT08U CheckSum;
  CPU_INT08U i = 0;
  CPU_INT08U erst;
  //CPU_INT08U PayloadLength = 0;
  
  
  for(;;)
  {

   //Get the next byte from the packet
     x = GetByte();
     if (iBfr == NULL)
       iBfr = Allocate();
     
//    PktBfr *pktBfr = (PktBfr *) PutBfrAddr(&payloadBfrPair);
    if (x>=0)
    {
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
        erst = 1;
        AddByte(iBfr, erst);
        AddByte(iBfr,0x0B);
        OSQPost(&FramerQueue, iBfr, sizeof(Que), OS_OPT_POST_FIFO, &osErr);
        assert(osErr == OS_ERR_NONE);
        iBfr=NULL;
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
        erst = 2;           //preambe 2 error
        AddByte(iBfr, erst);
        AddByte(iBfr,0x0B);
        OSQPost(&FramerQueue, iBfr, sizeof(Que), OS_OPT_POST_FIFO, &osErr);
        assert(osErr == OS_ERR_NONE);
        iBfr=NULL;
        
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
        erst = 3;                      //preambe 3 error
        AddByte(iBfr, erst);
        AddByte(iBfr,0x0B);
        OSQPost(&FramerQueue, iBfr, sizeof(Que), OS_OPT_POST_FIFO, &osErr);
        assert(osErr == OS_ERR_NONE);
        iBfr=NULL;
      }
      break;
      
    case PLEN:
      
      if(x<MinimumPacketLength)
      {
        ParseState = ER;
        erst = 5;                      //packet length error
        AddByte(iBfr,erst);
    
      }
      else
      {
        AddByte(iBfr,(x-HeaderLength));
        CheckSum = CheckSum^x;
        //PayloadLength = x-HeaderLength;
        ParseState = D;
        i=0;
      }    
      break;
      
    case D:
      
      i++;
      AddByte(iBfr,x);
      CheckSum = CheckSum^x;
      if(i  >= iBfr->bfr[0])
      {
        ParseState = CSUM;
      }

      break; 
      
    case CSUM:
      CheckSum = CheckSum^x;
      if(CheckSum==CheckSumVerify)
      {
        ParseState = P1;
        OSQPost(&ParserQueue,iBfr, sizeof(Que),OS_OPT_POST_FIFO, &osErr);
        assert(osErr==OS_ERR_NONE);
        iBfr = NULL;     
      }
      else
      {     
        ParseState = ER;
        erst = 4;                      //check sum error
        AddByte(iBfr, erst);
        AddByte(iBfr,0x0B);
        OSQPost(&ParserQueue,iBfr, sizeof(Que),OS_OPT_POST_FIFO, &osErr);
        assert(osErr==OS_ERR_NONE);
        iBfr = NULL;
      }
      break;
      
    case ER:/*If the byte goes in the error state then the code looks for the correct P1Char and P2Char*/
       //CheckSum=0;
       // i=0;      
      if(x==P1Char)
      {
        CheckSum = x;
        x=GetByte();
        if(x>0)
        {
          if(x==P2Char)
          {
            ParseState = P3;
             CheckSum ^= P2Char;
          }
        }
      }
      break;
    }
    }
  }
}
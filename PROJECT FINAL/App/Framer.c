/*--------------------------------Framer.c----------------------------------*/
/*Name: Lalit Bhat
Student ID:01671833
FINAL PROJECT
*/

#include "Framer.h"
#include "Queue.h"


//Globals
OS_TCB FramerTCB; //Framer Task Control Block.
//Que *oBfr = NULL; //Framer Test

CPU_STK FramerStk[FRAMER_STK_SIZE];

/*PURPOSE
Create and initialize the Parser Task.
*/
CPU_VOID CreateFramerTask(CPU_VOID)
{
	OS_ERR osErr;/* -- OS Error code */

	/* Create Parser task. */	
  OSTaskCreate(&FramerTCB,
               "Framer Task",
               FramerTask, 
               NULL,
               FramerPrio,
               &FramerStk[0],
               FRAMER_STK_SIZE / 10,
               FRAMER_STK_SIZE,
               0,
               0,
               0,
               (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
               &osErr);

  assert(osErr == OS_ERR_NONE);
}


CPU_VOID FramerTask(CPU_VOID *data)
{
  Que *oBfr = NULL;
  OS_MSG_SIZE msgSize;
  OS_ERR osErr;
  
  for(;;)
  {
    if(oBfr == NULL)
    {
      oBfr =  OSQPend(&FramerQueue,0,OS_OPT_PEND_BLOCKING,&msgSize,NULL,&osErr);
      assert(osErr==OS_ERR_NONE);
    }
    outPut *out = (outPut *)oBfr->bfr;

    switch(out->messageType)
    {
    case 0x07:
      PutByte(0x03);
      PutByte(0xaf);
      PutByte(0xef);
      PutByte(0x09);
      PutByte(out->PresentBot);
      PutByte(0x02);
      PutByte(out->messageType);
      PutByte(out->Error);
      PutByte(0x03^0xaf^0xef^0x09^out->PresentBot^0x02^out->messageType^out->Error);
      TxFlush();
      break;
    case 0x0A:
      PutByte(0x03);
      PutByte(0xaf);
      PutByte(0xef);
      PutByte(0x09);
      PutByte(0x01);
      PutByte(0x02);
      PutByte(out->messageType);
      PutByte(out->Error);
      PutByte(0x03^0xaf^0xef^0x09^0x01^0x02^out->messageType^out->Error);
      TxFlush();
      break;
    case 0x0B:
      PutByte(0x03);
      PutByte(0xaf);
      PutByte(0xef);
      PutByte(0x09);
      PutByte(0x01);
      PutByte(0x02);
      PutByte(out->messageType);
      PutByte(out->Error);
      PutByte(0x03^0xaf^0xef^0x09^0x01^0x02^out->messageType^out->Error);
      TxFlush();
      break;
    }
    Free(oBfr);
    oBfr = NULL;
  }
}
    
      
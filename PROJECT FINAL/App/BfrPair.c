/*-------------------------------------------BfrPair.c---------------------------------------------*/
/*Name: Lalit Bhat
Student ID:01671833
Program Number: 4
*/

#include "BfrPair.h"
#include "Buffer.h"

#define MaxSize 4
#define Null 0

/*Initialize both buffers of the buffer pair*/
void BfrPairInit(BfrPair *bfrPair, CPU_INT08U *bfr0Space, CPU_INT08U *bfr1Space, CPU_INT16U size)
{
  bfrPair->putBfrNum = Null;
  BfrInit(&bfrPair -> buffers[0],bfr0Space,size);
  BfrInit(&bfrPair -> buffers[1],bfr1Space,size);
}

/*Reset the put buffer*/
void PutBfrReset(BfrPair *bfrPair)
{
  BfrReset(&bfrPair->buffers [bfrPair->putBfrNum]);
}

/*Obtain the address of the put buffer's buffer data space*/
CPU_INT08U *PutBfrAddr(BfrPair *bfrPair)
{
  return(bfrPair->buffers[bfrPair->putBfrNum].buffer);
}

/*Obtain the address of the get buffer's buffer data space*/
CPU_INT08U *GetBfrAddr(BfrPair *bfrPair)
{
  return(bfrPair->buffers[1-bfrPair->putBfrNum].buffer);
}

/*Test wether or not the put buffer closed. */
CPU_BOOLEAN PutBfrClosed(BfrPair *bfrPair)
{
  return(BfrClosed(&bfrPair->buffers [bfrPair->putBfrNum]));
}

/*Test wether or not the get buffer closed. */
CPU_BOOLEAN GetBfrClosed(BfrPair *bfrPair)
{
  return(BfrClosed(&bfrPair->buffers [1-bfrPair->putBfrNum]));
}

/*Mark the put buffer closed*/
void ClosePutBfr(BfrPair *bfrPair)
{
  BfrClose(&bfrPair->buffers [bfrPair->putBfrNum]);
}

//----
CPU_BOOLEAN PutBfrEmpty(BfrPair *bfrPair)
{
  return(BfrEmpty(&bfrPair->buffers[bfrPair->putBfrNum]));
}

CPU_BOOLEAN GetBfrEmpty(BfrPair *bfrPair)
{
  return(BfrEmpty(&bfrPair->buffers[1-bfrPair->putBfrNum]));
}

/*Mark the get buffer open*/
void OpenGetBfr(BfrPair *bfrPair)
{
  BfrOpen(&bfrPair->buffers [1-bfrPair->putBfrNum]);
}

/*Add a byte to the put buffer at position "putIndex" and increment "putIndex" by 1. If the buffer becomes full, mark it closed*/
CPU_INT16S PutBfrAddByte (BfrPair *bfrPair, CPU_INT16S byte)
{
  return(BfrAddByte(&bfrPair->buffers [bfrPair->putBfrNum],byte));
}

/*Return the byte from position "getIndex" of the get buffer or return -1 if the get buffer is empty*/
CPU_INT16S GetBfrNextByte(BfrPair *bfrPair)
{
  return(BfrNextByte(&bfrPair->buffers [1-bfrPair->putBfrNum]));
}

/*Return the byte from position "getIndex" in the get buffer and increment the get buffer's "getIndex" by 1. If the buffer becomes empty, mark it open.*/
CPU_INT16S GetBfrRemByte(BfrPair *bfrPair)
{
  return(BfrRemByte(&bfrPair->buffers [1-bfrPair->putBfrNum]));
}

/*Test whether or not a buffer pair is ready to be swapped. It is ready if the put buffer is closed and the get buffer is open*/
CPU_BOOLEAN BfrPairSwappable(BfrPair *bfrPair)
{
  if (PutBfrClosed(bfrPair))
  {   
    if(!GetBfrClosed(bfrPair))
      return TRUE;
  }
    return FALSE;
}

/*Swap the put buffer and the get buffer, and reset the put buffer*/
void BfrPairSwap(BfrPair *bfrPair)
{
  bfrPair->putBfrNum = 1 - bfrPair->putBfrNum;
  PutBfrReset(bfrPair);
}

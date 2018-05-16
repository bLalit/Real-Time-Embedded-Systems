/*--------------------------------Buffer.c----------------------------------*/
/*Name: Lalit Bhat
Student ID:01671833
Program Number: 4
*/

#include "Buffer.h"

#define BufferArray 4
#define BufferMax 4
#define Null 0

//CPU_INT08U bfrSpace[BufferArray];

/*Initialize a buffer: record the size, set putIndex and getIndex to zero,
and mark buffer open.*/
void BfrInit (Buffer *bfr, CPU_INT08U *bfrSpace, CPU_INT16U size)
  {
    bfr->size = size;
    bfr->putIndex = Null;
    bfr->getIndex = Null;
    bfr->closed = FALSE;
    bfr->buffer = bfrSpace;
  }

/*Reset the buffer: set putIndex and getIndex to zero, and mark  the buffer open.*/
void BfrReset (Buffer *bfr)
  {
    bfr->putIndex = Null;
    bfr->getIndex = Null;
    bfr->closed = FALSE;
  }

/*Test wether or not a buffer is closed*/
CPU_BOOLEAN BfrClosed(Buffer *bfr)
  {
    if ((bfr->closed)!=FALSE)
    {
      return TRUE;
    }
    else
      return FALSE;
  }

/*Mark the buffer closed*/
void BfrClose(Buffer *bfr)
  {
    bfr->closed = TRUE;
  }

/*Mark the buffer open*/
void BfrOpen(Buffer *bfr)
  {
    bfr->closed = FALSE;
  }

/*Test whether or not a buffer is full*/
CPU_BOOLEAN BfrFull(Buffer *bfr)
  {
    if(bfr->putIndex >= bfr->size)
    {
      return TRUE;
    }
    else
      return FALSE;
  }

/*Test whether or not a buffer is empty*/
CPU_BOOLEAN BfrEmpty(Buffer *bfr)
  {
    if((bfr->getIndex) >= (bfr->putIndex))
    {
      return TRUE;
    }
    else 
      return FALSE;
  }

/*Add a byte to a buffer at position "putIndex" and increment "putIndex"
by 1. If buffer becomes full, mark it closed*/
CPU_INT16S BfrAddByte(Buffer *bfr, CPU_INT16S theByte)
{
  if (BfrFull(bfr))
  {
    return -1;
  }
  else 
    bfr->buffer[bfr->putIndex ++] = theByte;
      if (BfrFull(bfr))
      {
        bfr->closed = TRUE;
      }
    return (theByte);
}

/*Return the byte from position "getIndex" or return -1 if the buffer is empty*/
CPU_INT16S BfrNextByte(Buffer *bfr)
{
  if (BfrEmpty(bfr))
    return -1;
  else 
    return (bfr->buffer[bfr->getIndex]);
}

/*Return the byte from position "getIndex" and increment "getIndex" by 1.
If buffer becomes empty, mark it open*/
CPU_INT16S BfrRemByte(Buffer *bfr)
{
  CPU_INT08U temp;
  if (BfrEmpty(bfr))
    return -1;
  else
  {
    temp = bfr->buffer[bfr->getIndex++];
    if (BfrEmpty(bfr))
    {
      bfr->closed = FALSE;
    }
    return temp;
  }
}
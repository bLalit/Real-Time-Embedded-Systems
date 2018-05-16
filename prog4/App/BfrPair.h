/*--------------------------------BfrPair.h----------------------------------*/
/*Name: Lalit Bhat
Student ID:01671833
Program Number: 4
*/

#ifndef BFR_PAIR_H
#define BFR_PAIR_H

#include <includes.h>
#include "Buffer.h"
#pragma pack(1)

#define NumBfrs 2

typedef struct
{
  CPU_INT08U  putBfrNum;          /* -- The index of the put buffer */
  Buffer      buffers[NumBfrs];   /* -- The 2 buffers */
} BfrPair;

void BfrPairInit(BfrPair *bfrPair, CPU_INT08U *bfr0Space, CPU_INT08U *bfr1Space, CPU_INT16U size);
void PutBfrReset(BfrPair *bfrPair);
CPU_INT08U *PutBfrAddr(BfrPair *bfrPair);
CPU_INT08U *GetBfrAddr(BfrPair *bfrPair);
CPU_BOOLEAN PutBfrClosed(BfrPair *bfrPair);
CPU_BOOLEAN GetBfrClosed(BfrPair *bfrPair);
CPU_BOOLEAN PutBfrEmpty(BfrPair *bfrPair);//
CPU_BOOLEAN GetBfrEmpty(BfrPair *bfrPair);//
void ClosePutBfr(BfrPair *bfrPair);
void OpenGetBfr(BfrPair *bfrPair);
CPU_INT16S PutBfrAddByte(BfrPair *bfrPair, CPU_INT16S theByte);
CPU_INT16S GetBfrNextByte(BfrPair *bfrPair);
CPU_INT16S GetBfrRemByte(BfrPair *bfrPair);
CPU_BOOLEAN BfrPairSwappable(BfrPair *bfrPair);
void BfrPairSwap(BfrPair *bfrPair);

#endif
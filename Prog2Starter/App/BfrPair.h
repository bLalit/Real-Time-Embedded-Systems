/*-------------------------------------------BfrPair.h---------------------------------------------*/
/*Name: Lalit Bhat
Student ID:01671833
Program Number: 2
*/

#include "Buffer.h"
#ifndef BfrPair_H
#define BfrPair_H

#define NumBfrs 2

typedef struct 
{
  CPU_INT08U putBfrNum;
  Buffer buffers[NumBfrs];
}BfrPair;

/*------------------------Fuction Prototypes----------------------------------*/

void BfrPairInit(BfrPair *bfrPair, CPU_INT08U *bfr0Space, CPU_INT08U *bfr1Space, CPU_INT16U size);
void PutBfrReset(BfrPair *bfrPair);
CPU_INT08U *PutBfrAddr(BfrPair *bfrPair);
CPU_INT08U *GetBfrAddr(BfrPair *bfrPair);
CPU_BOOLEAN PutBfrClosed(BfrPair *bfrPair);
CPU_BOOLEAN GetBfrClosed(BfrPair *bfrPair);
void ClosePutBfr(BfrPair *bfrPair);
void OpenGetBfr(BfrPair *bfrPair);
CPU_INT16S PutBfrAddByte(BfrPair *bfrPair, CPU_INT16S byte);
CPU_INT16S GetBfrNextByte(BfrPair *bfrPair);
CPU_INT16S GetBfrRemByte(BfrPair *bfrPair);
CPU_BOOLEAN BfrPairSwappable(BfrPair *bfrPair);
void BfrPairSwap(BfrPair *bfrPair);

#endif

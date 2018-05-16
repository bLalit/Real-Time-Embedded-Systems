/*---------------------------------Buffer.h--------------------------------------*/
/*Name: Lalit Bhat
Student ID:01671833
Program Number: 2
*/

#ifndef Buffer_H
#define Buffer_H

#include "includes.h"

typedef struct
{
  volatile CPU_BOOLEAN closed;
  CPU_INT16U size;
  CPU_INT16U putIndex;
  CPU_INT16U getIndex;
  CPU_INT08U *buffer;
} Buffer;

/*------------------------Fuction Prototypes----------------------------------*/
void BfrInit (Buffer *bfr, CPU_INT08U *bfrSpace, CPU_INT16U size);
void BfrReset (Buffer *bfr);
CPU_BOOLEAN BfrClosed (Buffer *bfr);
void BfrClose (Buffer *bfr);
void BfrOpen (Buffer *bfr);
CPU_BOOLEAN BfrFull (Buffer *bfr);
CPU_BOOLEAN BfrEmpty  (Buffer *bfr);
CPU_INT16S BfrAddByte (Buffer *bfr, CPU_INT16S theByte);
CPU_INT16S BfrNextByte (Buffer *bfr);
CPU_INT16S BfrRemByte (Buffer *bfr);

#endif
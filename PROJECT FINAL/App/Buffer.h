/*=============== B u f f e r . h ===============*/

/*
BY:	George Cheney
		16.472 / 16.572 Embedded Real Time Systems
		Electrical and Computer Engineering Dept.
		UMASS Lowell
*/

/*
PURPOSE
Provides functions for operating buffers that store sequences of bytes.

CHANGES
02-14-2017  - Created for Spring 2017
*/

#ifndef Buffer_H
#define Buffer_H
#include "includes.h"

#pragma pack(1)

/*----- t y p e    b u f f e r -----

Defines a buffer data type for storing sequences of bytes.
*/
typedef struct
{
  volatile CPU_BOOLEAN closed;/* -- True if buffer has data ready to process */
  CPU_INT16U  size;           /* -- The capacity of the buffer in bytes */
  CPU_INT16U  putIndex;       /* -- The position where the next byte is added */
  CPU_INT16U  getIndex;       /* -- The position of the next byte to remove */
  CPU_INT08U  *buffer;        /* -- The address of the buffer space */
} Buffer;

//----- f u n c t i o n    p r o t o t y p e s -----

void BfrInit(Buffer *bfr, CPU_INT08U *bfrSpace, CPU_INT16U size);
void BfrReset(Buffer *bfr);
CPU_BOOLEAN BfrClosed(Buffer *bfr);
void BfrClose(Buffer *bfr);
void BfrOpen(Buffer *bfr);
CPU_BOOLEAN BfrFull(Buffer *bfr);
CPU_BOOLEAN  BfrEmpty(Buffer *bfr);
CPU_INT16S BfrAddByte(Buffer * bfr, CPU_INT16S theByte);
CPU_INT16S BfrNextByte(Buffer *bfr);
CPU_INT16S BfrRemByte(Buffer *bfr);
#endif
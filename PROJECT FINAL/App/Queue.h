
#ifndef QUEUE_H
#define QUEUE_H
#include <cpu.h>
/*=============== B u f f e r . h ===============*/

/*
BY:	    George Cheney
		16.472 / 16.572 Embedded Real Time Systems
		Electrical and Computer Engineering Dept.
		UMASS Lowell
*/

/*
PURPOSE
Provide a set of operations on buffers (not circular)


CHANGES
03-24-2011  - Release to class.
*/

/*----- c o n s t a n t    d e f i n i t i o n s -----*/

// Define the buffer capacity

#ifndef QueSize
#define QueSize 35
#endif

/*----- t y p e    d e f i n i t i o n s -----*/

// Define the buffer type

typedef struct
{
  CPU_INT08U  *in;             // Points to space to add next byte
  CPU_INT08U  *out;            // Points to next byte to remove
  CPU_INT08U   bfr[QueSize];    // The buffer storage area
}Que;


/*----- f u n c t i o n    p r o t o t y p e s -----*/

CPU_BOOLEAN Empty(Que *theBfr);
CPU_BOOLEAN Full(Que *theBfr);
CPU_INT16S AddByte(Que *theBfr, CPU_INT16S theByte);
CPU_INT16S RemoveByte(Que *theBfr);
void InitBfr(Que *bfr);
#endif

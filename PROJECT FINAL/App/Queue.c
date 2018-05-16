/*=============== B u f f e r . c ===============*/

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
03-25-2010  - Release to class.
*/

#include "includes.h"

#include "Queue.h"

/*--------------- E m p t y ( ) ---------------*/

/*
PURPOSE
Test for an empty buffer.

INPUT PARAMETERS
theBfr  -- the address of the buffer to test

RETURN VALUE
true if the buffer is empty
false otherwis
*/

CPU_BOOLEAN Empty(Que *theBfr)
{
	return theBfr->out >= theBfr->in;
}

/*--------------- F u l l ( ) ---------------*/

/*
PURPOSE
Test for an full buffer.

INPUT PARAMETERS
theBfr  -- the address of the buffer to test

RETURN VALUE
true if the buffer is full
false otherwise
*/

CPU_BOOLEAN Full(Que *theBfr)
{
	return theBfr->in >= theBfr->bfr+BfrSize;
}

/*--------------- A d d B y t e ( ) ---------------*/

/*
PURPOSE
Add a byte to a buffer at the position "in," and increment "in."

INPUT PARAMETERS
theBfr  -- the address of the buffer where the byte is to be added
theByte -- the byte to add

RETURN VALUE
If the add failed due to a full buffer, return false;
otherwise, return the byte that was added.
*/

CPU_INT16S AddByte(Que *theBfr, CPU_INT16S theByte)
{
	if (Full(theBfr))
		return -1;
	
	*theBfr->in++ = theByte;
	
	return theByte;
}

/*--------------- R e m o v e B y t e ( ) ---------------*/

/*
PURPOSE
Remove the byte at position"out" from the buffer and increment "out."

INPUT PARAMETERS
theBfr  -- the address of the buffer from which the byte is to be removed

RETURN VALUE
If the remove failed due to an empty buffer, return false;
otherwise, return the byte that was removed.
*/

CPU_INT16S RemoveByte(Que *theBfr)
{
	if (Empty(theBfr))
		return -1;
		
	return *theBfr->out++;
}

/*--------------- I n i t B f r ( ) ---------------*/

/*
PURPOSE
Initialize a buffer. Reset "in" and "out" to the beginning of the buffer.

INPUT PARAMETERS
theBfr  -- the address of the buffer from which the byte is to be removed
*/

void InitBfr(Que *theBfr)
{
	theBfr->in = theBfr->bfr;
	theBfr->out = theBfr->bfr;
}


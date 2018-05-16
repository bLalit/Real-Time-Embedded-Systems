#include "includes.h"
#include "Intrpt.h"

/*--------------- I n t r p t . c ---------------

By: George Cheney
    ECE Dept.
    UMASS Lowell

PURPOSE
Defines global variable "disableCnt" to track nested
interrupt disables. 

CHANGES
02-10-2014  gpc - Updated for spring 2014 16.572 Program 3
*/

// Level of nesting of interrupt disables.
static CPU_INT16S disableCnt = 0;

/*--------------- I n t D i s ( ) ----------

PURPOSE
Set PRIMASK = 1 to disable interrupts.
*/

void IntDis(void)
{
 	asm(" cpsid i");
	++disableCnt;
}

/*--------------- I n t E n ( ) ----------

PURPOSE
Clear PRIMASK = 0 to enable interrupts.
*/

void IntEn(void)
{
	if (disableCnt)
		--disableCnt;
 	if (disableCnt == 0)
		asm(" cpsie i");
}

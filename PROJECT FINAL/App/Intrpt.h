#ifndef Intrpt_H
#define Intrpt_H

/*--------------- I n t r p t . h ---------------

By: George Cheney
    ECE Dept.
    UMASS Lowell

PURPOSE
Provides functions to enable and disable interrupts.

CHANGES
02-10-2014  gpc - Updated for spring 2014 16.572 Program 3
*/

#define INLINE_INTRPT_FCNS

#include "CPU.h"

// Level of nesting of interrupt disables.
extern CPU_INT16S disableCnt;

/*----- f u n c t i o n     p r o t o t y p e s -----*/
void IntDis(void);
void IntEn(void);
#endif
/*=============== G l o b a l s . h ===============*/

//Edited by: Lalit Bhat
/*
BY:	George Cheney
		16.472 / 16.572 Embedded Real Time Systems
		Electrical and Computer Engineering Dept.
		UMASS Lowell

PURPOSE
Define QDemo globals

CHANGES
03-25-2010  - Release to class.
*/

#ifndef GLOBALS_H
#define GLOBALS_H

/*----- c o n s t a n t    d e f i n i t i o n s -----*/

//#define SuspendTimeout 0	/* -- Timeout suspended task if not awakened. */

/*
Task priorities
*/
enum t_prio
{
	InitPrio = 1,		  /* -- Initialization task */
	InputPrio = 2,	  /* -- Input task */
	OutputPrio = 3,	  /* -- Output task */
	ProcessPrio = 4		/* -- Processing task */
};

//extern OS_Q	ParserQueue;
//extern OS_Q     FramerQueue;


#endif
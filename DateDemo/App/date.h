#ifndef Date_H
#define Date_H
/*--------------- d a t e . h ---------------*/

/*
by:	George Cheney
	Embedded Real Time Systems
	Electrical and Computer Engineering Dept.
	UMASS Lowell
*/

/*
PURPOSE
This header file defines the public names (functions and types)
exported from the module "datetype.c."

CHANGES
02-03-2010 gpc - Ported to STM32F107 Eval. Board
02-02-2011 gpc - Updated for new versions of uC/OS-III and IAR EWB
01-19-2018 gpc - Updated for Spring 2018
*/

/*----- c o n s t a n t    d e f i n i t i o n s -----*/

#define MonthLength 3 /* --   Size of month mnemonic 
                              not including the '\0' terminator */

/*----- t y p e    d e f i n i t i o n s -----*/

/*
Three character month mnemonic
*/

typedef CPU_CHAR Month[MonthLength+1];

/*
Date
*/
typedef struct
{
	CPU_INT08U	day;		/* -- Day of the month */
	Month		month;		/* -- Month mnemonic */
	CPU_INT16U	year;		/* -- Year number */
} Date;

/*----- f u n c t i o n    p r o t o t y p e s -----*/

void PutDate(Date theDate);
CPU_BOOLEAN GetDate(CPU_INT08S *prompt, Date *theDate);
CPU_INT16U DayOfYear(Date d);

#endif
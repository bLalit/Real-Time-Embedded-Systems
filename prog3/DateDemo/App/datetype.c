/*--------------- d a t e t y p e . c ---------------*/

/*
by:	George Cheney
	Embedded Real Time Systems
	Electrical and Computer Engineering Dept.
	UMASS Lowell
*/

/*
PURPOSE
Provide a data type for representing calendar dates,
along with functions to implement operations on dates.

CHANGES
02-03-2010 gpc - Ported to STM32F107 Eval. Board
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include <CPU.h>

#include "includes.h"

#include "datetype.h"

/*----- c o n s t a n t    d e f i n i t i o n s -----*/

#define LeapYrInterval  4   /* Spacing between leap years */
#define FebMonthNumber  1   /* Month numbers start at zero. */
#define NumStrBfrSize   20  /* Size of buffer for reading numbers */

/*--------------- P u t D a t e ( ) ---------------*/

/*
PURPOSE
Display a calendar date on the screen.

INPUT PARAMETERS
theDate	- the date to be displayed
*/

void PutDate(Date theDate)
{
	BSP_Ser_Printf("%s", theDate.month);
	BSP_Ser_Printf(" %hu, %hu", (CPU_INT16U) theDate.day, theDate.year);
}

/*--------------- G e t D a t e ( ) ---------------*/

/*
PURPOSE
Read a date from the keyboard.

INPUT PARAMETERS
prompt -	the prompt identifying what date is to be 

OUTPUT PARAMETERS
theDate	-	the address into which to return the date.

RETURN VALUE
	TRUE  if a date was entered
	FALSE if the user entered an empty line to cancel input
*/

CPU_BOOLEAN GetDate(CPU_INT08S *prompt, Date *theDate)
{
	Month         theMonth;   /* -- Read in month here. */
	CPU_INT16U    theDay;	  /* -- Read in day here. */
	CPU_INT16U    theYear;	  /* -- Read in the year here. */
	CPU_INT08U    i;  /* -- Month letter index for capitalization loop */
        
        CPU_CHAR      numStr[NumStrBfrSize];  /* Numeric input buffer */

/*	Read in the month mnemonic, and terminate if empty. */
	BSP_Ser_Printf("\nEnter %s: ", prompt);	
	BSP_Ser_Printf("\nMonth: ");
	BSP_Ser_RdStr(theMonth, sizeof(theMonth));
	if (strlen(theMonth) == 0)
		return FALSE;

/*	Capitalize the month letters. */
	for (i=0; i<strlen(theMonth); i++)
		theMonth[i] = toupper(theMonth[i]);

/*	Read in the day of the month. */
	BSP_Ser_Printf("Day: ");
        BSP_Ser_RdStr(numStr, sizeof(numStr));
	sscanf(numStr, "%hu", &theDay);

/*	Read in the year. */
	BSP_Ser_Printf("Year: ");
        BSP_Ser_RdStr(numStr, sizeof(numStr));
	sscanf(numStr, "%hu", &theYear);

/*	Copy buffered date settings to the returned date. */
	strcpy(theDate->month, theMonth);
	theDate->day =  (CPU_INT08U) theDay;
	theDate->year = theYear;
	
	return TRUE;
}



/*--------------- D a y O f Y e a r( ) ---------------*/

/*
PURPOSE
Given a date, determine the day of the year.

INPUT PARAMETERS
theDate	- the date whose day number is to be found

RETURN VALUE
	the day of the year for the given date
*/

CPU_INT16U DayOfYear(Date theDate)
{
    /* Month names for computing the month number */
    static CPU_CHAR months[] = "JANFEBMARAPRMAYJUNJULAUGSEPOCTNOVDEC";

    /* Number of days in each month, if not a leap year. */
    static CPU_INT08U days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    CPU_INT08U  i;	  /* -- Index to array of month lengths */
    CPU_CHAR    *monthPtr;/* -- Pointer to month mnemonic */
    CPU_INT08U  monthNum; /* -- Month number */
    CPU_INT16U  dayNum;	  /* -- Day of the year */
	
/*  Determine the month number. */
    monthPtr = strstr(months, theDate.month);
    assert(monthPtr != NULL);
    monthNum = (CPU_INT08U) (monthPtr - months) / MonthLength;

/*  Determine the the number of days until the current month. */
    dayNum=0;
    for (i=monthNum; i>0; --i)
      dayNum += days[i-1];
            
/*  Add in the day number of the current month. */
    assert(theDate.day <= days[monthNum]);
    dayNum += theDate.day;

/*  Correct for leap years. */
    if (  theDate.year % LeapYrInterval == 0 && 
          monthNum > FebMonthNumber)    	        /** -- Note Y2K Bug! */
      ++dayNum;
                            
    return dayNum;
}



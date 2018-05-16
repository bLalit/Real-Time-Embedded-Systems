/*--------------- D a t e D e m o . c ---------------*/

/*
by:	George Cheney
	  Embedded Real Time Systems
	  Electrical and Computer Engineering Dept.
	  UMASS Lowell

PURPOSE
Read in two dates from the keyboard, determine the day of the year for
each date, and compute the difference.

CHANGES
01-19-2018 gpc - Updated for Spring 2018
*/

/* Include Micrium and STM headers. */
#include "includes.h"

/* Include Date module header. */
#include "date.h"

/*----- c o n s t a n t    d e f i n i t i o n s -----*/

#define BaudRate 9600           /* RS232 Port Baud Rate */
  
/*----- f u n c t i o n    p r o t o t y p e s -----*/

int AppMain();

/*--------------- m a i n ( ) -----------------*/

CPU_INT32S main()
{
    CPU_INT32S	exitCode;       // Return this code on exit.
	
//  Initialize the STM32F107 eval. board.
    BSP_IntDisAll();            /* Disable all interrupts. */

    BSP_Init();                 /* Initialize BSP functions */

    BSP_Ser_Init(BaudRate);     /* Initialize the RS232 interface. */

//  Run the application.    
    exitCode = AppMain();
    
    return exitCode;
}

/*--------------- A p p M a i n ( ) -----------------*/

CPU_INT32S AppMain()
{
    Date	date1;		/* -- First date */
    Date	date2;		/* -- Second date */
    CPU_INT16U	dayNum1;	/* -- Day of year for first date */
    CPU_INT16U	dayNum2;	/* -- Day of year for second date */

/*
    Repeatedly read in two dates, determine their day numbers,
    and the difference.
*/
    while (GetDate("First date", &date1))
      {
      /* Read second date; quit on empty. */
      if(!GetDate("Second date", &date2))
        break;

      /* Display the first date, and its day of the year number. */
      BSP_Ser_Printf("\nThe first date is: ");
      PutDate(date1);
      dayNum1 = DayOfYear(date1);
      BSP_Ser_Printf("\nFirst Day Number = %u", dayNum1);

      /* Display the second date, and its day of the year number. */
      BSP_Ser_Printf("\nThe second date is: ");
      PutDate(date2);
      dayNum2 = DayOfYear(date2);
      BSP_Ser_Printf("\nSecond Day Number = %u", dayNum2);

      /* Compute and display the difference. */
      BSP_Ser_Printf("\nDifference = %u", dayNum2-dayNum1);
      }

    return 0;
}
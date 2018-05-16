/*--------------- P r o g 2 . c ---------------

by: George Cheney
    ECE Dept.
    UMASS Lowell

PURPOSE
Read a text file from the RS232 Rx, "process" it, and output the result to the RS22 Tx.

DEMONSTRATES
Cooperative multitasking
Concurrent, polled I/O

CHANGES
01-19-2017 gpc -  Created for EECE472 / EECE572
01-25-2017 gpc -  Fixed call to PutByte() in function Process() to check >= 0, not > 0.
01-25-2017 gpc -  Don't include BfrPair.h.
01-29-2018 gpc -  Updated for spring 2018
*/
#include <ctype.h>

#include "includes.h"
#include "SerIODriver.h"
#include "Intrpt.h"

/*----- c o n s t a n t    d e f i n i t i o n s -----*/

// Define RS232 baud rate.
#define BaudRate 9600

/*----- f u n c t i o n    p r o t o t y p e s -----*/

void AppMain(void);

/*--------------- m a i n ( ) -----------------*/

CPU_INT32S main()
{
//  Initialize the STM32F107 eval. board.
    IntDis();                   /* Disable all interrupts. */

    BSP_Init();                 /* Initialize BSP functions */

    BSP_Ser_Init(BaudRate);     /* Initialize the RS232 interface. */

//  Run the application.    
    AppMain();
    
    return 0;
}

//#define TxTest // Uncomment to enable Tx Test mode
//#define RxTest // Uncomment to enable Rx Test mode

#if defined(TxTest)
/*--------------- A p p M a i n ( ) ---------------

PURPOSE
Test the operation of the RS232 Tx.
*/
void AppMain(void)
{
  // Create and Initialize iBfr and oBfr.
  InitSerIO();
  
  // Repeatedly output alphabets to the Tx.
  for (;;)
    {
    static CPU_INT16S c = 'A';
    
    while (TRUE)
      {
      if (PutByte(c) < 0)
        break;
      
      if (c < 'Z')
        c++;
      else
        c = 'A';
      }
    // If possible, output a byte from the oBfrPair Get Buffer to the UART Tx.
    ServiceTx();
    }
}
#elif defined(RxTest)
/*--------------- A p p M a i n ( ) ---------------

PURPOSE
Test the operation of the RS232 Tx and Rx together.
*/
void AppMain(void)
{
  CPU_BOOLEAN putPending = FALSE;

  InitSerIO();

  // Repeatedly read a byte from the Rx and echo it to the Tx.
  while(TRUE)
    {
    CPU_INT16S c;
      
    ServiceRx();
    for (;;)
      {
      if (!putPending)
        c = GetByte();
      if (c < 0)
        break;
      if (PutByte(c) < 0)
        {
        putPending = TRUE;
        break;
        }
      else
        putPending  = FALSE;
      }
    ServiceTx();
    }
}
#else
/*--------------- P r o c e s s ( ) ---------------

PURPOSE
This function performs the processing of input data to output data. Input bytes may
be output as follows.

1. If "NumDupes" is zero, output each input byte once,
2. If "NumDupes" is positive, output each byte and then duplicate it "NumDupes" times, or
3. If "NumDupes" is negative, output the bytes skipping "-NumDupes" bytes at a time.

Also, "ProcTime" may be set to simulate time spent processing an input byte to 
produce an output byte.
*/
void Process(void)
{
  /* If NumDupes is not defined outside of the code, define it. */  
  #ifndef NumDupes  
  #define NumDupes 0      /* Number of times to repeat or skip bytes */
  #endif
    
  /* If ProcTime is not defined outside of the code, define it. */  
  #ifndef ProcTime  
  #define ProcTime 0      /* Number of counts to delay to simulate processing time */
  #endif
  
  static CPU_INT08U outsLeft = 0;       /* Output repetition counter */  
  static CPU_INT16S c;                  /* Next character */
  static CPU_INT32U count = ProcTime;   /* Counter to simulate time spent in processing */
  static CPU_INT08U numSkips = (NumDupes<0) ? -NumDupes : 0;/* Number of input bytes to skip */

  /* If there are no bytes left to output, try to read a new one. */
  if (outsLeft == 0)
    {
    // Try to read the next character.
    c = GetByte();
    
    // Check whether or not a byte was available and not skipping.
    if (c < 0 || numSkips > 0)
      {
      if (c >= 0)
        // If character is skipped, reduce the skip count.
        --numSkips;
      return;                   // No byte available or skipped
      }
    }
  
  // If done skipping input, reset skip counter.
  if (numSkips == 0)
    numSkips = (NumDupes<0) ? -NumDupes : 0;

  // Simulate time spent processing input byte to output byte.
  while (count > 0)
    --count;
  count = ProcTime;
  
  // If starting a new output byte, reset output counter.
  if (outsLeft == 0)
    outsLeft = ((NumDupes>0) ? NumDupes : 0) + 1;
  
  // If output characters are left, try to output one.
  // 01-25-2017 gpc -  Fixed call to PutByte() in function Process() to check >= 0, not > 0.
  if(PutByte(toupper(c)) >= 0)
    --outsLeft;         // If "NumDupes" copies have been output, reset.
}

/*--------------- A p p M a i n ( ) ---------------

PURPOSE
This is the application main program.
*/
void AppMain(void)
{
  // Create and Initialize iBfrPair and oBfrPair.
  InitSerIO();
  
  //Enable interrupts
  IntEn();

  // Multitasking Executive Loop: Tasks are executed round robin.
  for (;;)
  {
    // Process iBfrPair to oBfrPair
    Process();
  }
}
#endif

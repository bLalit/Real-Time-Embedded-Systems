/*--------------------------------Error.c----------------------------------*/
/*Name: Lalit Bhat
Student ID:01671833
Program Number: 4
*/
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <cpu.h>

#include "Error.h"
#include "includes.h"


void Errortp(CPU_INT16S erst) /*srst here tells us the kind of error*/
{
   switch(erst)
   {
   case -1:                                             //Priamble Byte 1 Error.
     SerPrintf("\n *** ERROR: Preamble Bye 1, Error code: -1 \n"); 
      break;
      
   case -2:                                             //Priamble Byte 2 Error.
      SerPrintf("\n *** ERROR: Preamble Bye 2, Error code: -2\n");
      break;
   
   case -3:                                             //Priamble Byte 3 Error.
     SerPrintf("\n *** ERROR: Preamble Bye 3, Error code:  -3\n");
      break;
      
   case -4:                                             //CheckSum Error.
      SerPrintf("\n *** ERROR: Checksum Error, Error code: -4\n");
      break;
      
   case -5:                                             //Packet Length Error
      SerPrintf("\n *** ERROR: Packet Length Error, Error code: -5 \n");
      break;
   
   default:  
      SerPrintf("\n*** ERROR: Unknown Message Error \n");
      break;
   
   }
}
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <cpu.h>

#include "Error.h"
#include "includes.h"


void Errortp(CPU_INT08U erst) /*srst here tells us the kind of error*/
{
   switch(erst)
   {
   case P1Error:
      BSP_Ser_Printf("\n *** ERROR: Preamble Bye 1 \n");
      break;
      
   case P2Error:
      BSP_Ser_Printf("\n *** ERROR: Preamble Bye 2 \n");
      break;
   
   case P3Error:
      BSP_Ser_Printf("\n *** ERROR: Preamble Bye 3 \n");
      break;
      
   case ChecksumError:
      BSP_Ser_Printf("\n *** ERROR: Checksum Error \n");
      break;
      
   case PacketLengthError:
      BSP_Ser_Printf("\n *** ERROR: Packet Length Error \n");
      break;
   
   case DestinationAddr:
      BSP_Ser_Printf("\n *** ERROR: Destination Address Error \n");
      break;

   case UnknownMsg:
      BSP_Ser_Printf("\n*** ERROR: Unknown Message Error \n");
   
   }
}
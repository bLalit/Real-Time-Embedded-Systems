#include <stdio.h>
#include <stdlib.h>

#include "includes.h"
#include "Error.h"
#include "ProjGlobals.h"
#include "PktParser.h"

#define BaudRate 9600

#pragma pack (1)



#pragma pack (1)



typedef union
{
	CPU_INT08S		temp;		// The temperature report data
	CPU_INT16U		pres;		// Barometric pressure report
	struct					// The humidity report data
		{
		CPU_INT08S	dewPt;	// Dew Point
		CPU_INT08U		hum;		// Humidity
		} hum;
	struct					// Wind report data
		{
		CPU_INT08U		speed[2];// Wind speed
		CPU_INT16U	dir;		// Wind direction
		} wind;
	CPU_INT08U			depth[2];// Precipitation depth
	CPU_INT16U		rad;		// Solar radiation report
	CPU_INT32U		dateTime;// Date/Time stamp
	CPU_INT08U			id[10];	// Sensor node ID string
}  DataPart;

typedef struct
{
   CPU_INT08U          srcAddr;    // Address of source	
   CPU_INT08U        dstAddr;
   CPU_INT08U				dataLen;	   // Payload length
	CPU_INT08U				msgType;	   // The message type
   DataPart       dataPart;   // The data part of the message
} Payload;

void ProcessPkt(Payload *payload);
CPU_INT16U ByteSwap(CPU_INT16U word);
CPU_INT32U Extract(CPU_INT32U packedData, CPU_INT08U lsb, CPU_INT08U width);
CPU_INT16U BCDToINT16U(CPU_INT08U *bcd);
void SayMsgType(CPU_INT08U src, CPU_INT08U type);
void ShowWindMsg(DataPart *data);
void ShowPrecMsg(DataPart *data);
void ShowDateTimeMsg(DataPart *data);
void ShowIDMsg(CPU_INT08U length, DataPart *data);

void AppMain(void);

/*--------------- m a i n ( ) -----------------*/

CPU_INT32S main()
{
    BSP_IntDisAll();            /* Disable all interrupts. */

//  Initialize the STM32F107 eval. board.
    BSP_Init();                 /* Initialize BSP functions */

    BSP_Ser_Init(BaudRate);     /* Initialize the RS232 interface. */

    InitSerIO();                /* Initialize I/O drivers */
  
//  Run the application.    
    AppMain();
    
    return 0;
}

void AppMain(void)
{
#if 1
  CPU_BOOLEAN putPending = FALSE;
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
      else if (c == '\r')
        {
        c = '\n';
        putPending = TRUE;
        }
      else
        putPending  = FALSE;
      }
    ServiceTx();
    }
#else
  Payload  payloadBfr;

   for (;;)
      {
      ParsePkt(&payloadBfr);
      ProcessPkt(&payloadBfr);
      }
#endif   
}

void ProcessPkt(Payload *payload)
{
   DataPart *data = &payload->dataPart;   // Points to data part of the payload

   // Check message validity.
   if (  payload->msgType < TempMsg ||
         payload->msgType > MaxMsgType)
      {
      Error("EUnknown Message Type");
      return;
      }

   // Say what message type.
   SayMsgType(payload->srcAddr, payload->msgType-TempMsg);

   // Process the specific message.
   switch(payload->msgType)
      {
      case TempMsg:  // Temperature Message
			BSP_Ser_Printf("\n  Temperature = %ld", data->temp);
         break;
      case HumMsg:   // Humidity Message
			BSP_Ser_Printf("\n  Dew Point = %ld Humidity = %u",
                  data->hum.dewPt, data->hum.hum);
         break;
      case PresMsg:  // Barometric Pressure Message
			BSP_Ser_Printf("\n  Pressure = %lu", ByteSwap(data->pres));
         break;
		case RadMsg:   // Solar Radiation Intensity Message
			BSP_Ser_Printf("\n  Solar Radiation Intensity = %lu", ByteSwap(data->rad));
			break;
		case WindMsg:  // Wind Message
         ShowWindMsg(data);
			break;
		case PrecMsg:  // Precipitation Message
			ShowPrecMsg(data);
			break;
      case DateTimeMsg: // Date/Time Message
         ShowDateTimeMsg(data);
         break;
		case IDMsg:    
			ShowIDMsg(payload->dataLen-4, data);
			break;
      }
	BSP_Ser_Printf("\n");
}

CPU_INT16U ByteSwap(CPU_INT16U word)
{
//   return ((word & ByteMask) << BitsPerByte) | ((word >> BitsPerByte) & ByteMask);
   return (CPU_INT16U) Extract(word, 0, BitsPerByte) << BitsPerByte | (CPU_INT16U) Extract(word, BitsPerByte, BitsPerByte);
}

CPU_INT32U Extract(CPU_INT32U packedData, CPU_INT08U lsb, CPU_INT08U width)
{
   return (packedData >> lsb) & (~(0xFFFFFFFF << width));
}



#if 1
CPU_INT16U BCDToINT16U(CPU_INT08U *bcd)
{
  CPU_INT16U result = (bcd[0] & 0x0F) * 100;
	result += ((bcd[0] >> 4) & 0x0F) * 1000;
	result +=	(bcd[1] & 0x0F);
	result += ((bcd[1] >> 4) & 0x0F) * 10;

   return result;
}
#elif
CPU_INT16U BCDToINT16U(CPU_INT08U *bcd)
{
   CPU_INT16U result = (bcd[0] & 0x0F) * 10;
	result += ((bcd[0] >> 4) & 0x0F);
	result +=	(bcd[1] & 0x0F) * 1000;
	result += ((bcd[1] >> 4) & 0x0F) * 100;

   return result;
}
#else
CPU_INT16U BCDToINT16U(CPU_INT08U *bcd)
{
   CPU_INT16U result = bcd[0] & 0x0F;
	result += ((bcd[0] >> 4) & 0x0F) * 10;
	result +=	(bcd[1] & 0x0F) * 100;
	result += ((bcd[1] >> 4) & 0x0F) * 1000;

   return result;
}
#endif
void SayMsgType(CPU_INT08U src, CPU_INT08U type)
{
   const CPU_INT08U *CmdID[] = 
      {
      "TEMPERATURE MESSAGE",
      "BAROMETRIC PRESSURE MESSAGE",
      "HUMIDITY MESSAGE",
      "WIND MESSAGE",
      "SOLAR RADIATION MESSAGE",
      "DATE/TIME STAMP MESSAGE",
      "PRECIPITATION MESSAGE",
		"SENSOR ID MESSAGE"
      };

   BSP_Ser_Printf("\nSOURCE NODE %lu: %s", src, CmdID[type]);
}

void ShowWindMsg(DataPart *data)
{
   CPU_INT16U speed;
   speed = BCDToINT16U(data->wind.speed);
   BSP_Ser_Printf("\n  Speed = %lu.%lu Wind Direction = %lu",
            speed/10, speed%10, data->wind.dir);
}
void ShowPrecMsg(DataPart *data)
{
   CPU_INT16U depth;
   depth = BCDToINT16U(data->depth);
   BSP_Ser_Printf("\n  Precipitation Depth = %u.%02u  ", depth/100, depth%100);
}

void ShowDateTimeMsg(DataPart *data)
{
   CPU_INT32U   dateTime =  Extract(data->dateTime, 24, 8) |
                        (Extract(data->dateTime, 16, 8) << 8) |
                        (Extract(data->dateTime, 8, 8) << 16) |
                        (Extract(data->dateTime, 0, 8) << 24);

   BSP_Ser_Printf("\n  Time Stamp = %d/%d/%d",Extract(dateTime, MonthBit, 4),
                        Extract(dateTime, DayBit, 5),
                        Extract(dateTime, YearBit, 12));
   BSP_Ser_Printf(" %d:%d", (dateTime >> HourBit) & HourMask,
            (dateTime >>MinBit) & MinMask);
}
void ShowIDMsg(CPU_INT08U length, DataPart *data)
{
   CPU_INT08U	i;

   BSP_Ser_Printf("\n  Node ID = ");
   for (i=0; i<length; i++)
	   BSP_Ser_WrByte(data->id[i]);
}

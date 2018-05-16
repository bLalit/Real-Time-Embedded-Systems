#include <stdio.h>
#include <ctype.h>
#include <cpu.h>
#include <string.h>
#include <assert.h>
#include "includes.h"
#include "Error.h"
#include "PktParser.h"

#define BaudRate 9600
#define DestinationAdrs 1
#define WindSpeedArray 2
#define DepthArray 2
#define IdArray 10
#define Array0 0
#define Array1 1
#define ShiftBy8Pos 8
#define Mask8Bits 0xff
#define ShiftBy4Pos 4
#define Mask4Bits 0x0f
#define ShiftBy24Pos 24
#define DateMask1 0xff000000
#define DateMask2 0x00ff0000
#define DateMask3 0x0000ff00
#define DateMask4 0x000000ff
#define ShiftBy5Pos 5
#define ShiftBy9Pos 9
#define ShiftBy21Pos 21
#define ShiftBy27Pos 27
#define MonthMask 0x0000000f
#define DayMask 0x0000001f
#define YearMask 0x00000fff
#define HourMask 0x0000001f
#define MinuteMask 0x0000003f

/*----------------------- function prototypes -------------------*/
void AppMain(void);

/*------------------------- main () -----------------------------*/
CPU_INT32S main()
{ 
  //Initialie the STM32F107 eval. board.
  BSP_IntDisAll();              /*Disable all interrupts*/
  BSP_Init();                   /*Initialoze BSP functions*/
  BSP_Ser_Init(BaudRate);       /*Initialize the RS232 interface*/
  
  //Run the application
  AppMain();
  return 0;
}

#pragma pack(1) 
typedef struct                   /*Payload Data Structure*/
{
  CPU_INT08U payloadLen;
  CPU_INT08U dstAddr;
  CPU_INT08U srcAddr;
  CPU_INT08U msgType;
  union
    {
    CPU_INT08S temp;
    CPU_INT16U pres;
    struct
      {
      CPU_INT08U dewPt;
      CPU_INT08U hum;
      }hum;
    struct
      {
      CPU_INT08U speed[WindSpeedArray];
      CPU_INT16U dir;
      }wind;
    CPU_INT16U rad;
    CPU_INT32U dateTime;
    CPU_INT08U depth[DepthArray];
    CPU_INT08U id[IdArray];
   } dataPart;
}Payload;

enum {TemperatureMessage,BarometricPressureMessage,HumidityMessage,WindMessage,SolarRadiationMessage,DateTimeMessage,PrecipitationMessage,IdMessage} TypeOfMessage;
Payload payloadBuffer;

 CPU_INT32U Msb; //MSB of Date/Time 
 CPU_INT32U Smsb; //SSB of Date/Time
 CPU_INT32U Tmsb; //TSB of Date/Time
 CPU_INT32U Lsb; //LSB of Date/Time


void PrintMsg()
{
  switch(payloadBuffer.msgType)
  {
    case TemperatureMessage:
       BSP_Ser_Printf("\nSOURCE NODE %d: TEMPERATURE MESSAGE \n Temperature = %d \n",payloadBuffer.srcAddr,payloadBuffer.dataPart.temp);
        break;
                          
     case BarometricPressureMessage:
        BSP_Ser_Printf("\nSOURCE NODE %d: BAROMETRIC PRESSURE MESSAGE \n Pressure = %d \n",payloadBuffer.srcAddr,(((payloadBuffer.dataPart.pres>>ShiftBy8Pos)& Mask8Bits)|((payloadBuffer.dataPart.pres & Mask8Bits)<<ShiftBy8Pos)));
        break;
                          
    case HumidityMessage:
        BSP_Ser_Printf("\nSOURCE NODE %d: HUMIDITY MESSAGE \n Dew Point = %d Humidity = %d \n",payloadBuffer.srcAddr,payloadBuffer.dataPart.hum.dewPt,payloadBuffer.dataPart.hum.hum);      
        break;
                          
    case WindMessage:
        BSP_Ser_Printf("\nSOURCE NODE %d: WIND MESSAGE \n Wind Speed = %d%d%d.%d Direction = %d \n",payloadBuffer.srcAddr,
                           ((payloadBuffer.dataPart.wind.speed[Array0]>>ShiftBy4Pos)&Mask4Bits),(payloadBuffer.dataPart.wind.speed[Array0] & Mask4Bits),
                           ((payloadBuffer.dataPart.wind.speed[Array1]>>ShiftBy4Pos)&Mask4Bits),(payloadBuffer.dataPart.wind.speed[Array1] & Mask4Bits),
                           (((payloadBuffer.dataPart.wind.dir>>ShiftBy8Pos)& Mask8Bits)|((payloadBuffer.dataPart.wind.dir & Mask8Bits)<<ShiftBy8Pos)));
        break;
        
    case SolarRadiationMessage:
        BSP_Ser_Printf("\nSOURCE NODE %d: SOLAR RADIATION MESSAGE \n Solar Radiation Intensity = %d \n",payloadBuffer.srcAddr,
                          (((payloadBuffer.dataPart.rad>>ShiftBy8Pos)& Mask8Bits)|((payloadBuffer.dataPart.rad & Mask8Bits)<<ShiftBy8Pos)));
        break;
      
    case DateTimeMessage:
        Msb = ((payloadBuffer.dataPart.dateTime << ShiftBy24Pos) & DateMask1);
        Smsb = ((payloadBuffer.dataPart.dateTime << ShiftBy8Pos) & DateMask2);
        Tmsb = ((payloadBuffer.dataPart.dateTime >> ShiftBy8Pos) & DateMask3);
        Lsb = ((payloadBuffer.dataPart.dateTime >> ShiftBy24Pos) & DateMask4);
        
        payloadBuffer.dataPart.dateTime = (Msb | Smsb | Tmsb | Lsb);
        
        BSP_Ser_Printf("\nSOURCE NODE %d: DATE/TIME STAMP MESSAGE \n Time Stamp = %d/%d/%d %d:%d \n",payloadBuffer.srcAddr,
                            ((payloadBuffer.dataPart.dateTime >> ShiftBy5Pos) & MonthMask),
                            (payloadBuffer.dataPart.dateTime & DayMask),
                            ((payloadBuffer.dataPart.dateTime >> ShiftBy9Pos) & YearMask),
                            ((payloadBuffer.dataPart.dateTime >> ShiftBy27Pos) & HourMask),
                            ((payloadBuffer.dataPart.dateTime >> ShiftBy21Pos) & MinuteMask));
        break;        
      
    case PrecipitationMessage:
       BSP_Ser_Printf("\nSOURCE NODE %d: PRECIPITATION MESSAGE \n Precipitation Depth = %d/%d./%d%d \n",payloadBuffer.srcAddr,
                      ((payloadBuffer.dataPart.depth[Array0] >> ShiftBy4Pos) & Mask4Bits),
                      (payloadBuffer.dataPart.depth[Array0] & Mask4Bits),
                      ((payloadBuffer.dataPart.depth[Array1] >> ShiftBy4Pos) & Mask4Bits),
                      (payloadBuffer.dataPart.depth[Array1] & Mask4Bits));
       break;
       
  default :
    Errortp(UnknownMsg);
   break; 
   
  } 
}

void AppMain(void)
{
  for(;;)
  {
   parsePkt(&payloadBuffer); 
  if(payloadBuffer.dstAddr == DestinationAdrs ) 
  {
    PrintMsg();
  }
  else 
  {
     Errortp(DestinationAddr);
  }
 }
}
                           
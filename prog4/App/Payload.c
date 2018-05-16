/*--------------------------------Payload.c----------------------------------*/
/*Name: Lalit Bhat
Student ID:01671833
Program Number: 4
*/

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "Payload.h"
#include "BfrPair.h"

#define Array0 0 //MSB
#define Array1 1 //LSB
#define BaudRate 9600
#define WindSpeedArray 2
#define DepthArray 2
#define IdArray 10
#define PrintBfrSize 81

#define PayloadTaskPrio 6
#define PAYLOAD_STK_SIZE 256

#define ShiftBy8Pos 8        //shift 8 bits
#define Mask8Bits 0xff       //masking
#define ShiftBy4Pos 4        //Shift 4 bits
#define Mask4Bits 0x0f       //masking
#define ShiftBy24Pos 24      //Shift 24 bits
#define DateMask1 0xff000000 //masking logic in date/time node
#define DateMask2 0x00ff0000 //masking logic in date/time node
#define DateMask3 0x0000ff00 //masking logic in date/time node
#define DateMask4 0x000000ff //masking logic in date/time node
#define ShiftBy5Pos 5        //shift 5 bits in date/time node
#define ShiftBy9Pos 9        //shift 9 bits in date/time node
#define ShiftBy21Pos 21      //shift 21 bits in date/time node
#define ShiftBy27Pos 27      //shift 27 bits
#define MonthMask 0x0000000f //masking logic in date/time node
#define DayMask 0x0000001f   //masking logic in date/time node
#define YearMask 0x00000fff  //masking logic in date/time node
#define HourMask 0x0000001f  //masking logic in date/time node

#define MinuteMask 0x0000003f//masking logic in date/time node
#define InitVal 0            //Starting counter of for loop in ID NODE
#define MaxVal 6             //max counter value of for loop in ID NODE
#define PayloadBfrSize 14
#define SuspendTimeout 0




typedef struct
{
  CPU_INT08S payloadLen;
  CPU_INT08U dstAddr;
  CPU_INT08U srcAddr;
  CPU_INT08U msgType;
  union
  {
    CPU_INT08S temp;
    CPU_INT16U pres;
    struct
    {
      CPU_INT08S dewPt;
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
  }dataPart;
}payload;

BfrPair payloadBfrPair;
CPU_INT08U PBfr0Space[PayloadBfrSize]; 
CPU_INT08U PBfr1Space[PayloadBfrSize];

OS_SEM openPayloadBfrs;
OS_SEM closedPayloadBfrs;

static OS_TCB PayloadTCB;
static CPU_STK PayloadStk[PAYLOAD_STK_SIZE];

void PayloadInit(BfrPair *pBfrPair)
{
  BfrPairInit(pBfrPair,PBfr0Space,PBfr1Space,PayloadBfrSize);
}

void CreatePayloadTask(void)
{
  OS_ERR osErr; // O/S error code
  
  /*Create the payload task*/
  OSTaskCreate(&PayloadTCB,
               "PayloadTask",
               PayloadTask,
               NULL,
               PayloadTaskPrio,
               &PayloadStk[0],
               PAYLOAD_STK_SIZE / 10,
               PAYLOAD_STK_SIZE,
               0,
               0,
               0,
               (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
               &osErr);
  assert(osErr == OS_ERR_NONE);
  
  OSSemCreate(&openPayloadBfrs,"Open payloadBfrs",2,&osErr);
  assert(osErr==OS_ERR_NONE);

  OSSemCreate(&closedPayloadBfrs,"Closed payloadBfrs",0,&osErr);
  assert(osErr==OS_ERR_NONE);
  
  PayloadInit(&payloadBfrPair);
}
BfrPair payloadBfrPair;
void PayloadTask(void *data)
{
  enum {TemperatureMessage=1,BarometricPressureMessage,HumidityMessage,WindMessage,SolarRadiationMessage,DateTimeMessage,PrecipitationMessage,IdMessage};
  payload *payloadBuffer;
  
  CPU_INT32U Msb; //MSB of Date/Time 
 CPU_INT32U Smsb; //SSB of Date/Time
 CPU_INT32U Tmsb; //TSB of Date/Time
 CPU_INT32U Lsb; //LSB of Date/Time
 CPU_INT08U idCounter; //for sensor message type for loop
 
 for(;;)
 {
   
   if(!GetBfrClosed(&payloadBfrPair))
   {
     OS_ERR osErr;
      OSSemPend(&closedPayloadBfrs,SuspendTimeout,OS_OPT_PEND_BLOCKING,NULL,&osErr);
      assert(osErr==OS_ERR_NONE);
      BfrPairSwap(&payloadBfrPair);
   }
   
  payloadBuffer= (payload *) GetBfrAddr(&payloadBfrPair);
  if(GetBfrClosed(&payloadBfrPair))
  {
    if(payloadBuffer->payloadLen <= 0)
    {
        Errortp(payloadBuffer->payloadLen);
    }
    else
    {
      if(payloadBuffer->dstAddr==1)
      {
    
        switch(payloadBuffer->msgType)
        
        {
        case TemperatureMessage:
          SerPrintf("\nSOURCE NODE %d: TemperatureMessage \n \tTemperature = %d\n",payloadBuffer->srcAddr,payloadBuffer->dataPart.temp);
            break;
        
        case BarometricPressureMessage:
          SerPrintf("\nSOURCE NODE %d: BAROMETRIC PRESSURE MESSAGE\n \tPressure = %d\n",payloadBuffer->srcAddr,(((payloadBuffer->dataPart.pres>>ShiftBy8Pos)& Mask8Bits)|((payloadBuffer->dataPart.pres & Mask8Bits)<<ShiftBy8Pos)));
            break;
        
        case HumidityMessage:
          SerPrintf("\nSOURCE NODE %d: HUMIDITY MESSAGE\n \tDew Point = %d Humidity = %d\n",payloadBuffer->srcAddr,payloadBuffer->dataPart.hum.dewPt,payloadBuffer->dataPart.hum.hum);
            break;
        
        case WindMessage:
          SerPrintf("\nSOURCE NODE %d: WIND MESSAGE\n \tSpeed = %d%d%d.%d Wind Direction = %d\n", payloadBuffer->srcAddr,
                              ((payloadBuffer->dataPart.wind.speed[Array0]>>ShiftBy4Pos)&Mask4Bits),(payloadBuffer->dataPart.wind.speed[Array0] & Mask4Bits),
                              ((payloadBuffer->dataPart.wind.speed[Array1]>>ShiftBy4Pos)&Mask4Bits),(payloadBuffer->dataPart.wind.speed[Array1] & Mask4Bits),
                              (((payloadBuffer->dataPart.wind.dir>>ShiftBy8Pos)& Mask8Bits)|((payloadBuffer->dataPart.wind.dir & Mask8Bits)<<ShiftBy8Pos)));
            break;
        
        case SolarRadiationMessage:
          SerPrintf("\nSOURCE NODE %d: SOLAR RADIATION MESSAGE\n \tSolar Radiation Intensity = %d\n",payloadBuffer->srcAddr,
                              (((payloadBuffer->dataPart.rad>>ShiftBy8Pos)& Mask8Bits)|((payloadBuffer->dataPart.rad & Mask8Bits)<<ShiftBy8Pos)));
            break;
        
        case DateTimeMessage:
      
            Msb = ((payloadBuffer->dataPart.dateTime << ShiftBy24Pos) & DateMask1);
            Smsb = ((payloadBuffer->dataPart.dateTime << ShiftBy8Pos) & DateMask2);
            Tmsb = ((payloadBuffer->dataPart.dateTime >> ShiftBy8Pos) & DateMask3);
            Lsb = ((payloadBuffer->dataPart.dateTime >> ShiftBy24Pos) & DateMask4);
        
            payloadBuffer->dataPart.dateTime = (Msb | Smsb | Tmsb | Lsb);
        
            SerPrintf("\nSOURCE NODE %d: DATE/TIME STAMP MESSAGE\n \tTime Stamp = %lu/%lu/%lu %lu:%lu\n", payloadBuffer->srcAddr,
                                ((payloadBuffer->dataPart.dateTime >> ShiftBy5Pos) & MonthMask),
                                (payloadBuffer->dataPart.dateTime & DayMask),
                                ((payloadBuffer->dataPart.dateTime >> ShiftBy9Pos) & YearMask),
                                ((payloadBuffer->dataPart.dateTime >> ShiftBy27Pos) & HourMask),
                                ((payloadBuffer->dataPart.dateTime >> ShiftBy21Pos) & MinuteMask));
            break;
        
        case PrecipitationMessage:
          SerPrintf("\nSOURCE NODE %d: PRECIPITATION MESSAGE\n \tPrecipitation Depth = %d%d.%d%d\n",payloadBuffer->srcAddr,
                          ((payloadBuffer->dataPart.depth[Array0] >> ShiftBy4Pos) & Mask4Bits),
                          (payloadBuffer->dataPart.depth[Array0] & Mask4Bits),
                          ((payloadBuffer->dataPart.depth[Array1] >> ShiftBy4Pos) & Mask4Bits),
                          (payloadBuffer->dataPart.depth[Array1] & Mask4Bits));
            break;
        
        case IdMessage:
           SerPrintf("\nSOURCE NODE %d: ID MESSAGE\n\t NODE ID = ",payloadBuffer->srcAddr);
            for(idCounter=InitVal ; idCounter<MaxVal ; idCounter++)
            {
              SerPrintf("%c",payloadBuffer->dataPart.id[idCounter]);
            }
            break;
        
        default:
          SerPrintf("\n*** ERROR: Message unknown \a");
          break;
        }
      }
      else
      {
        SerPrintf("\a***Not the correct address\n");
      }
    }
 
        OS_ERR osErr;
        OpenGetBfr(&payloadBfrPair);
        OSSemPost(&openPayloadBfrs,OS_OPT_POST_1,&osErr);
        assert(osErr==OS_ERR_NONE);
    
    }
 }
}
      
                 
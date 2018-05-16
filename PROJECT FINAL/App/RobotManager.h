/*--------------------------------RobotManager.h----------------------------------*/
/*Name: Lalit Bhat
Student ID:01671833
FINAL PROJECT
*/
#ifndef RobotManager_H
#define RobotManager_H

#include "Queue.h"
#include "BfrPair.h"
#include "includes.h"

#define XYCoordinateArray 10
#define XLabelArray 39
#define YLabelArray 18
#define StopArray 13

/*---Structure of the Robot Manager Payload---*/
typedef struct
{
  CPU_INT08U payloadLength;
  CPU_INT08U destinationAddress;
  CPU_INT08U sourceAddress;
  CPU_INT08U messageType;
  union
  {
    struct
    {
      CPU_INT08U robotAddress;
      struct
      {
        CPU_INT08U x;
        CPU_INT08U y;
      }coordinates[XYCoordinateArray];
    }botAddrPos;
  }botInfo;
}payload;

/*--------globals----------*/

extern CPU_INT08U factoryFloor[XLabelArray][YLabelArray];
extern CPU_BOOLEAN stopBot[StopArray];

/*-------Function Prototypes--------*/

void CreatePayloadTask(void);
void PayloadTask(void *data);
void AcknowledgeCommand(CPU_INT08U Type);

#endif
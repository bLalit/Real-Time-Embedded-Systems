/*--------------------------------Framer.h----------------------------------*/
/*Name: Lalit Bhat
Student ID:01671833
FINAL PROJECT
*/

#ifndef FRAMER_H
#define FRAMER_H

/*---------------Header files---------------*/
#include "Globals.h"
#include "includes.h"
#include "MemMgr.h"
#include "SerIODriver.h"
#include "assert.h"
#include "Queue.h"
#include "PktParser.h"

/*-----------Constant Definations----------*/
//#define SuspendTimeout 0
#define FramerPrio 2
#define	FRAMER_STK_SIZE     256 

/*-------------Struct-----------------*/
typedef struct
{
  CPU_INT08U Error;
  CPU_INT08U messageType;
  CPU_INT08U PresentBot;
}outPut;

//globals
extern OS_Q FramerQueue;


/*----------Function Prototypes------------*/

CPU_VOID CreateFramerTask();
CPU_VOID FramerTask(CPU_VOID *data);


#endif
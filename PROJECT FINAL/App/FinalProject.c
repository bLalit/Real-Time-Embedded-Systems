//Edited by Lalit Bhat
/*=============== P r o g 4 . c ===============*/

/*
BY:	George Cheney
		EECE472 / EECE572 Embedded Real Time Systems
		Electrical and Computer Engineering Dept.
		UMASS Lowell
*/

/*
PURPOSE
Read wireless sensor node packets from the Rx, interpret the payloads,
and send back appropriate reply messages.

DEMONSTRATES
Message Queues
Memory Management
Multitasking
Semaphores
Buffer Pool

*/


/*
NOTE: This example is derived from example code provided by Micrium whose copyright notice is below.

*********************************************************************************************************
*                                              EXAMPLE CODE
*
*                             (c) Copyright 2009; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*               Knowledge of the source code may NOT be used to develop a similar product.
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                            EXAMPLE CODE
*
*                                     ST Microelectronics STM32
*                                              on the
*
*                                     Micrium uC-Eval-STM32F107
*                                        Evaluation Board
*
* Filename      : app.c
* Version       : V1.00
* Programmer(s) : JJL
                  EHS
                  gpc
*********************************************************************************************************
*/

#include "includes.h"
#include "assert.h"
#include "Queue.h"
#include "MemMgr.h"
#include "Buffer.h"
#include "Framer.h"
#include "SerIODriver.h"
#include "PktParser.h"
#include "RobotManager.h"

/*------ c o n s t a n t    d e f i n i t i o n s -----*/

#define Init_STK_SIZE 128      // Init task stack size
#define Init_PRIO 1            // Init task Priority

// USART2 Settings
#define BaudRate 9600          // Baud Rate Setting

/*----- G l o b a l    V a r i a b l e s -----*/

static  OS_TCB   initTCB;                         // Init task TCB
static  CPU_STK  initStk[Init_STK_SIZE];          // Space for Init task stack

/*----- f u n c t i o n    p r o t o t y p e s -----*/

static  void  Init  (void *data);

//--------------- m a i n ( ) ---------------

CPU_INT32S  main (CPU_VOID)
{
    OS_ERR  err;                          // OS Error Code


    BSP_IntDisAll();                      // Disable all interrupts.

    OSInit(&err);                         // Init uC/OS-III.
    assert(err == OS_ERR_NONE);

    OSTaskCreate(&initTCB,                // Create the init task.
                 "Init Task",
                 Init, 
                 NULL,
                 Init_PRIO,
                 &initStk[0],
                 Init_STK_SIZE / 10,
                 Init_STK_SIZE,
                 0,
                 0,
                 0,
                 (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 &err);
    assert(err == OS_ERR_NONE);

    OSStart(&err);                        // Start multitasking.
    assert(err == OS_ERR_NONE);
}

/*--------------- I n i t ( ) ---------------*/

/*
PURPOSE
Perform O/S and application initialization and delete self.
Create application tasks.

INPUT PARAMETERS
data		-- pointer to task data (not used)
*/

static  CPU_VOID  Init (CPU_VOID *data)
{
  CPU_INT32U  cpu_clk_freq; // CPU Clock Frequency in Hz.
  CPU_INT32U  cnts;         // SysTick initial count value
  OS_ERR      err;          // uCOS error code
  
  BSP_Init();                                                   /* Initialize BSP functions                         */
  CPU_Init();                                                   /* Initialize the uC/CPU services                   */
  
  cpu_clk_freq = BSP_CPU_ClkFreq();                             /* Determine SysTick reference freq.                */                                                                        
  cnts         = cpu_clk_freq / (CPU_INT32U)OSCfg_TickRate_Hz;  /* Determine nbr SysTick increments                 */
  OS_CPU_SysTickInit(cnts);                                     /* Init uC/OS periodic time src (SysTick).          */

#if OS_CFG_STAT_TASK_EN > 0u
  OSStatTaskCPUUsageInit(&err);                                 /* Compute CPU usage with no task running        */
#endif

  // Resets current maximum interrupts disabled time.
  CPU_IntDisMeasMaxCurReset();
  
  // Initialize the USART2 I/O Driver.
  
  InitMemMgr();
  CreatePayloadTask();
  CreateParserTask();
  CreateFramerTask();

  // Initialize USART2 and USART2 interupts.
  BSP_IntVectSet(BSP_INT_ID_USART2, ISR); // Set USART2 interrupt vector.
  BSP_Ser_Init(BaudRate);                 // Set Baud rate.
  BSP_IntEn(BSP_INT_ID_USART2);           // Enable IRQ38.
  InitSerIO();
  
  	/* This task is done, so it deletes itself. */
  OSTaskDel(&initTCB, &err);
  assert(err == OS_ERR_NONE);
}

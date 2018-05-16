/*--------------------------------RobotManager.c----------------------------------*/
/*Name: Lalit Bhat
Student ID:01671833
FINAL PROJECT
*/

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "RobotManager.h"
#include "includes.h"
#include "Queue.h"
#include "assert.h"
#include "Globals.h"
#include "PktParser.h"
#include "Framer.h"
#include "MemMgr.h"
#include "SerIODriver.h"
#include "RobotControl.h"

//Constant Definitions

#define MaxBots 16
#define BaudRate 9600
#define SuspendTimeout 0
#define PayloadPrio 2
#define PAYLOAD_STK_SIZE 600
#define AddBotBadBotAddr 11
#define AddBotBadLocation 12
#define AddBotLocationOccupied 13
#define AddBotRobotAlreadyExists 14
#define MoveBotBadRobotAddress 21
#define MoveBotNonexistentRobot 22
#define MobeBotBadLocation 23
#define FollowPathBadBotAddr 31
#define FollowPathNonexistentRobot 32
#define FollowPathBadLocation 33
#define LoopBadBotAddress 41
#define LoopNonExistentRobot 42
#define LoopBadLocation 43
#define StopBadRobotAddress 51
#define StopNonexistentRobot 52
#define BotMgrTaskBasMessageType 61
#define PAYLOAD_STK_SIZE 600

CPU_INT08U factoryFloor[39][18];
CPU_BOOLEAN stopBot[13];
CPU_INT08U Robot[16];
CPU_INT08U ctr=0; //check this

static OS_TCB PayloadTCB;
static CPU_STK PayloadStk[PAYLOAD_STK_SIZE];
OS_Q BotQueue[MaxBots]; 

void AddRobot(CPU_INT08U Bots);
void MoveFollowLoop(Que *bfr, CPU_INT08U CurrentBot);
void AcknowledgeCommand(CPU_INT08U Type);
void StopLooping(CPU_INT08U Robot);
CPU_BOOLEAN CheckBot(CPU_INT08U x,CPU_INT08U y);
void HereIAm(CPU_INT08U Robot, CPU_INT08U x, CPU_INT08U y);

BotLocation bot;

void CreatePayloadTask(void) //Creating payloadtask for robot manager
{
  OS_ERR osErr;
  OSMutexCreate (&Mutex,"Floor Control",&osErr);
  assert(osErr==OS_ERR_NONE);
  OSTaskCreate(&PayloadTCB,
               "Robot Manager",
               PayloadTask,
               NULL,
               PayloadPrio,
               &PayloadStk[0],
               PAYLOAD_STK_SIZE/10,
               PAYLOAD_STK_SIZE,
               0,
               0,
               NULL,
               (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
               &osErr);
  assert(osErr==OS_ERR_NONE);
}

void PayloadTask(void *data)
{ 
  OS_ERR osErr;
  
  Que *iBfr = NULL;
  OS_MSG_SIZE msgSize;
  
  for(;;)
  {
    if (iBfr == NULL) //getting a filled buffer to empty
    {
      iBfr =  OSQPend(&ParserQueue,
                      0,
                      OS_OPT_PEND_BLOCKING,
                      &msgSize,
                      NULL,
                      &osErr);
      assert(osErr==OS_ERR_NONE);
    }
    
    payload *payloadBfr = (payload *)iBfr->bfr;

    switch(payloadBfr->messageType)
    {
    case 0x00:
      NVIC_GenerateCoreReset();
      break;
      
    case 0x01:
      bot.BotAddr=payloadBfr->botInfo.botAddrPos.robotAddress;
      bot.xPosition=payloadBfr->botInfo.botAddrPos.coordinates->x;
      bot.yPosition=payloadBfr->botInfo.botAddrPos.coordinates->y;
      if (/*bot.BotAddr>=3 &&*/bot.BotAddr<=16)
      {
        if (bot.xPosition<=39 && bot.yPosition<=18)
        {
          if (factoryFloor[bot.xPosition][bot.yPosition]==0)
          {
            if(Robot[bot.BotAddr] != 1)
            {
              
              CreateRobot(bot);
              Robot[bot.BotAddr] =1;
              factoryFloor[bot.xPosition][bot.yPosition]=1;
              AcknowledgeCommand(payloadBfr->messageType);
            }
            else
            {
             //robot already exists;
              Que *iBfr = NULL;
              if(iBfr == NULL)
              {
                iBfr = Allocate();
              }
              AddByte(iBfr, AddBotRobotAlreadyExists);
              AddByte(iBfr,0x0B); 
              OSQPost(&FramerQueue, iBfr, sizeof(Que), OS_OPT_POST_FIFO, &osErr);
              assert(osErr == OS_ERR_NONE);
              iBfr=NULL;
            }
          }
          else
          {
            //occupied location;
            Que *iBfr = NULL;
            if(iBfr == NULL)
            {
              iBfr = Allocate();
            }
            AddByte(iBfr, AddBotLocationOccupied);
            AddByte(iBfr,0x0B);
            OSQPost(&FramerQueue, iBfr, sizeof(Que), OS_OPT_POST_FIFO, &osErr);
            assert(osErr == OS_ERR_NONE);
            iBfr=NULL; 
          }
        }
        else
        {
           //bad robot location;
          Que *iBfr = NULL;
           if(iBfr == NULL)
           {
              iBfr = Allocate();
           }
          AddByte(iBfr, AddBotBadLocation);
          AddByte(iBfr,0x0B);
          OSQPost(&FramerQueue, iBfr, sizeof(Que), OS_OPT_POST_FIFO, &osErr);
          assert(osErr == OS_ERR_NONE);
          iBfr=NULL;          
        }
      }
      else
      {
        //Bad robot address
        Que *iBfr = NULL;
        if(iBfr == NULL)
        {
          iBfr = Allocate();
        }
        AddByte(iBfr, AddBotBadBotAddr);
        AddByte(iBfr,0x0B);
        OSQPost(&FramerQueue, iBfr, sizeof(Que), OS_OPT_POST_FIFO, &osErr);
        assert(osErr == OS_ERR_NONE);
        iBfr=NULL;
      }
      Free(iBfr);
      break;
      
    case 0x02:
      if(payloadBfr->botInfo.botAddrPos.robotAddress<=16)
      {
        if(Robot[payloadBfr->botInfo.botAddrPos.robotAddress])
        {
          if(payloadBfr->botInfo.botAddrPos.coordinates->x<=39 &&payloadBfr->botInfo.botAddrPos.coordinates->y<=18)
          {
            MoveFollowLoop(iBfr,payloadBfr->botInfo.botAddrPos.robotAddress);
            AcknowledgeCommand(payloadBfr->messageType);
          }
          else
          {
             //move: Bad Location
            Que *iBfr = NULL;
            if(iBfr == NULL)
            {
              iBfr = Allocate();
            }
            AddByte(iBfr, MobeBotBadLocation);
            AddByte(iBfr,0x0B);
            OSQPost(&FramerQueue, iBfr, sizeof(Que), OS_OPT_POST_FIFO, &osErr);
            assert(osErr == OS_ERR_NONE);
            iBfr=NULL;
          }
        }
        else
        {
          //move: Non-Existant Robot
          Que *iBfr = NULL;
          if(iBfr == NULL)
          {
            iBfr = Allocate();
          }
          AddByte(iBfr, MoveBotNonexistentRobot);
          AddByte(iBfr,0x0B);
          OSQPost(&FramerQueue, iBfr, sizeof(Que), OS_OPT_POST_FIFO, &osErr);
          assert(osErr == OS_ERR_NONE);
          iBfr=NULL;
        }
      }
      else
      {
         //move: Bad Robot Address
        Que *iBfr = NULL;
        if(iBfr == NULL)
        {
          iBfr = Allocate();
        }
         AddByte(iBfr, MoveBotBadRobotAddress);
         AddByte(iBfr,0x0B);
         OSQPost(&FramerQueue, iBfr, sizeof(Que), OS_OPT_POST_FIFO, &osErr);
         assert(osErr == OS_ERR_NONE);
         iBfr=NULL;   
      }
      break;
      
    case 0x03:
      if(payloadBfr->botInfo.botAddrPos.robotAddress<=16)
      {
        if(Robot[payloadBfr->botInfo.botAddrPos.robotAddress])
        {
          if(payloadBfr->botInfo.botAddrPos.coordinates->x<=39 && payloadBfr->botInfo.botAddrPos.coordinates->y<=18)
          {
            MoveFollowLoop(iBfr,payloadBfr->botInfo.botAddrPos.robotAddress);
            AcknowledgeCommand(payloadBfr->messageType);
          }
          else
          {
             //Follow Path: Bad Location
            Que *iBfr = NULL;
            if(iBfr == NULL)
            {
              iBfr = Allocate();
            }
            AddByte(iBfr, FollowPathBadLocation);
            AddByte(iBfr,0x0B);
            OSQPost(&FramerQueue, iBfr, sizeof(Que), OS_OPT_POST_FIFO, &osErr);
            assert(osErr == OS_ERR_NONE);
            iBfr=NULL;
          }
        }
        else
        {
          //Follow Path: Non-Existant Robot
          Que *iBfr = NULL;
          if(iBfr == NULL)
          {
            iBfr = Allocate();
          }
          AddByte(iBfr, FollowPathNonexistentRobot);
          AddByte(iBfr,0x0B);
          OSQPost(&FramerQueue, iBfr, sizeof(Que), OS_OPT_POST_FIFO, &osErr);
          assert(osErr == OS_ERR_NONE);
          iBfr=NULL;
        }
      }
      else
      {
         //Follow Path: Bad Robot Address
        Que *iBfr = NULL;
        if(iBfr == NULL)
        {
          iBfr = Allocate();
        }
         AddByte(iBfr, FollowPathBadBotAddr);
         AddByte(iBfr,0x0B);
         OSQPost(&FramerQueue, iBfr, sizeof(Que), OS_OPT_POST_FIFO, &osErr);
         assert(osErr == OS_ERR_NONE);
         iBfr=NULL;   
      }
      break;
      
    case 0x04:
      if(payloadBfr->botInfo.botAddrPos.robotAddress<=16)
      {
        if(Robot[payloadBfr->botInfo.botAddrPos.robotAddress])
        {
          if(payloadBfr->botInfo.botAddrPos.coordinates->x<=39 && payloadBfr->botInfo.botAddrPos.coordinates->y<=18)
          {
            MoveFollowLoop(iBfr,payloadBfr->botInfo.botAddrPos.robotAddress);
            AcknowledgeCommand(payloadBfr->messageType);
          }
          else
          {
            //Loop: Bad Location
            Que *iBfr = NULL;
            if(iBfr == NULL)
            {
              iBfr = Allocate();
            }
            AddByte(iBfr, LoopBadLocation);
            AddByte(iBfr,0x0B);
            OSQPost(&FramerQueue, iBfr, sizeof(Que), OS_OPT_POST_FIFO, &osErr);
            assert(osErr == OS_ERR_NONE);
            iBfr=NULL;
          }
        }
        else
        {
          //Loop: Non-Existant Robot
          Que *iBfr = NULL;
          if(iBfr == NULL)
          {
            iBfr = Allocate();
          }
          AddByte(iBfr, LoopNonExistentRobot);
          AddByte(iBfr,0x0B);
          OSQPost(&FramerQueue, iBfr, sizeof(Que), OS_OPT_POST_FIFO, &osErr);
          assert(osErr == OS_ERR_NONE);
          iBfr=NULL;
        }
      }
      else
      {
         //Loop: Bad Robot Address
         AddByte(iBfr, LoopBadBotAddress);
         AddByte(iBfr,0x0B);
         OSQPost(&FramerQueue, iBfr, sizeof(Que), OS_OPT_POST_FIFO, &osErr);
         assert(osErr == OS_ERR_NONE);
         iBfr=NULL;   
      }
      break;
      
    case 0x09:
      payloadBfr->botInfo.botAddrPos.coordinates->y = payloadBfr->botInfo.botAddrPos.coordinates->x;
      payloadBfr->botInfo.botAddrPos.coordinates->x = payloadBfr->botInfo.botAddrPos.robotAddress;
      HereIAm(payloadBfr->sourceAddress, payloadBfr->botInfo.botAddrPos.coordinates->x, payloadBfr->botInfo.botAddrPos.coordinates->y);
      Free(iBfr);
      break;
      
    case 0x05:
      AcknowledgeCommand(0x05);
      if(payloadBfr->botInfo.botAddrPos.robotAddress<=16)
      {
        if(Robot[payloadBfr->botInfo.botAddrPos.robotAddress])
        {
          StopLooping(payloadBfr->botInfo.botAddrPos.robotAddress);
        }
        else
        {
         //Stop: Non-existent Robot
         Que *iBfr = NULL;
         if(iBfr == NULL)
         {
           iBfr = Allocate();
         }
         AddByte(iBfr, StopNonexistentRobot);
         AddByte(iBfr,0x0B);
         OSQPost(&FramerQueue, iBfr, sizeof(Que), OS_OPT_POST_FIFO, &osErr);
         assert(osErr == OS_ERR_NONE);
         iBfr=NULL;           
        }
      }
      else
      {
         //Stop: Bad Robot Address;
        Que *iBfr = NULL;
        if(iBfr == NULL)
        {
          iBfr = Allocate();
        }
         AddByte(iBfr, StopBadRobotAddress);
         AddByte(iBfr,0x0B);
         OSQPost(&FramerQueue, iBfr, sizeof(Que), OS_OPT_POST_FIFO, &osErr);
         assert(osErr == OS_ERR_NONE);
         iBfr=NULL;         
      }
      break;
    }
    iBfr=NULL;
  }
}

void StopLooping(CPU_INT08U Robot)
{
  Que *oBfr = NULL;
  if (oBfr == NULL)
    oBfr = Allocate();
  AddByte(oBfr,0x05);
  AddByte(oBfr, Robot);
  OS_ERR osErr;
  OSQPost(&MailQueue[Robot],oBfr, sizeof(Que), OS_OPT_POST_FIFO, &osErr);
  assert(osErr==OS_ERR_NONE); 
}

void MoveFollowLoop(Que *bfr, CPU_INT08U CurrentBot)
{
  OS_ERR osErr;
  OSQPost(&BotQueue[CurrentBot],bfr,sizeof(Que),OS_OPT_POST_FIFO, &osErr);
  assert(osErr==OS_ERR_NONE);
}

void HereIAm(CPU_INT08U Robot, CPU_INT08U x, CPU_INT08U y)
{
  Que *oBfr = NULL;
  if (oBfr == NULL)
  {
    oBfr = Allocate();
  }
  AddByte(oBfr,0x09);
  AddByte(oBfr,Robot);
  AddByte(oBfr,x);
  AddByte(oBfr,y);
  OS_ERR osErr;
  OSQPost(&MailQueue[Robot],oBfr,sizeof(Que),OS_OPT_POST_FIFO, &osErr);
  assert(osErr==OS_ERR_NONE);
}

void AcknowledgeCommand(CPU_INT08U Type)
{
  Que *oBfr = NULL;
  if (oBfr == NULL)
  {
    oBfr = Allocate();
  }
  AddByte(oBfr,Type);
  AddByte(oBfr,0x0A);
  OS_ERR osErr;
  OSQPost(&FramerQueue,oBfr, sizeof(Que), OS_OPT_POST_FIFO, &osErr);
  assert(osErr==OS_ERR_NONE);
}
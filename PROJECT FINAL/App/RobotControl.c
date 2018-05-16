/*--------------------------------RobotControl.c----------------------------------*/
/*Name: Lalit Bhat
Student ID:01671833
FINAL PROJECT
*/

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "includes.h"//
#include "Globals.h"//
#include "PktParser.h"
#include "Framer.h"//
#include "RobotControl.h"//
#include "RobotManager.h"//
#include "assert.h"//
#include "Queue.h"//
#include "MemMgr.h"//

#define ROBOT_STK_SIZE 256
#define RobotPrio 1
#define N 1
#define NE 2
#define E 3
#define SE 4
#define S 5
#define SW 6
#define W 7
#define NW 8
#define NoStep 0

typedef struct
{
  CPU_INT08U Addr;
  CPU_INT08U RobAdrs;
  CPU_INT08U sx;
  CPU_INT08U sy;
}Here;


OS_MUTEX Mutex;//
OS_Q MailQueue[Maximum];//                
//OS_Q BotQueue[Maximum];
OS_SEM Semaphore[Maximum];//

//function prototypes
static CPU_STK RobotStk[Maximum][ROBOT_STK_SIZE];//
static OS_TCB RobotTCB[Maximum];//
void BotTask(void *data);//
void RobotStep(CPU_INT08U Robot,CPU_INT08U X,CPU_INT08U Y);//
void HereIAM(CPU_INT08U CurrentRobot);//
CPU_INT08U StepLogic(CPU_INT08U Presentx,CPU_INT08U Presenty,CPU_INT08S difx,CPU_INT08S dify);//


CPU_INT08U Direction;

OS_ERR osErr;

BotLocation BotLoc[Maximum];
void CreateRobot(BotLocation Robots)
{
  OS_ERR osErr;
  
  OSQCreate(&BotQueue[Robots.BotAddr], "Robot Queue",PoolSize, &osErr);
  assert(osErr == OS_ERR_NONE);
  
  OSQCreate(&MailQueue[Robots.BotAddr],"MailBox Queue",1,&osErr);
  assert(osErr == OS_ERR_NONE);
  
  OSTaskCreate(&RobotTCB[Robots.BotAddr],                             
               "BotTask",         
               BotTask,                 
               &Robots,                    
               RobotPrio,            
               &RobotStk[Robots.BotAddr][256],         
               ROBOT_STK_SIZE/ 10,  
               ROBOT_STK_SIZE,       
               0,                       
               0,                       
               0,                    
               (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
               &osErr);

  assert(osErr == OS_ERR_NONE);
  BotLoc[Robots.BotAddr].xPosition = Robots.xPosition;
  BotLoc[Robots.BotAddr].yPosition = Robots.yPosition;
}

void BotTask(void *data)
{
  BotLocation Robot = *(BotLocation *)data;
  OS_MSG_SIZE msgSize;
  OS_ERR osErr;
  Que *iBfr = NULL;
  CPU_INT08U z;
  CPU_INT08U q;
  
  for(;;)
  {
    if(iBfr==NULL)
    {
      iBfr = OSQPend(&BotQueue[Robot.BotAddr],0,OS_OPT_PEND_BLOCKING,&msgSize,NULL,&osErr);
      assert(osErr==OS_ERR_NONE);
    }
    payload *Robots = (payload *)iBfr->bfr;
    switch(Robots->messageType)
    {
    case 0x02:
      BotLoc[Robots->botInfo.botAddrPos.robotAddress].xLabelNext = Robots->botInfo.botAddrPos.coordinates[0].x;
      BotLoc[Robots->botInfo.botAddrPos.robotAddress].yLabelNext = Robots->botInfo.botAddrPos.coordinates[0].y;
      RobotStep(Robots->botInfo.botAddrPos.robotAddress,Robots->botInfo.botAddrPos.coordinates[0].x,Robots->botInfo.botAddrPos.coordinates[0].y);
      //RobotStep(Robots->botInfo.botAddrPos.robotAddress,BotLoc[Robots->botInfo.botAddrPos.robotAddress].xLabelNext,BotLoc[Robots->botInfo.botAddrPos.robotAddress].yLabelNext);
      break;
      
    case 0x03:
      for (z=0;z<=(Robots->payloadLength-5)/2;z++)
      {
        RobotStep(Robots->botInfo.botAddrPos.robotAddress,Robots->botInfo.botAddrPos.coordinates[z].x,Robots->botInfo.botAddrPos.coordinates[z].y);
      }
      break;
    case 0x04:
      while(BotLoc[Robots->botInfo.botAddrPos.robotAddress].Stp==FALSE)
      {
        for(q=0;q<=(Robots->payloadLength-5)/2;q++)
        {
          RobotStep(Robots->botInfo.botAddrPos.robotAddress,Robots->botInfo.botAddrPos.coordinates[q].x,Robots->botInfo.botAddrPos.coordinates[q].y);
        }
        RobotStep(Robots->botInfo.botAddrPos.robotAddress,Robots->botInfo.botAddrPos.coordinates[0].x,Robots->botInfo.botAddrPos.coordinates[0].y);
      }
      break;
    }
    Free(iBfr);
    iBfr=NULL;
  }
}

void RobotStep(CPU_INT08U Robot,CPU_INT08U aX,CPU_INT08U aY)
{
  Que *oBfr=NULL;
  while((BotLoc[Robot].xPosition != aX || BotLoc[Robot].yPosition != aY) && !BotLoc[Robot].Stp)
  {
    CPU_INT08U CurrentXPos=BotLoc[Robot].xPosition;
    CPU_INT08U CurrentYPos=BotLoc[Robot].yPosition;
    CPU_INT08U differencex = aX - CurrentXPos;
    CPU_INT08U differencey = aY - CurrentYPos;
    
    if(oBfr==NULL)
      oBfr=Allocate();  
    OSMutexPend(&Mutex,NULL,OS_OPT_PEND_BLOCKING,NULL,&osErr);
    
    Direction = StepLogic(CurrentXPos,CurrentYPos,differencex,differencey);
    AddByte(oBfr,Direction);
    AddByte(oBfr,0x07);
    AddByte(oBfr,Robot);
    
    OS_ERR osErr;
    OSQPost(&FramerQueue,oBfr, sizeof(Que),OS_OPT_POST_FIFO,&osErr);
    assert(osErr==OS_ERR_NONE);
    
    if(!Direction==0)
    {
      factoryFloor[BotLoc[Robot].xPosition][BotLoc[Robot].yPosition]=0;
    }
    else
    {
      BotLoc[Robot].Try=BotLoc[Robot].Try+1;
    }
    HereIAM(Robot);
    OSMutexPost(&Mutex,OS_OPT_POST_NONE,&osErr);
    if(BotLoc[Robot].Try>=10)
    {
      InitBfr(oBfr);
      BotLoc[Robot].Stp=TRUE;
      AddByte(oBfr,0x10 | Robot);
      AddByte(oBfr,0x0B);
      OS_ERR osErr;
      OSQPost(&FramerQueue,oBfr,sizeof(Que),OS_OPT_POST_FIFO,&osErr);
      assert(osErr==OS_ERR_NONE);
    }
     oBfr=NULL;
  }
}

CPU_INT08U StepLogic(CPU_INT08U Presentx,CPU_INT08U Presenty,CPU_INT08S difx,CPU_INT08S dify)
{
  CPU_INT08U nextstep;
  if(difx>0 && dify==0)
  {
    if(factoryFloor[Presentx+1][Presenty]==0)
      nextstep=E;
    else if(factoryFloor[Presentx+1][Presenty+1]==0)
      nextstep=NE;
    else if(factoryFloor[Presentx+1][Presenty-1]==0)
      nextstep=SE;
    else if(factoryFloor[Presentx][Presenty+1]==0)
      nextstep=N;
    else if(factoryFloor[Presentx][Presenty-1]==0)
      nextstep=S;
    else if(factoryFloor[Presentx-1][Presenty+1]==0)
      nextstep=NW;
    else if(factoryFloor[Presentx-1][Presenty-1]==0)
      nextstep=SW;
    else if(factoryFloor[Presentx-1][Presenty]==0)
      nextstep=W;
    else
      nextstep=NoStep;
  }
  else if(difx<0 && dify==0)
  {
    if(factoryFloor[Presentx-1][Presenty]==0)
      nextstep=W;
    else if(factoryFloor[Presentx-1][Presenty+1]==0)
      nextstep=NW;
    else if(factoryFloor[Presentx-1][Presenty-1]==0)
      nextstep=SW;
    else if(factoryFloor[Presentx][Presenty+1]==0)
      nextstep=N;
    else if(factoryFloor[Presentx][Presenty-1]==0)
      nextstep=S;
    else if(factoryFloor[Presentx+1][Presenty+1]==0)
      nextstep=NE;
    else if(factoryFloor[Presentx+1][Presenty-1]==0)
      nextstep=SE;
    else if(factoryFloor[Presentx+1][Presenty]==0)
      nextstep=E;
    else
      nextstep=NoStep;
  }
  else if(difx==0 && dify>0)
  {
    if(factoryFloor[Presentx][Presenty+1]==0)
      nextstep=N;
    else if(factoryFloor[Presentx-1][Presenty+1]==0)
      nextstep=NW;
    else if(factoryFloor[Presentx+1][Presenty+1]==0)
      nextstep=NE;
    else if(factoryFloor[Presentx-1][Presenty]==0)
      nextstep=W;
    else if(factoryFloor[Presentx+1][Presenty]==0)
      nextstep=E;
    else if(factoryFloor[Presentx-1][Presenty-1]==0)
      nextstep=SW;
    else if(factoryFloor[Presentx+1][Presenty-1]==0)
      nextstep=SE;
    else if(factoryFloor[Presentx][Presenty-1]==0)
      nextstep=S;
    else
      nextstep=NoStep;
  }
  else if(difx==0 && dify<0)
  {
    if(factoryFloor[Presentx][Presenty-1]==0)
      nextstep=S;
    else if(factoryFloor[Presentx-1][Presenty-1]==0)
      nextstep=SW;
    else if(factoryFloor[Presentx+1][Presenty-1]==0)
      nextstep=SE;
    else if(factoryFloor[Presentx-1][Presenty]==0)
      nextstep=W;
    else if(factoryFloor[Presentx+1][Presenty]==0)
      nextstep=E;
    else if(factoryFloor[Presentx-1][Presenty+1]==0)
      nextstep=NW;
    else if(factoryFloor[Presentx+1][Presenty+1]==0)
      nextstep=NE;
    else if(factoryFloor[Presentx][Presenty+1]==0)
      nextstep=N;
    else
      nextstep=NoStep;
  }
  else if(difx>0 && dify>0)
  {
    if(factoryFloor[Presentx+1][Presenty+1]==0)
      nextstep=NE;
    else if(factoryFloor[Presentx][Presenty+1]==0)
      nextstep=N;
    else if(factoryFloor[Presentx+1][Presenty]==0)
      nextstep=E;
    else if(factoryFloor[Presentx-1][Presenty+1]==0)
      nextstep=NW;
    else if(factoryFloor[Presentx+1][Presenty-1]==0)
      nextstep=SE;
    else if(factoryFloor[Presentx-1][Presenty]==0)
      nextstep=W;
    else if(factoryFloor[Presentx][Presenty-1]==0)
      nextstep=S;
    else if(factoryFloor[Presentx-1][Presenty-1]==0)
      nextstep=SW;
    else
      nextstep=NoStep;
  }
  else if(difx<0 && dify<0)
  {
    if(factoryFloor[Presentx-1][Presenty-1]==0)
      nextstep=SW;
    else if(factoryFloor[Presentx-1][Presenty]==0)
      nextstep=W;
    else if(factoryFloor[Presentx][Presenty-1]==0)
      nextstep=S;
    else if(factoryFloor[Presentx-1][Presenty+1]==0)
      nextstep=NW;
    else if(factoryFloor[Presentx+1][Presenty-1]==0)
      nextstep=SE;
    else if(factoryFloor[Presentx][Presenty+1]==0)
      nextstep=N;
    else if(factoryFloor[Presentx+1][Presenty]==0)
      nextstep=E;
    else if(factoryFloor[Presentx+1][Presenty+1]==0)
      nextstep=NE;
    else
      nextstep=NoStep;
  }
  else if(difx>0 && dify<0)
  {
    if(factoryFloor[Presentx+1][Presenty-1]==0)
      nextstep=SE;
    else if(factoryFloor[Presentx][Presenty-1]==0)
      nextstep=S;
    else if(factoryFloor[Presentx+1][Presenty]==0)
      nextstep=E;
    else if(factoryFloor[Presentx-1][Presenty-1]==0)
      nextstep=SW;
    else if(factoryFloor[Presentx+1][Presenty+1]==0)
      nextstep=NE;
    else if(factoryFloor[Presentx-1][Presenty]==0)
      nextstep=W;
    else if(factoryFloor[Presentx][Presenty+1]==0)
      nextstep=N;
    else if(factoryFloor[Presentx-1][Presenty+1]==0)
      nextstep=NW;
    else
      nextstep=NoStep;
  }
  else if(difx<0 && dify>0)
  {
    if(factoryFloor[Presentx-1][Presenty+1]==0)
      nextstep=NW;
    else if(factoryFloor[Presentx][Presenty+1]==0)
      nextstep=N;
    else if(factoryFloor[Presentx-1][Presenty]==0)
      nextstep=W;
    else if(factoryFloor[Presentx+1][Presenty+1]==0)
      nextstep=NE;
    else if(factoryFloor[Presentx-1][Presenty-1]==0)
      nextstep=SW;
    else if(factoryFloor[Presentx+1][Presenty]==0)
      nextstep=E;
    else if(factoryFloor[Presentx][Presenty-1]==0)
      nextstep=S;
    else if(factoryFloor[Presentx+1][Presenty-1]==0)
      nextstep=SE;
    else
      nextstep=NoStep;
  }
  return nextstep;
}
         
void HereIAM(CPU_INT08U CurrentRobot)
{
  Que *mBfr=NULL;
  OS_MSG_SIZE msgSize;
  OS_ERR osErr;
  
  if(mBfr==NULL)
  {
    mBfr = OSQPend(&MailQueue[CurrentRobot],NULL,OS_OPT_PEND_BLOCKING,&msgSize,NULL,&osErr);
    assert(osErr==OS_ERR_NONE);
  }
  Here *Robot = (Here *)mBfr->bfr;
  switch(Robot->Addr)
  {
  case 0x09:
    BotLoc[CurrentRobot].xPosition=Robot->sx;
    BotLoc[CurrentRobot].yPosition=Robot->sy;
    factoryFloor[BotLoc[Robot->RobAdrs].xPosition][BotLoc[Robot->RobAdrs].yPosition]=1;
    break;
  case 0x05:
    BotLoc[CurrentRobot].Stp=TRUE;
    break;
  }
  Free(mBfr);
}
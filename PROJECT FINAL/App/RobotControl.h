/*--------------------------------RobotControl.h----------------------------------*/
/*Name: Lalit Bhat
Student ID:01671833
FINAL PROJECT
*/

#ifndef ROBOTCONTROL_H
#define ROBOTCONTROL_H

#define Maximum 16
typedef struct
{
  CPU_INT08U BotAddr;
  CPU_INT08U xPosition;
  CPU_INT08U yPosition;
  CPU_INT08U xLabelNext;
  CPU_INT08U yLabelNext;
  CPU_INT08U Try;
  CPU_BOOLEAN Stp;
}BotLocation;

extern OS_MUTEX Mutex;
extern OS_Q MailQueue[Maximum];
extern OS_Q BotQueue[Maximum];
void CreateRobot(BotLocation Robot);
#endif
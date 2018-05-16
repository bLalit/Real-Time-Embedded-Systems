/*--------------------------------Payload.h----------------------------------*/
/*Name: Lalit Bhat
Student ID:01671833
Program Number: 4
*/

#ifndef Payload_H
#define Payload_H

#include "includes.h"
#include "SerIODriver.h"
#include "assert.h"
#include "BfrPair.h"
#include "Payload.h"
#include "Error.h"
#include "PktParser.h"

/*--------------------constant definitions-----------*/
#define MaxPayload 14

/*-----------------extern declarations-----------------*/

// Payload buffer pair defined in payload.c
extern BfrPair payloadBfrPair;

// Semaphores defined in payload.c
extern OS_SEM openPayloadBfrs; // Number of open payload buffers
extern OS_SEM closedPayloadBfrs; // Number of closed payload buffers

/*------------function prototypes--------------------*/
void CreatePayloadTask(void);
void PayloadTask(void *data);
void SerPrintf(CPU_CHAR *format, ...);

#endif
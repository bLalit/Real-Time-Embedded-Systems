/*----------------------------SerIODriver.h----------------------------------*/
/*Name: Lalit Bhat
Student ID:01671833
Program Number: 2
*/

#ifndef SerIODriver_H
#define SerIODriver_H

#include "BfrPair.h"
#include "includes.h"

/*------------------------Fuction Prototypes----------------------------------*/

void InitSerIO(void);
CPU_INT16S PutByte(CPU_INT16S txChar);
void ServiceTx(void);
CPU_INT16S GetByte(void);
void ServiceRx(void);
void SerialISR(void);

#endif
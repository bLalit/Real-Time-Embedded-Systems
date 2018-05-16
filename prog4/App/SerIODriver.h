/*--------------------------------SerIODriver.h----------------------------------*/
/*Name: Lalit Bhat
Student ID:01671833
Program Number: 4
*/

#ifndef SerIODriver_H
#define SerIODriver_H

/*-------------------------Function Prototypes---------------------------------*/
void InitSerIO(void);
void ServiceRx(void);
CPU_INT16S GetByte(void);
void ServiceTx(void);
void TxFlush(void);
CPU_INT16S PutByte(CPU_INT16S c);
void  SerPrintf (CPU_CHAR *format, ...);
void  ISR (void);

#endif
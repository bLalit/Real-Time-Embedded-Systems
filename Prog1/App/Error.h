#ifndef Error_H
#define Error_H

typedef enum {P1Error,P2Error,P3Error,ChecksumError,PacketLengthError,DestinationAddr,UnknownMsg}erst;
    void Errortp(CPU_INT08U erst);
#endif
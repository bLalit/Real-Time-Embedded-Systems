#include <stdio.h>
#include <ctype.h>
#include <cpu.h>
#include <string.h>
#include <assert.h>
#include "PktParser.h"
#include "Error.h"
#include "includes.h"
#define HeaderLength 4
#define P1Char 0x03
#define P2Char 0xEF
#define P3Char 0xAF
#define nilchar 0x00
#define MinimumPacketLength 8
#define CheckSumVerify 0


/*-----------general PaketParser states--------------*/

typedef enum{P1,P2,P3,PLEN,A,CSUM,ERR} ParserState;

/*-----------Defined Packet Structure----------------*/

typedef struct
{
  CPU_INT08U dataLen;
  CPU_INT08U data[1];

}PktBfr;

void parsePkt(void *payloadBfr)
{
  ParserState parseState = P1;   /*--Current parser state*/
  
  CPU_INT08U x;                   /*--Next state*/
  CPU_INT08U i=nilchar;
  CPU_INT08U checkSum;
  
  PktBfr *pktBfr = payloadBfr;  /*--Making the payloadBfr look like a pktBfr*/
  
  for (;;)
    {
    /*-- Get the next byte from the packet*/
    x = GetByte();
    switch (parseState)
      {
       case P1:
          if (x == P1Char)
           {
            parseState = P2;
            checkSum = x;
           }
          else
           {
            Errortp(P1Error);
            parseState = ERR; /*priamble1 err code to be written*/
           }
          break;
       
       case P2:
          if (x == P2Char)
           {
            parseState = P3;
            checkSum = checkSum^x;
           }
          else
           {
            Errortp(P2Error);
            parseState = ERR; /*priamble3 err code to be written*/
           }
          break;
          
       case P3:
          if (x == P3Char)
           { parseState = PLEN;
             checkSum = checkSum^x;
           }
          else
            {
              Errortp(P3Error);
              parseState = ERR; /*priamble3 err code to be written*/
            }
          break;
       
       case PLEN:
          pktBfr->dataLen = x - HeaderLength;
          parseState = A;
          checkSum = checkSum^x;
          i=0;
          
          if (x<MinimumPacketLength)
             {
               Errortp(PacketLengthError);
               parseState = ERR;
             }
          break;
          
       case A:
          pktBfr->data[i++] = x;
          checkSum = checkSum^x;
          if (i >= pktBfr->dataLen) //check this one more time
              {
              parseState = CSUM;
              }
          break;
       
       case CSUM:
           checkSum = checkSum^x;
           if(checkSum == CheckSumVerify)
           {
             parseState = P1;
             return;
           }
           else
           {
             Errortp(ChecksumError);
             parseState = ERR;
           }
          break;
        
       case ERR:
          if (x == P1Char)
          {
            checkSum = x;
            parseState = P2;
          }
         break;
            
      
      }
     } 
  }
#ifndef PROJGLOBALS_H
#define PROJGLOBALS_H

#define CSPos -1 //checksum at end
#define LengthPos 5


typedef enum
{
	TempMsg           = 0x01,
   PresMsg 				= 0x02,
   HumMsg				= 0x03,
	WindMsg				= 0x04,
	RadMsg	 			= 0x05,
	DateTimeMsg			= 0x06,
	PrecMsg				= 0x07,
   IDMsg             = 0x08,
   MaxMsgType        = 0x08
} PktType;

typedef enum
{
#if 0
	P1Char = 0x80,
	P2Char = 0x40,
	P3Char = 0x20
#else
	P1Char = 0x03,
	P2Char = 0xEF,
	P3Char = 0xAF
#endif	
} PreambleBytes;


#define BitsPerByte 8
#define ByteMask 0x00FF
#define HourBit 27
#define HourMask 0x1F
#define MinBit 21
#define MinMask 0x3F

#define MonthBit 5
#define MonthMask 0x000F
#define DayBit 0
#define DayMask 0x1F
#define YearBit 9
#define YearMask 0x0FFF

#endif
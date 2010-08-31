/** @file
	@author Jukka Jylänki

	This work is copyrighted material and may NOT be used for any kind of commercial or 
	personal advantage and may NOT be copied or redistributed without prior consent
	of the author(s). 
*/
//namespace clb
//{

#ifdef NAALI_CLIENT

#include "CoreTypes.h"

typedef bool bit;
typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned long dword;

typedef unsigned long long u64;

typedef signed char s8;
typedef signed long long s64;

#else

typedef bool bit;
typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned long dword;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef signed char s8;
typedef signed short s16;
typedef signed int s32;
typedef signed long long s64;

typedef unsigned long ulong;
typedef unsigned int uint;

#endif

//}

#ifdef SCEPSP
#include "clb_PSP/clbPSPTypes.h"
#endif

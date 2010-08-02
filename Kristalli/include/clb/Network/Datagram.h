/** @file
	@author Jukka Jylänki

	This work is copyrighted material and may NOT be used for any kind of commercial or 
	personal advantage and may NOT be copied or redistributed without prior consent
	of the author(s). 

	@brief
*/
#ifndef Datagram_h
#define Datagram_h

static const unsigned long cDatagramBufferSize = 3 * 512;

/// @internal Datagram stores the raw data of a received UDP datagram.
struct Datagram
{
	unsigned char data[cDatagramBufferSize];

	unsigned long size;
};

#endif

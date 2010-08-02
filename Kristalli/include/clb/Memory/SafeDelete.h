/** @file SafeDelete.h
	@author Jukka Jylänki

	This work is copyrighted material and may NOT be used for any kind of commercial or 
	personal advantage and may NOT be copied or redistributed without prior consent
	of the author(s). 

	@brief Defines common idiomatic delete and COM Release macros, SAFE_DELETE, SAFE_RELEASE AND SAFE_DELETEARRAY.
*/
#ifndef SafeDelete_h
#define SafeDelete_h

/// Safe ->Release() of a raw pointer.
#define SAFE_RELEASE(p) { if (p) p->Release(); p = 0; }

/// Safe delete of a raw pointer.
#define SAFE_DELETE(p) { delete p; p = 0; }

/// Safe delete[] of a raw pointer.
#define SAFE_DELETEARRAY(p) { delete[] p; p = 0; }

#endif

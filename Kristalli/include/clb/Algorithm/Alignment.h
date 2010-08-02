/** @file
	@author Jukka Jylänki

	This work is copyrighted material and may NOT be used for any kind of commercial or 
	personal advantage and may NOT be copied or redistributed without prior consent
	of the author(s). 
*/
#ifndef clb_Util_Alignment_h
#define clb_Util_Alignment_h

#include "clb/Core/Types.h"
#include "clb/Core/clbAssert.h"

namespace clb
{

/// Is value an exact power of 2? i.e. 1,2,4,8,16,...
#define IS_POW2(value) (((value) & ((value)-1)) == 0)

/// Is the given pointer aligned to the pow2-boundary specified by alignment?
inline bool IsPow2Aligned(uintptr_t pointer, u32 alignment)
{
	assert(IS_POW2(alignment));
	return (pointer & (alignment - 1)) == 0;
}

/// @return The given pointer aligned up to the next pow2-boundary specified by alignment. (Alignment must be a pow2)
inline uintptr_t AlignUpPow2(uintptr_t pointer, u32 alignment)
{
	assert(IS_POW2(alignment));
	return (pointer + alignment - 1) & ~((uintptr_t)alignment - 1);
}

/// @return The given pointer aligned down to the previous pow2-boundary specified by alignment. (Alignment must be a pow2)
inline uintptr_t AlignDownPow2(uintptr_t pointer, u32 alignment)
{
	assert(IS_POW2(alignment));
	return pointer & ~((uintptr_t)alignment - 1);
}

inline u32 RoundUpToNextPow2(u32 x)
{
	x = x - 1; 
	x = x | (x >>  1); 
	x = x | (x >>  2); 
	x = x | (x >>  4); 
	x = x | (x >>  8); 
	x = x | (x >> 16); 
	return x + 1; 
}

} // ~clb

#endif

/** @file SortCmp.h
	@author Jukka Jylänki

	This work is copyrighted material and may NOT be used for any kind of commercial or 
	personal advantage and may NOT be copied or redistributed without prior consent
	of the author(s). 
*/
#ifndef SortCmp_h
#define SortCmp_h

#include "clb/Core/clbAssert.h"

namespace clb
{
namespace sort
{

template<typename T>
int TriCmp(const T &a, const T &b)
{
//	assume(!(a < b && b < a));
	if (a < b) return -1;
	if (b < a) return 1;
//	assert(a == b); // Require trichotomy.

	return 0;
}

template<typename T>
class TriCmpObj
{
public:
	int operator ()(const T &a, const T &b)
	{
		if (a < b) return -1;
		if (b < a) return 1;
		assert(a == b); // Require trichotomy.
		return 0;
	}
};

} // ~sort
} // ~clb

#endif

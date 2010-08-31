/** @file MathOps.h
	@author Jukka Jylänki

	This work is copyrighted material and may NOT be used for any kind of commercial or 
	personal advantage and may NOT be copied or redistributed without prior consent
	of the author(s). 
*/
#ifndef MathOps_h
#define MathOps_h

#include "clb/Core/clbAssert.h"

namespace clb
{
namespace math
{

/** @return a clamped to the range [min, max]. */
template<typename T>
const T Clamp(const T &a, const T &min, const T &max)
{
	assert(min <= max);
	return a <= min ? min : (a >= max ? max : a);
}

/** Limits a to the range [min, max], operates in-place. */
template<typename T>
void ClampIP(T &a, const T &min, const T &max)
{
	assert(min <= max);
	if (a < min) a = min;
	else if (a > max) a = max;
}

/** @return The smaller of two values. */
template<typename T>
const T Min(const T &a, const T &b)
{
	return a < b ? a : b;
}

/** Sets a equal to the smaller of {a,b}. */
template<typename T>
void MinIP(T &a, const T &b)
{
	if (b < a) a = b;
}

/** @return The larger of two values. */
template<typename T>
const T Max(const T &a, const T &b)
{
	return a >= b ? a : b;
}

/** @return True if a > b. */
template<typename T>
bool GT(const T &a, const T &b)
{
	return a > b;
}

/** @return True if a < b. */
template<typename T>
bool LT(const T &a, const T &b)
{
	return a < b;
}

/** Sets a equal to the larger of {a,b}. */
template<typename T>
void MaxIP(T &a, const T &b)
{
	if (b > a) a = b;
}


/** @return The absolute value of a. */
template<typename T>
const T Abs(const T &a)
{
	return a >= 0 ? a : -a;
}

/** Sets a to its absolute value. */
template<typename T>
void AbsIP(T &a)
{
	if (a < 0) a = -a;
}

template<typename T>
bool Equal(const T &a, const T &b)
{
	return a == b;
}

/** Compares the two values for equality, allowing the given amount of absolute error. */
template<typename T>
bool EqualAbsT(const T &a, const T &b, const T &epsilon)
{
	return Abs(a-b) < epsilon;
}

/** Compares the two values for equality, allowing the given amount of absolute error. */
bool EqualAbs(float a, float b, float epsilon = 1e-6f);

/** Compares the two values for equality, allowing the given amount of relative error. 
	Beware that for values very near 0, the relative error is significant. */
template<typename T>
bool EqualRelT(const T &a, const T &b, const T &maxRelError)
{
	if (a == b) return true; // Handles the special case where a and b are both zero.
	float relativeError = Abs((a-b)/b);
	return relativeError <= maxRelError;
}

/** Compares the two values for equality, allowing the given amount of relative error. 
	Beware that for values very near 0, the relative error is significant. */
bool EqualRel(float a, float b, float maxRelError = 1e-5f);

/** Compares two floats interpreted as integers, see 
	http://www.cygnus-software.com/papers/comparingfloats/comparingfloats.htm 
	Warning: This comparison is not safe with NANs or INFs. */
bool EqualUlps(float a, float b, int maxUlps = 10000);

}
}

#endif

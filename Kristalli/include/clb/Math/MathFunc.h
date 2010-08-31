/** @file
	@author Jukka Jylänki

	This work is copyrighted material and may NOT be used for any kind of commercial or 
	personal advantage and may NOT be copied or redistributed without prior consent
	of the author(s). 

	@brief Common mathematical functions.
*/
#ifndef MathFunc_h
#define MathFunc_h

#include <cmath>

#include "clb/Core/Types.h"
#include "MathSettings.h"

namespace clb
{
namespace math
{

inline real Cos(angle_t angle) { return std::cos(angle); }
inline real Sin(angle_t angle) { return std::sin(angle); }
inline real Sqrt(real v) { return std::sqrt(v); }
inline real Pow(real base, real exp) { return std::pow(base, exp); }

/// Integral base to an integral power.
template<u32 Base, u32 Power>
class PowIntT
{
public:
	enum { val = Base * PowIntT<Base,Power-1>::val };
};

/** @cond FULL */

/// End recursion for Base^1.
template<u32 Base>
class PowIntT<Base, 1>
{
public:
	enum { val = Base };
};
/// @endcond

/// Factorial<N> unfolds to N!.
template<int N>
class FactorialT
{
public:
	enum { val = N * FactorialT<N-1>::val };
};

/** @cond FULL */

/// Specialize 0! = 1 to end factorial recursion.
template<>
class FactorialT<0>
{
public:
	enum { val = 1 };
};
/// @endcond

/// Combinatorial<N, K> unfolds to (N nCr K).
template<int N, int K>
class CombinatorialT
{
public:
	enum { val = CombinatorialT<N-1,K-1>::val + CombinatorialT<N-1,K>::val };
};

/** @cond FULL */

/// Specialize (N nCr 0) = 1 to end recursion.
template<int N>
class CombinatorialT<N, 0>
{
public:
	enum { val = 1 };
};

/// Specialize (N nCr N) = 1 to end recursion.
template<int N>
class CombinatorialT<N, N>
{
public:
	enum { val = 1 };
};
/// @endcond

/// Calculates n! at runtime. Use class Factorial<N> to evaluate factorials at compile-time.
int Factorial(int n); 

/// Calculates (N nCr K) at runtime with recursion, running time is exponential to n. 
/// Use class Combinatorial<N, K> to evaluate combinatorials at compile-time.
int CombinatorialRec(int n, int k);

/// Calculates (N nCr K) at runtime, running time is proportional to n*k. 
/// Use class Combinatorial<N, K> to evaluate combinatorials at compile-time.
int CombinatorialTab(int n, int k);

/// Raises a float to an integer power.
float PowInt(float base, int exponent);

}
}

#endif

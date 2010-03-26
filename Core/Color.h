// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Core_Color_h
#define incl_Core_Color_h

#include "CoreTypes.h"

//! A 4-component color value.
class Color
{
public:
    Real r;
    Real g;
    Real b;
    Real a;
    
    Color() : r(0.0), g(0.0), b(0.0), a(1.0)
    {
    }
    
    Color(Real nr, Real ng, Real nb) : r(nr), g(ng), b(nb), a(1.0)
    {
    }
    
    Color(Real nr, Real ng, Real nb, Real na) : r(nr), g(ng), b(nb), a(na)
    {
    }
};

#endif
// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Core_Color_h
#define incl_Core_Color_h

#include "CoreTypes.h"

#include <QMetaType>
//! A 4-component color value.
class Color
{
public:
    float r;
    float g;
    float b;
    float a;
    
    Color() : r(0.0), g(0.0), b(0.0), a(1.0)
    {
    }
    
    Color(float nr, float ng, float nb) : r(nr), g(ng), b(nb), a(1.0)
    {
    }
    
    Color(float nr, float ng, float nb, float na) : r(nr), g(ng), b(nb), a(na)
    {
    }
    
    bool operator == (const Color& rhs) const
    {
        return (r == rhs.r) && (g == rhs.g) && (b == rhs.b) && (a == rhs.a);
    }
    
    bool operator != (const Color& rhs) const
    {
        return !(*this == rhs);
    }
};

Q_DECLARE_METATYPE(Color)

#endif
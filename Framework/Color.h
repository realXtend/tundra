// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#include "CoreTypes.h"
#include "Math/MathFunc.h"

#include <QColor>
#include <QMetaType>
/// A 4-component color value.
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

    Color(const Color &color) :
    r(color.r),
    g(color.g),
    b(color.b),
    a(color.a)
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

#ifdef QT_INTEROP
    operator QColor() const { return QColor(Clamp<int>(r*255.f, 0, 255), 
                                      Clamp<int>(g*255.f, 0, 255),
                                      Clamp<int>(b*255.f, 0, 255),
                                      Clamp<int>(a*255.f, 0, 255)); }
#endif
};

Q_DECLARE_METATYPE(Color)


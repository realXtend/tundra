// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#include "CoreTypes.h"
#include "Math/MathFunc.h"

#include <QMetaType>
#ifdef QT_INTEROP
#include <QColor>
#endif
#ifdef OGRE_INTEROP
#include <OgreColourValue.h>
#endif

/// A 4-component color value, component values are floating-points [0.0, 1.0]
class Color
{
public:
    float r;
    float g;
    float b;
    float a;

    /// The default ctor initializes values to 0.f, 0.f, 0.f, 1.f (opaque black).
    Color() : r(0.0f), g(0.0f), b(0.0f), a(1.0f)
    {
    }

    Color(const Color &color) : r(color.r), g(color.g), b(color.b), a(color.a)
    {
    }

    Color(float nr, float ng, float nb) : r(nr), g(ng), b(nb), a(1.0f)
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
    operator QColor() const
    {
        return QColor(Clamp<int>(r*255.f, 0, 255), Clamp<int>(g*255.f, 0, 255), Clamp<int>(b*255.f, 0, 255), Clamp<int>(a*255.f, 0, 255));
    }
#endif
#ifdef OGRE_INTEROP
    Color(const Ogre::ColourValue &other) { r = other.r; g = other.g; b = other.b; a = other.a; }
    operator Ogre::ColourValue() const { return Ogre::ColourValue(r, g, b, a); }
#endif
};

Q_DECLARE_METATYPE(Color)

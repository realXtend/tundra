/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   Color.cpp
    @brief  A 4-component color value, component values are floating-points [0.0, 1.0]. */

//#include "StableHeaders.h"
//#include "DebugOperatorNew.h"
#include "Color.h"
#include "Math/MathFunc.h"
#include "Math/float4.h"

#include <stdio.h>
#include <stdlib.h>
//#include "MemoryLeakCheck.h"

#ifdef MATH_QT_INTEROP
Color::operator QColor() const
{
    return QColor(Clamp<int>(r*255.f, 0, 255), Clamp<int>(g*255.f, 0, 255), Clamp<int>(b*255.f, 0, 255), Clamp<int>(a*255.f, 0, 255));
}

Color::operator QString() const
{
    char str[256];
    sprintf(str, "Color(%.3f, %.3f, %.3f, %.3f)", r, g, b, a);
    return str;
}

QString Color::SerializeToString() const
{
    char str[256];
    sprintf(str, "%f %f %f %f", r, g, b, a);
    return str;
}
#endif

Color Color::FromString(const char *str)
{
    assume(str);
    if (!str)
        return Color();
    if (*str == '(')
        ++str;
    Color c;
    c.r = (float)strtod(str, const_cast<char**>(&str));
    if (*str == ',' || *str == ';')
        ++str;
    c.g = (float)strtod(str, const_cast<char**>(&str));
    if (*str == ',' || *str == ';')
        ++str;
    c.b = (float)strtod(str, const_cast<char**>(&str));
    if (*str == ',' || *str == ';')
        ++str;
    if (str && *str != '\0') // alpha optional
        c.a = (float)strtod(str, const_cast<char**>(&str));
    return c;

}

Color::operator float4() const
{
    return float4(r, g, b, a);
}

float4 Color::ToFloat4() const
{
    return (float4)*this;
}

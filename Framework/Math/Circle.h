/** @file
    @author Jukka Jylänki

    This work is copyrighted material and may NOT be used for any kind of commercial or 
    personal advantage and may NOT be copied or redistributed without prior consent
    of the author(s). 
*/
#pragma once

#include "MathFwd.h"
#include "float3.h"

/// A two-dimensional circle in 3D space.
class Circle
{
public:
    /// The default ctor does not initialize the Circle to any value.
    Circle() {}

    float3 center;
    float3 normal;
    /// The radius of the circle.
    float r;

    bool Contains(const float3 &point) const;
};

/** @file
    @author Jukka Jylänki

    This work is copyrighted material and may NOT be used for any kind of commercial or 
    personal advantage and may NOT be copied or redistributed without prior consent
    of the author(s). 
*/
#pragma once

#include "MathFwd.h"
#include "float3.h"

class Triangle
{
public:
    float3 a;
    float3 b;
    float3 c;

    /// Expresses the given point in barycentric (u,v,w) coordinates with respect to this triangle.
    void Barycentric(const float3 &point, float &u, float &v, float &w) const;

    /// Returns true if this triangle is degenerate.
    bool IsDegenerate() const;

    /// Returns true if the given triangle defined by the three given points is degenerate (zero surface area).
    static bool IsDegenerate(const float3 &p1, const float3 &p2, const float3 &p3, float epsilon = 1e-3f);
};

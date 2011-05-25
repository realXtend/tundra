/** @file
    @author Jukka Jylänki

    This work is copyrighted material and may NOT be used for any kind of commercial or 
    personal advantage and may NOT be copied or redistributed without prior consent
    of the author(s). 
*/
#pragma once

#include "MathFwd.h"
#include "float3.h"

// A 3D generalization of an ellipse.
class Ellipsoid
{
public:
    Ellipsoid() {}

    // Bounding Ellipsoid construction.

    bool Contains(const float3 &point) const;
};

// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "CoreDefines.h"
#include "PhysicsModuleApi.h"
#include "Math/float3.h"

class btConvexHullShape;

namespace Physics
{
/** @cond PRIVATE */
struct ConvexHull
{
    float3 position_;
    boost::shared_ptr<btConvexHullShape> hull_;
};

struct ConvexHullSet
{
    std::vector<ConvexHull> hulls_;
};
/** @endcond */
}

// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "CoreDefines.h"
#include "PhysicsModuleApi.h"
#include "PhysicsModuleFwd.h"
#include "Math/float3.h"

namespace Physics
{
/** @cond PRIVATE */
struct ConvexHull
{
    float3 position_;
    shared_ptr<btConvexHullShape> hull_;
};

struct ConvexHullSet
{
    std::vector<ConvexHull> hulls_;
};
/** @endcond */
}

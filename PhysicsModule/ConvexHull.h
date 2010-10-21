// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Physics_ConvexHull_h
#define incl_Physics_ConvexHull_h

#include "Core.h"
#include "PhysicsModuleApi.h"

class btConvexHullShape;

namespace Physics
{

struct ConvexHull
{
    Vector3df position_;
    boost::shared_ptr<btConvexHullShape> hull_;
};

struct ConvexHullSet
{
    std::vector<ConvexHull> hulls_;
};

}

#endif
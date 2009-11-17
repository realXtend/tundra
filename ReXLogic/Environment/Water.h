/// @file Water.h
/// @brief Manages Water-related Rex logic.
/// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Water_h
#define incl_Water_h

#include "EntityComponent/EC_Water.h"

namespace RexLogic
{

class Water
{
public:
    Water(RexLogicModule *owner_);
    ~Water();

    //! Looks through all the entities in RexLogic's currently active scene to find the Water
    //! entity. Caches it internally. Use GetWaterEntity to obtain it afterwards.
    void FindCurrentlyActiveWater();

    //! @return The scene entity that represents the terrain in the currently active world.
    Scene::EntityWeakPtr GetWaterEntity();

    void CreateWaterGeometry();

private:
    RexLogicModule *owner_;

    Scene::EntityWeakPtr cachedWaterEntity_;
};

}

#endif

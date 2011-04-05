/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_ProximityTrigger.cpp
 *  @brief  EC_ProximityTrigger reports distance, each frame, to other entities with EC_ProximityTrigger
 *  @note   The entity should have EC_Placeable available in advance.
 */

#include "StableHeaders.h"
#include "EC_ProximityTrigger.h"

#include "Entity.h"
#include "SceneManager.h"
#include "EC_Placeable.h"
#include "LoggingFunctions.h"
#include "FrameAPI.h"

DEFINE_POCO_LOGGING_FUNCTIONS("EC_ProximityTrigger")

EC_ProximityTrigger::EC_ProximityTrigger(IModule *module)
    :IComponent(module->GetFramework()),
    active(this, "Is active", true),
    thresholdDistance(this, "Threshold distance", 0.0f)
{
    connect(framework_->Frame(), SIGNAL(Updated(float)), this, SLOT(Update(float)));
    connect(this, SIGNAL(OnAttributeChanged(IAttribute*, AttributeChange::Type)),
            SLOT(AttributeUpdated(IAttribute*)));
}

EC_ProximityTrigger::~EC_ProximityTrigger()
{
}

void EC_ProximityTrigger::AttributeUpdated(IAttribute* attr)
{
}

void EC_ProximityTrigger::Update(float timeStep)
{
    if (!active.Get())
        return;
    float threshold = thresholdDistance.Get();
    
    Scene::Entity* entity = GetParentEntity();
    if (!entity)
        return;
    Scene::SceneManager* mgr = entity->GetScene();
    if (!mgr)
        return;
    EC_Placeable* placeable = entity->GetComponent<EC_Placeable>().get();
    if (!placeable)
        return;
    
    Scene::EntityList otherTriggers = mgr->GetEntitiesWithComponent(EC_ProximityTrigger::TypeNameStatic());
    for (Scene::EntityList::iterator i = otherTriggers.begin(); i != otherTriggers.end(); ++i)
    {
        Scene::Entity* otherEntity = (*i).get();
        if (otherEntity != entity)
        {
            EC_Placeable* otherPlaceable = otherEntity->GetComponent<EC_Placeable>().get();
            if (!otherPlaceable)
                continue;
            Vector3df offset = placeable->transform.Get().position - otherPlaceable->transform.Get().position;
            float distance = offset.getLength();
            
            if ((threshold <= 0.0f) || (distance <= threshold))
                emit Triggered(otherEntity, distance);
        }
    }
}

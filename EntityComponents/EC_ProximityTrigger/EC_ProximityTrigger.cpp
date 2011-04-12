/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_ProximityTrigger.cpp
 *  @brief  EC_ProximityTrigger reports distance, each frame, of other entities that also have EC_ProximityTrigger component
 */

#include "StableHeaders.h"
#include "EC_ProximityTrigger.h"

#include "Entity.h"
#include "SceneManager.h"
#include "EC_Placeable.h"
#include "LoggingFunctions.h"
#include "FrameAPI.h"

DEFINE_POCO_LOGGING_FUNCTIONS("EC_ProximityTrigger")

EC_ProximityTrigger::EC_ProximityTrigger(IModule *module) :
    IComponent(module->GetFramework()),
    active(this, "Is active", true),
    thresholdDistance(this, "Threshold distance", 0.0f),
    period(this, "Period", 0.0f)
{
    SetUpdateMode();
    connect(this, SIGNAL(AttributeChanged(IAttribute*, AttributeChange::Type)), SLOT(OnAttributeUpdated(IAttribute*)));
}

EC_ProximityTrigger::~EC_ProximityTrigger()
{
}

void EC_ProximityTrigger::OnAttributeUpdated(IAttribute* attr)
{
    if (attr == &period)
        SetUpdateMode();
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
            {
                emit Triggered(otherEntity, distance);
            }
        }
    }
}

void EC_ProximityTrigger::SetUpdateMode()
{
    FrameAPI* frame = framework_->Frame();
    
    float perSec = period.Get();
    if (perSec <= 0.0f)
    {
        // Update every frame
        connect(frame, SIGNAL(Updated(float)), this, SLOT(Update(float)));
    }
    else
    {
        // Update periodically
        disconnect(frame, SIGNAL(Updated(float)), this, SLOT(Update(float)));
        frame->DelayedExecute(perSec, this, SLOT(PeriodicUpdate()));
    }
}

void EC_ProximityTrigger::PeriodicUpdate()
{
    // Set up the next periodic update
    float perSec = period.Get();
    if (perSec > 0.0f)
        framework_->Frame()->DelayedExecute(perSec, this, SLOT(PeriodicUpdate()));
    
    Update(perSec);
}


/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   EC_ProximityTrigger.cpp
    @brief  Reports distance, each frame, of other entities that also have this same component. */

#include "EC_ProximityTrigger.h"

#include "Framework.h"
#include "Scene/Scene.h"
#include "Entity.h"

#include "EC_Placeable.h"
#include "LoggingFunctions.h"
#include "FrameAPI.h"

EC_ProximityTrigger::EC_ProximityTrigger(Scene *scene) :
    IComponent(scene),
    active(this, "Is active", true),
    thresholdDistance(this, "Threshold distance", 0.0f),
    interval(this, "Trigger signal interval", 0.0f)
{
    SetUpdateMode();
}

EC_ProximityTrigger::~EC_ProximityTrigger()
{
}

void EC_ProximityTrigger::AttributesChanged()
{
    if (interval.ValueChanged())
        SetUpdateMode();
}

void EC_ProximityTrigger::Update(float /*timeStep*/)
{
    if (!active.Get())
        return;
    float threshold = thresholdDistance.Get();
    
    Entity* entity = ParentEntity();
    if (!entity)
        return;
    Scene* scene = entity->ParentScene();
    if (!scene)
        return;
    EC_Placeable* placeable = entity->GetComponent<EC_Placeable>().get();
    if (!placeable)
        return;
    
    EntityList otherTriggers = scene->GetEntitiesWithComponent(EC_ProximityTrigger::TypeNameStatic());
    for(EntityList::iterator i = otherTriggers.begin(); i != otherTriggers.end(); ++i)
    {
        Entity* otherEntity = (*i).get();
        if (otherEntity != entity)
        {
            EC_Placeable* otherPlaceable = otherEntity->GetComponent<EC_Placeable>().get();
            if (!otherPlaceable)
                continue;
            float3 offset = placeable->transform.Get().pos - otherPlaceable->transform.Get().pos;
            float distance = offset.Length();
            
            if ((threshold <= 0.0f) || (distance <= threshold))
            {
                emit triggered(otherEntity, distance);
            }
        }
    }
}

void EC_ProximityTrigger::SetUpdateMode()
{
    FrameAPI* frame = framework->Frame();
    
    float intervalSec = interval.Get();
    if (intervalSec <= 0.0f)
    {
        // Update every frame
        connect(frame, SIGNAL(Updated(float)), this, SLOT(Update(float)), Qt::UniqueConnection);
    }
    else
    {
        // Update periodically
        disconnect(frame, SIGNAL(Updated(float)), this, SLOT(Update(float)));
        frame->DelayedExecute(intervalSec, this, SLOT(PeriodicUpdate()));
    }
}

void EC_ProximityTrigger::PeriodicUpdate()
{
    // Set up the next periodic update
    float intervalSec = interval.Get();
    if (intervalSec > 0.0f)
        framework->Frame()->DelayedExecute(intervalSec, this, SLOT(PeriodicUpdate()));
    
    Update(intervalSec);
}


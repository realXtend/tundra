// For conditions of distribution and use, see copyright notice in license.txt

/// @file Water.cpp
/// @brief Manages Water-related Rex logic.

#include "StableHeaders.h"
#include "Water.h"
#include "EnvironmentModule.h"

// Ogre renderer -specific.
#include <OgreManualObject.h>
#include <OgreSceneManager.h>
#include <OgreMaterialManager.h>
#include <OgreTextureManager.h>
#include <OgreIteratorWrappers.h>
#include <OgreTechnique.h>

#include "EC_OgrePlaceable.h"
#include "EC_WaterPlane.h"
#include "Renderer.h"
#include "OgreTextureResource.h"
#include "OgreMaterialUtils.h"
#include "SceneManager.h"
#include "EC_Water.h"

namespace Environment
{

Water::Water(EnvironmentModule *owner) :
    owner_(owner), activeWaterComponent_(0)
{
}

Water::~Water()
{
    // Does not own.
    activeWaterEntity_.reset();
    activeWaterComponent_ = 0;
    owner_ = 0;
}

Scene::EntityWeakPtr Water::GetActiveWater()
{
    /*
    Scene::ScenePtr scene = owner_->GetFramework()->GetDefaultWorldScene();

    // Check that is current water entity still valid.

    if ( !activeWaterEntity_.expired() )
        if(scene->GetEntity(activeWaterEntity_.lock()->GetId()).get() != 0)
            return activeWaterEntity_;

    // Current is not valid so search new, takes first entity which has water 

    for(Scene::SceneManager::iterator iter = scene->begin();
        iter != scene->end(); ++iter)
    {
        Scene::Entity &entity = **iter;
        activeWaterComponent_ = entity.GetComponent<EC_Water>().get();
        if (activeWaterComponent_ != 0)
        {
            activeWaterEntity_ = scene->GetEntity(entity.GetId());

            if ( !activeWaterEntity_.expired())
                return activeWaterEntity_;
        }
     }
    
    // There was any water entity so reset it to null state. 

    activeWaterEntity_.reset();
    activeWaterComponent_ = 0;
    return Scene::EntityWeakPtr();
    */
    //return Scene::EntityWeakPtr();
    return activeWaterEntity_;
}

Scene::EntityWeakPtr Water::GetWaterEntity()
{
    // Find currently active water entity
    return GetActiveWater();
}

void Water::CreateWaterGeometry(float height)
{
    // Here we assume that there is only one water in one scene (and it is ocean). 
     
    if ( !GetActiveWater().expired())
        RemoveWaterGeometry();

    Scene::ScenePtr active_scene = owner_->GetFramework()->GetDefaultWorldScene();
    Scene::EntityPtr entity = active_scene->CreateEntity(active_scene->GetNextFreeId());
    // Create EC_OgrePlaceable
    //entity->AddComponent(owner_->GetFramework()->GetComponentManager()->CreateComponent(EC_OgrePlaceable::TypeNameStatic()));
    //EC_OgrePlaceable* comp = entity->GetComponent<EC_OgrePlaceable>().get();
    

    entity->AddComponent(owner_->GetFramework()->GetComponentManager()->CreateComponent(EC_WaterPlane::TypeNameStatic()));
    activeWaterComponent_ = entity->GetComponent<EC_WaterPlane>().get();
    activeWaterComponent_->positionAttr_.Set(Vector3df(0.0,0.0,height), AttributeChange::Network);
    //activeWaterComponent_->SetWaterHeight(height);
    activeWaterEntity_ = entity;
    active_scene->EmitEntityCreated(entity);

/*
    if ( !GetActiveWater().expired())
        RemoveWaterGeometry();

    Scene::ScenePtr active_scene = owner_->GetFramework()->GetDefaultWorldScene();
    Scene::EntityPtr entity = active_scene->CreateEntity(active_scene->GetNextFreeIdLocal());
    entity->AddComponent(owner_->GetFramework()->GetComponentManager()->CreateComponent(EC_Water::TypeNameStatic()));
    activeWaterComponent_ = entity->GetComponent<EC_Water>().get();
    activeWaterComponent_->SetWaterHeight(height);
    activeWaterEntity_ = entity;
    active_scene->EmitEntityCreated(entity);
  */

    emit WaterCreated();

 }

void Water::RemoveWaterGeometry()
{
    // Adjust that we are removing correct water
    //if( GetActiveWater().expired())
    //    return;
    
    // Remove component
    if ( activeWaterComponent_ != 0)
    {
        Scene::EntityPtr entity = activeWaterEntity_.lock();
        entity->RemoveComponent(entity->GetComponent(EC_Water::TypeNameStatic()));
        activeWaterComponent_ = 0;
    }

    if ( activeWaterEntity_.lock().get() == 0)
        return;

    // Remove entity from scene
    Scene::ScenePtr active_scene = owner_->GetFramework()->GetDefaultWorldScene();
    active_scene->RemoveEntity(activeWaterEntity_.lock()->GetId());
    activeWaterEntity_.reset();

    emit WaterRemoved();
}

void Water::SetWaterHeight(float height)
{
    if (activeWaterComponent_ != 0)
    {
        //activeWaterComponent_->SetWaterHeight(height);
        Vector3df vec = activeWaterComponent_->positionAttr_.Get();
        vec.z = height;
        activeWaterComponent_->positionAttr_.Set(vec, AttributeChange::Network);
        emit HeightChanged(static_cast<double>(height));
    }
}

float Water::GetWaterHeight() const 
{
    float height = 0.0;
    if (activeWaterComponent_ != 0)
        height = activeWaterComponent_->positionAttr_.Get().z;
        
        //height = activeWaterComponent_->GetWaterHeight();

    return height;
}

}

/// @file Water.cpp
/// @brief Manages Water-related Rex logic.
/// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"

// Ogre renderer -specific.
#include <OgreManualObject.h>
#include <OgreSceneManager.h>
#include <OgreMaterialManager.h>
#include <OgreTextureManager.h>
#include <OgreIteratorWrappers.h>
#include <OgreTechnique.h>

#include "EC_OgrePlaceable.h"
#include "Renderer.h"
#include "OgreTextureResource.h"
#include "OgreMaterialUtils.h"

#include "EnvironmentModule.h"
#include "Water.h"
#include "SceneManager.h"

namespace Environment
{

Water::Water(EnvironmentModule *owner)
: owner_(owner), waterComponent_(0)
{
    
}

Water::~Water()
{
    // Does not own.
    waterComponent_ = 0;
}

void Water::FindCurrentlyActiveWater()
{
    Scene::ScenePtr scene = owner_->GetFramework()->GetDefaultWorldScene();
    for(Scene::SceneManager::iterator iter = scene->begin();
        iter != scene->end(); ++iter)
    {
        Scene::Entity &entity = **iter;
        Foundation::ComponentInterfacePtr waterComponent = entity.GetComponent("EC_Water");
        if (waterComponent.get())
        {
            cachedWaterEntity_ = scene->GetEntity(entity.GetId());
        }
    }
}

Scene::EntityWeakPtr Water::GetWaterEntity()
{
    return cachedWaterEntity_;
}

void Water::CreateWaterGeometry()
{
    Scene::ScenePtr active_scene = owner_->GetFramework()->GetDefaultWorldScene();

    Scene::EntityPtr entity = active_scene->CreateEntity(active_scene->GetNextFreeId());
    entity->AddEntityComponent(owner_->GetFramework()->GetComponentManager()->CreateComponent("EC_Water"));
    waterComponent_ = checked_static_cast<EC_Water*>(entity->GetComponent("EC_Water").get());
    waterComponent_->SetWaterHeight(20.f);

    cachedWaterEntity_ = entity;
}

void Water::SetWaterHeight(float height)
{   
    if ( waterComponent_ != 0)
        waterComponent_->SetWaterHeight(height);
}

float Water::GetWaterHeight() const 
{
    float height = -1.0;
    if ( waterComponent_ != 0)
        height = waterComponent_->GetWaterHeight();

    return height;
}

}
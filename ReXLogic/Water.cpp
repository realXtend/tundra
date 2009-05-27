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

#include "../OgreRenderingModule/EC_OgrePlaceable.h"
#include "../OgreRenderingModule/Renderer.h"
#include "../OgreRenderingModule/OgreTextureResource.h"
#include "../OgreRenderingModule/OgreMaterialUtils.h"

#include "RexLogicModule.h"
#include "Water.h"
#include "SceneManager.h"

namespace RexLogic
{

Water::Water(RexLogicModule *owner)
:owner_(owner)
{
}

Water::~Water()
{
}

void Water::FindCurrentlyActiveWater()
{
    Scene::ScenePtr scene = owner_->GetCurrentActiveScene();
    for(Scene::SceneManager::iterator iter = scene->begin();
        iter != scene->end(); ++iter)
    {
        Scene::EntityInterface &entity = **iter;
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
    Scene::ScenePtr active_scene = owner_->GetCurrentActiveScene();

    Scene::EntityPtr entity = active_scene->CreateEntity(active_scene->GetNextFreeId());
    entity->AddEntityComponent(owner_->GetFramework()->GetComponentManager()->CreateComponent("EC_Water"));
    EC_Water *waterComponent = checked_static_cast<EC_Water*>(entity->GetComponent("EC_Water").get());
    waterComponent->SetWaterHeight(20.f);

    cachedWaterEntity_ = entity;
}

}
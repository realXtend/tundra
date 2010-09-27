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
   

    entity->AddComponent(owner_->GetFramework()->GetComponentManager()->CreateComponent(EC_WaterPlane::TypeNameStatic()));
    activeWaterComponent_ = entity->GetComponent<EC_WaterPlane>().get();
    activeWaterComponent_->positionAttr_.Set(Vector3df(0.0,0.0,height), AttributeChange::Network);
    //activeWaterComponent_->SetWaterHeight(height);
    activeWaterEntity_ = entity;
    active_scene->EmitEntityCreated(entity);

    emit WaterCreated();

 }

void Water::RemoveWaterGeometry()
{
   
    // Remove component
    if ( activeWaterComponent_ != 0)
    {
        Scene::EntityPtr entity = activeWaterEntity_.lock();
        entity->RemoveComponent(entity->GetComponent(EC_WaterPlane::TypeNameStatic()));
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

void Water::SetWaterHeight(float height, AttributeChange::Type type)
{
    if (activeWaterComponent_ != 0)
    {
        //activeWaterComponent_->SetWaterHeight(height);
        Vector3df vec = activeWaterComponent_->positionAttr_.Get();
        vec.z = height;
        activeWaterComponent_->positionAttr_.Set(vec, type);
        emit HeightChanged(static_cast<double>(height));
    }
}

float Water::GetWaterHeight() const 
{
    float height = 0.0;
    if (activeWaterComponent_ != 0)
        height = activeWaterComponent_->positionAttr_.Get().z;
       
    return height;
}

void Water::SetWaterFog(float fogStart, float fogEnd, const QVector<float>& color)
{
    if ( activeWaterComponent_ == 0)
        return;

    activeWaterComponent_->fogStartAttr_.Set(fogStart,AttributeChange::Local);
    activeWaterComponent_->fogEndAttr_.Set(fogEnd, AttributeChange::Local);
    activeWaterComponent_->fogColorAttr_.Set(Color(color[0]*255, color[1]*255, color[2]*255,255), AttributeChange::Local);
 
    emit WaterFogAdjusted(fogStart, fogEnd, color);
    
}

QVector<float> Water::GetFogWaterColor() const
{
    if ( activeWaterComponent_ == 0)
        return QVector<float>();
    
    Ogre::ColourValue color = activeWaterComponent_->GetFogColorAsOgreValue();
    QVector<float> vec; 
    vec<<color[0]<<color[1]<<color[2];
    return vec;
    
  
}

void Water::SetWaterFogColor(const QVector<float>& color)
{
   if ( activeWaterComponent_ == 0)
        return;
 
  
   Color col(color[0], color[1], color[2],1.0);
   activeWaterComponent_->fogColorAttr_.Set(col, AttributeChange::Local); 
    
}

void Water::SetWaterFogDistance(float fogStart, float fogEnd)
{
    if ( activeWaterComponent_ == 0)
        return;   

    activeWaterComponent_->fogStartAttr_.Set(fogStart, AttributeChange::Local);
    activeWaterComponent_->fogEndAttr_.Set(fogEnd, AttributeChange::Local);
    
}

float Water::GetWaterFogStartDistance() const
{
   if ( activeWaterComponent_ == 0)
        return 0.f;   

    return activeWaterComponent_->fogStartAttr_.Get();

}

float Water::GetWaterFogEndDistance() const
{
    if ( activeWaterComponent_ == 0)
        return 0.f;   

    return activeWaterComponent_->fogEndAttr_.Get();

   
}

}

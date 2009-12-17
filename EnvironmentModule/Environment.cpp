/// @file Environment.cpp
/// @brief Manages environment-related reX-logic, e.g. world time and lightning.
/// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EnvironmentModule.h"
#include "Environment.h"
#include <EC_OgreEnvironment.h>
#include <SceneManager.h>
#include <NetworkEvents.h>

#include <QVector>

namespace Environment
{

Environment::Environment(EnvironmentModule *owner) : 
    owner_(owner), 
    activeEnvComponent_(0), 
    activeEnvEntity_(Scene::EntityWeakPtr()), 
    time_override_(false),
    usecSinceStart_(0),
    secPerDay_(0),
    secPerYear_(0),
    sunDirection_(RexTypes::Vector3()),
    sunPhase_(0.0),
    sunAngVelocity_(RexTypes::Vector3())
{}

Environment::~Environment()
{
    // Does not own.
    activeEnvEntity_.reset();
    activeEnvComponent_ = 0;
    owner_ = 0;

}

Scene::EntityWeakPtr Environment::FindActiveEnvironment()
{
    Scene::ScenePtr scene = owner_->GetFramework()->GetDefaultWorldScene();

    // Check that is current environment entity still valid.

    if ( !activeEnvEntity_.expired() )
        if(scene->GetEntity(activeEnvEntity_.lock()->GetId()).get() != 0)
            return activeEnvEntity_;

    for(Scene::SceneManager::iterator iter = scene->begin();
        iter != scene->end(); ++iter)
    {
        Scene::Entity &entity = **iter;
        activeEnvComponent_ = static_cast<OgreRenderer::EC_OgreEnvironment*>(entity.GetComponent("EC_OgreEnvironment").get());
        if (activeEnvComponent_ != 0)
        {
            activeEnvEntity_ = scene->GetEntity(entity.GetId());
            if ( !activeEnvEntity_.expired())
                return activeEnvEntity_;
        }
     }
    activeEnvComponent_ = 0;
    return Scene::EntityWeakPtr();
}

Scene::EntityWeakPtr Environment::GetEnvironmentEntity()
{
    return FindActiveEnvironment();
}

void Environment::CreateEnvironment()
{
    FindActiveEnvironment();
    if ( !activeEnvEntity_.expired())
        return;

    Scene::ScenePtr active_scene = owner_->GetFramework()->GetDefaultWorldScene();
    Scene::EntityPtr entity = active_scene->CreateEntity(active_scene->GetNextFreeId());
    entity->AddEntityComponent(owner_->GetFramework()->GetComponentManager()->CreateComponent("EC_OgreEnvironment"));
    activeEnvComponent_ = static_cast<OgreRenderer::EC_OgreEnvironment*>(entity->GetComponent("EC_OgreEnvironment").get()); 
    activeEnvEntity_ = entity;
}

bool Environment::DecodeSimulatorViewerTimeMessage(ProtocolUtilities::NetworkEventInboundData *data)
{
    ProtocolUtilities::NetInMessage &msg = *data->message;
    msg.ResetReading();

    ///\ secPerDay,secPerYear, sunPhase seems to be zero, at least with 0.4 server
    usecSinceStart_ = (time_t)msg.ReadU64();
    secPerDay_ = msg.ReadU32();
    secPerYear_ = msg.ReadU32();
    sunDirection_ = msg.ReadVector3();
    sunPhase_ = msg.ReadF32();
    sunAngVelocity_ = msg.ReadVector3();

    FindActiveEnvironment();
   
    if (activeEnvEntity_.expired())
        return false;

    // Update the sunlight direction and angle velocity.
    ///\note Not needed anymore as we use Caleum now.
    //OgreRenderer::EC_OgreEnvironment &env = *checked_static_cast<OgreRenderer::EC_OgreEnvironment*>
    //    (component.get());
    //env.SetSunDirection(-sunDirection_);

    if (!time_override_ && activeEnvComponent_ != 0)
    {
        activeEnvComponent_->SetTime(usecSinceStart_);
    }

    return false;
}

void Environment::SetWaterFog(float fogStart, float fogEnd, const QVector<float>& color)
{
    FindActiveEnvironment();
    if ( activeEnvComponent_ != 0)
    {   
        activeEnvComponent_->SetWaterFogStart(fogStart);
        activeEnvComponent_->SetWaterFogEnd(fogEnd);
        Ogre::ColourValue fogColour(color[0], color[1], color[2]);
        activeEnvComponent_->SetWaterFogColor(fogColour); 
        emit WaterFogAdjusted(fogStart, fogEnd, color);
    }

   
}

void Environment::SetGroundFog(float fogStart, float fogEnd, const QVector<float>& color)
{
    FindActiveEnvironment();
    if ( activeEnvComponent_ != 0)
    {   
        activeEnvComponent_->SetGroundFogStart(fogStart);
        activeEnvComponent_->SetGroundFogEnd(fogEnd);
        Ogre::ColourValue fogColour(color[0], color[1], color[2]);
        activeEnvComponent_->SetGroundFogColor(fogColour); 
        emit GroundFogAdjusted(fogStart, fogEnd, color);
    }
}

void Environment::SetFogColorOverride(bool enabled)
{
    FindActiveEnvironment();
    if ( activeEnvComponent_ != 0)
        activeEnvComponent_->SetFogColorOverride(enabled);
    
}

bool Environment::GetFogColorOverride() 
{
    FindActiveEnvironment();
    if ( activeEnvComponent_ != 0)
        return activeEnvComponent_->GetFogColorOverride();

    return false;
}

QVector<float> Environment::GetFogGroundColor()
{
    FindActiveEnvironment();
    if ( activeEnvComponent_ != 0)
    {
        Ogre::ColourValue color = activeEnvComponent_->GetGroundFogColor();     
        QVector<float> vec; 
        vec<<color[0]<<color[1]<<color[2];
        return vec;
    }

    return QVector<float>();
}

QVector<float> Environment::GetFogWaterColor()
{
    FindActiveEnvironment();
    if ( activeEnvComponent_ != 0)
    {
        Ogre::ColourValue color = activeEnvComponent_->GetWaterFogColor();     
        QVector<float> vec; 
        vec<<color[0]<<color[1]<<color[2];
        return vec;
    }

    return QVector<float>();

   
}

void Environment::Update(Core::f64 frametime)
{
    FindActiveEnvironment();
    if (activeEnvComponent_ != 0)
        activeEnvComponent_->UpdateVisualEffects(frametime);

}

bool Environment::IsCaelum()
{
    FindActiveEnvironment();
    if (activeEnvComponent_ != 0)
        return activeEnvComponent_->IsCaleumUsed();
    
    return false;

}

void Environment::SetGroundFogColor(const QVector<float>& color)
{
    FindActiveEnvironment();
    if (activeEnvComponent_ != 0)
    {
        Ogre::ColourValue fogColour(color[0], color[1], color[2]);
        activeEnvComponent_->SetGroundFogColor(fogColour); 
    }
    
}
        
void Environment::SetWaterFogColor(const QVector<float>& color)
{
    FindActiveEnvironment();
    if (activeEnvComponent_ != 0)
    {
        Ogre::ColourValue fogColour(color[0], color[1], color[2]);
        activeEnvComponent_->SetWaterFogColor(fogColour); 
    }
}

void Environment::SetGroundFogDistance(float fogStart, float fogEnd)
{
    FindActiveEnvironment();
    if (activeEnvComponent_ != 0)
    {
        activeEnvComponent_->SetGroundFogStart(fogStart);
        activeEnvComponent_->SetGroundFogEnd(fogStart);
    }
}

void Environment::SetWaterFogDistance(float fogStart, float fogEnd)
{
    FindActiveEnvironment();
    if (activeEnvComponent_ != 0)
    {
        activeEnvComponent_->SetWaterFogStart(fogStart);
        activeEnvComponent_->SetWaterFogEnd(fogStart);
    }

}

float Environment::GetWaterFogStartDistance()
{
    FindActiveEnvironment();
    if (activeEnvComponent_ != 0)
       return activeEnvComponent_->GetWaterFogStart();
    
     return 0.0;
}

float Environment::GetWaterFogEndDistance()
{
    FindActiveEnvironment();
    if (activeEnvComponent_ != 0)
       return activeEnvComponent_->GetWaterFogEnd();
    
     return 0.0;
}

float Environment::GetGroundFogStartDistance()
{
    FindActiveEnvironment();
    if (activeEnvComponent_ != 0)
        return activeEnvComponent_->GetGroundFogStart();
    
    return 0.0;
}

float Environment::GetGroundFogEndDistance()
{
    FindActiveEnvironment();
    if (activeEnvComponent_ != 0)
       return activeEnvComponent_->GetGroundFogEnd();
    
     return 0.0;
}


}

/// @file Environment.cpp
/// @brief Manages environment-related reX-logic, e.g. world time and lightning.
/// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EnvironmentModule.h"
#include "Environment.h"
#include <EC_OgreEnvironment.h>
#include <SceneManager.h>
#include <NetworkEvents.h>

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
    
    //Foundation::ComponentPtr component = GetEnvironmentEntity().lock()->GetComponent("EC_OgreEnvironment");
    
    if (activeEnvEntity_.expired())
        return false;

    // Update the sunlight direction and angle velocity.
    ///\note Not needed anymore as we use Caleum now.
    //OgreRenderer::EC_OgreEnvironment &env = *checked_static_cast<OgreRenderer::EC_OgreEnvironment*>
    //    (component.get());
//    env.SetSunDirection(-sunDirection_);

    if (!time_override_ && activeEnvComponent_ != 0)
    {
        activeEnvComponent_->SetTime(usecSinceStart_);
    }

    return false;
}

void Environment::SetFog(float fogStart, float fogEnd, const QVector<float>& color)
{

}

void Environment::Update(Core::f64 frametime)
{
    FindActiveEnvironment();
    OgreRenderer::EC_OgreEnvironment &env = *checked_static_cast<OgreRenderer::EC_OgreEnvironment*>
        (GetEnvironmentEntity().lock()->GetComponent("EC_OgreEnvironment").get());
    env.UpdateVisualEffects(frametime);
}

bool Environment::IsCaelum()
{
    FindActiveEnvironment();
    OgreRenderer::EC_OgreEnvironment &env = *checked_static_cast<OgreRenderer::EC_OgreEnvironment*>
        (GetEnvironmentEntity().lock()->GetComponent("EC_OgreEnvironment").get());
    return env.IsCaleumUsed();
}

}

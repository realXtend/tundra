/// @file Environment.cpp
/// @brief Manages environment-related reX-logic, e.g. world time and lightning.
/// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "RexLogicModule.h"
#include "Environment.h"
#include "Foundation.h"
#include "EC_OgreEnvironment.h"
#include "SceneManager.h"

namespace RexLogic
{

Environment::Environment(RexLogicModule *owner)
:owner_(owner)
{
}

Environment::~Environment()
{
}

void Environment::FindCurrentlyActiveEnvironment()
{
    Scene::ScenePtr scene = owner_->GetCurrentActiveScene();
    for(Scene::SceneManager::iterator iter = scene->begin();
        iter != scene->end(); ++iter)
    {
        Scene::Entity &entity = **iter;
        Foundation::ComponentInterfacePtr envComponent = entity.GetComponent("EC_OgreEnvironment");
        if (envComponent.get())
            cachedEnvironmentEntity_ = scene->GetEntity(entity.GetId());
    }
}

Scene::EntityWeakPtr Environment::GetEnvironmentEntity()
{
    return cachedEnvironmentEntity_;
}

void Environment::CreateEnvironment()
{
    Scene::ScenePtr active_scene = owner_->GetCurrentActiveScene();
    Scene::EntityPtr entity = active_scene->CreateEntity(active_scene->GetNextFreeId());
    entity->AddEntityComponent(owner_->GetFramework()->GetComponentManager()->CreateComponent("EC_OgreEnvironment"));

    cachedEnvironmentEntity_ = entity;
}

bool Environment::HandleOSNE_SimulatorViewerTimeMessage(OpenSimProtocol::NetworkEventInboundData *data)
{
    NetInMessage &msg = *data->message;
    msg.ResetReading();

    usecSinceStart_ = (time_t)msg.ReadU64();
    secPerDay_ = msg.ReadU32(); // seems to be zero, at least with 0.4 server
    secPerYear_ = msg.ReadU32(); // seems to be zero, at least with 0.4 server
    sunDirection_ = msg.ReadVector3();
    sunPhase_ = msg.ReadF32(); // seems to be zero, at least with 0.4 server
    sunAngVelocity_ = msg.ReadVector3();

    FindCurrentlyActiveEnvironment();
    Foundation::ComponentPtr component = GetEnvironmentEntity().lock()->GetComponent("EC_OgreEnvironment");
    if (!component)
        return false;
    
    // Update the sunlight direction and angle velocity.
    OgreRenderer::EC_OgreEnvironment &env = *checked_static_cast<OgreRenderer::EC_OgreEnvironment*>
        (component.get());
        
    //env.SetSunDirection(-sunDirection_);
       
    return false; 
}

void Environment::UpdateVisualEffects()
{
    FindCurrentlyActiveEnvironment();
    OgreRenderer::EC_OgreEnvironment &env = *checked_static_cast<OgreRenderer::EC_OgreEnvironment*>
        (GetEnvironmentEntity().lock()->GetComponent("EC_OgreEnvironment").get());
    env.UpdateVisualEffects();
}

}

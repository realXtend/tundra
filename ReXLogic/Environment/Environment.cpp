/// @file Environment.cpp
/// @brief Manages environment-related reX-logic, e.g. world time and lightning.
/// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "RexLogicModule.h"
#include "Environment/Environment.h"
#include "Foundation.h"
#include "EC_OgreEnvironment.h"
#include "SceneManager.h"

namespace RexLogic
{

Environment::Environment(RexLogicModule *owner) : owner_(owner)
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

///\todo Remove this when Caelum is working ok.
bool test = true;

bool Environment::HandleOSNE_SimulatorViewerTimeMessage(ProtocolUtilities::NetworkEventInboundData *data)
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

    FindCurrentlyActiveEnvironment();
    Foundation::ComponentPtr component = GetEnvironmentEntity().lock()->GetComponent("EC_OgreEnvironment");
    if (!component)
        return false;

    // Update the sunlight direction and angle velocity.
    ///\note Not needed anymore as we use Caleum now.
    OgreRenderer::EC_OgreEnvironment &env = *checked_static_cast<OgreRenderer::EC_OgreEnvironment*>
        (component.get());
//    env.SetSunDirection(-sunDirection_);

    /** \note
     *  It's not necessary to update the environment time every time SimulatorViewerTimeMessage is received
     *  (about every tenth second that is) because the Caleum system has its own perception of time. But let's
     *  do it anyways for now.
     */
    if (test)
    {
        env.SetTime(usecSinceStart_);
        test = false;
    }

    return false;
}

void Environment::UpdateVisualEffects(Core::f64 frametime)
{
    FindCurrentlyActiveEnvironment();
    OgreRenderer::EC_OgreEnvironment &env = *checked_static_cast<OgreRenderer::EC_OgreEnvironment*>
        (GetEnvironmentEntity().lock()->GetComponent("EC_OgreEnvironment").get());
    env.UpdateVisualEffects(frametime);
}

bool Environment::UseCaelum()
{
    FindCurrentlyActiveEnvironment();
    OgreRenderer::EC_OgreEnvironment &env = *checked_static_cast<OgreRenderer::EC_OgreEnvironment*>
        (GetEnvironmentEntity().lock()->GetComponent("EC_OgreEnvironment").get());
    return env.IsCaleumUsed();
}

}

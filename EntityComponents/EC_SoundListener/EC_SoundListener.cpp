/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_SoundListener.cpp
 *  @brief  Entity-component which provides sound listener position for in-world 3D audio.
 *          Updates parent entity's placeable component's position to the sound service each frame.
 *  @note   Only one entity can have active sound listener at a time.
 */

#include "StableHeaders.h"
#include "EC_SoundListener.h"
#include "ModuleInterface.h"
#include "Entity.h"
#include "EC_OgrePlaceable.h"
#include "SceneManager.h"
#include "SoundServiceInterface.h"
#include "LoggingFunctions.h"

DEFINE_POCO_LOGGING_FUNCTIONS("EC_SoundListener")

EC_SoundListener::EC_SoundListener(Foundation::ModuleInterface *module):
    Foundation::ComponentInterface(module->GetFramework()),
    active_(false)
{
    soundService_ = GetFramework()->GetServiceManager()->GetService<Foundation::SoundServiceInterface>();

    connect(this, SIGNAL(ParentEntitySet()), SLOT(RetrievePlaceable()));
    connect(GetFramework(), SIGNAL(FrameProcessed(double)), SLOT(Update()));
}

EC_SoundListener::~EC_SoundListener()
{
}

void EC_SoundListener::RetrievePlaceable()
{
    if (!GetParentEntity())
        LogError("Couldn't find an parent entity for EC_SoundListener. Cannot retrieve placeable component.");

    placeable_ = GetParentEntity()->GetComponent<OgreRenderer::EC_OgrePlaceable>();
    if (!placeable_.lock())
        LogError("Couldn't find an EC_OgrePlaceable component from the parent entity.");
}

void EC_SoundListener::SetActive(bool active)
{
    active_ = active;

    Scene::ScenePtr scene = GetFramework()->GetDefaultWorldScene();
    if (!scene)
        return;

    if (active_)
    {
        // Disable all the other listeners, only one can be active at a time.
        ///\todo Maybe not the most sophisticated way to handle this here; do this in RexLogicModule maybe.
        Scene::EntityList listeners = scene->GetEntitiesWithComponent("EC_SoundListener");
        foreach(Scene::EntityPtr listener, listeners)
        {
            EC_SoundListener *ec = listener->GetComponent<EC_SoundListener>().get();
            if (ec != this)
                listener->GetComponent<EC_SoundListener>()->SetActive(!active_);
        }
    }
}

void EC_SoundListener::Update()
{
    if (active_ && !placeable_.expired() && !soundService_.expired())
        soundService_.lock()->SetListener(placeable_.lock()->GetPosition(), placeable_.lock()->GetOrientation());
}


/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_SoundListener.h
 *  @brief  
 *  @note   
 */

#include "StableHeaders.h"
#include "EC_SoundListener.h"
#include "ModuleInterface.h"
#include "Entity.h"
#include "EC_OgrePlaceable.h"
#include "SceneManager.h"

#include "LoggingFunctions.h"

DEFINE_POCO_LOGGING_FUNCTIONS("EC_SoundListener")

EC_SoundListener::EC_SoundListener(Foundation::ModuleInterface *module):
    Foundation::ComponentInterface(module->GetFramework()),
    active(this, "active", false)
{
    connect(this, SIGNAL(ParentEntitySet()), SLOT(RetrievePlaceable()));
    connect(this, SIGNAL(OnChanged()), SLOT(DisableOtherSoundListeners()));
}

EC_SoundListener::~EC_SoundListener()
{
    placeable_.reset();
}

void EC_SoundListener::RetrievePlaceable()
{
    if (!GetParentEntity())
        LogError("Couldn't find an parent entity for EC_SoundListener. Cannot retrieve placeable component.");

    placeable_ = GetParentEntity()->GetComponent<OgreRenderer::EC_OgrePlaceable>();
    if (!placeable_)
        LogError("Couldn't find an EC_OgrePlaceable component from the parent entity.");
}

void EC_SoundListener::DisableOtherSoundListeners()
{
    Scene::ScenePtr scene = GetFramework()->GetDefaultWorldScene();
    if (!scene)
        return;

    bool thisActive = active.Get();
    if (thisActive)
    {
        // Disable all the other listeners, only one can be active at a time.
        ///\todo Not the most sophisticated way, come up with a better one.
        Scene::EntityList listeners = scene->GetEntitiesWithComponent("EC_SoundListener");
        foreach(Scene::EntityPtr listener, listeners)
        {
            EC_SoundListener *ec = listener->GetComponent<EC_SoundListener>().get();
            if (ec != this)
                listener->GetComponent<EC_SoundListener>()->active.Set(!thisActive, AttributeChange::LocalOnly);
        }
    }
}


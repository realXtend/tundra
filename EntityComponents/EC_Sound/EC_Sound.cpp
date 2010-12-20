// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "EC_Sound.h"
#include "IModule.h"
#include "Framework.h"
#include "Entity.h"
#include "Audio.h"
#include "AssetAPI.h"
#include "EC_Placeable.h"
#include "EC_SoundListener.h"
#include "SceneManager.h"

#include "LoggingFunctions.h"
DEFINE_POCO_LOGGING_FUNCTIONS("EC_Sound")

#include "MemoryLeakCheck.h"

EC_Sound::EC_Sound(IModule *module):
    IComponent(module->GetFramework()),
    soundRef(this, "Sound ref"),
    soundInnerRadius(this, "Sound radius inner", 0.0f),
    soundOuterRadius(this, "Sound radius outer", 20.0f),
    loopSound(this, "Loop sound", false),
    soundGain(this, "Sound gain", 1.0f),
    spatial(this, "Spatial", true)
{
    static AttributeMetadata metaData("", "0", "1", "0.1");
    soundGain.SetMetadata(&metaData);

    connect(this, SIGNAL(ParentEntitySet()), SLOT(UpdateSignals()));
    connect(this, SIGNAL(OnAttributeChanged(IAttribute*, AttributeChange::Type)), SLOT(AttributeUpdated(IAttribute*)));
}

EC_Sound::~EC_Sound()
{
    StopSound();
}

void EC_Sound::AttributeUpdated(IAttribute *attribute)
{
    if (attribute == &soundRef)
        framework_->Asset()->RequestAsset(soundRef.Get().ref);

    UpdateSoundSettings();
}

void EC_Sound::RegisterActions()
{
    Scene::Entity *entity = GetParentEntity();
    assert(entity);
    if (entity)
    {
        entity->ConnectAction("PlaySound", this, SLOT(PlaySound()));
        entity->ConnectAction("StopSound", this, SLOT(StopSound()));
    }
}

void EC_Sound::PositionChange(const QVector3D &pos)
{
    if (soundChannel)
        soundChannel->SetPosition(Vector3df(pos.x(), pos.y(), pos.z()));
}

void EC_Sound::PlaySound()
{
    ComponentChanged(AttributeChange::LocalOnly);

    // If previous sound is still playing stop it before we apply a new sound.
    if (soundChannel)
    {
        soundChannel->Stop();
        soundChannel.reset();
    }

    AssetPtr audioAsset = GetFramework()->Asset()->GetAsset(soundRef.Get().ref);
    if (!audioAsset)
    {
        ///\todo Make a request.
        return;
    }

    bool soundListenerExists = true;
    EC_Placeable *placeable = dynamic_cast<EC_Placeable *>(FindPlaceable().get());

    // If we are going to play back positional audio, check that there is a sound listener enabled that can listen to it.
    // Otherwise, if no SoundListener exists, play back the audio as nonpositional.
    if (placeable && spatial.Get())
        soundListenerExists = (GetActiveSoundListener() != Scene::EntityPtr());

    if (placeable && spatial.Get() && soundListenerExists)
    {
        soundChannel = GetFramework()->Audio()->PlaySound3D(placeable->GetPosition(), audioAsset, SoundChannel::Triggered);
        if (soundChannel)
            soundChannel->SetRange(soundInnerRadius.Get(), soundOuterRadius.Get(), 2.0f);
    }
    else // Play back sound as a nonpositional sound, if no EC_Placeable was found or if spatial was not set.
    {
        soundChannel = GetFramework()->Audio()->PlaySound(audioAsset, SoundChannel::Ambient);
    }

    if (soundChannel)
    {
        soundChannel->SetGain(soundGain.Get());
        soundChannel->SetLooped(loopSound.Get());
    }
}

void EC_Sound::StopSound()
{
    if (soundChannel)
        soundChannel->Stop();
    soundChannel.reset();
}

void EC_Sound::UpdateSoundSettings()
{
    if (soundChannel)
    {
        soundChannel->SetGain(soundGain.Get());
        soundChannel->SetLooped(loopSound.Get());
        soundChannel->SetRange(soundInnerRadius.Get(), soundOuterRadius.Get(), 2.0f);
    }
}

Scene::EntityPtr EC_Sound::GetActiveSoundListener()
{
#ifdef _DEBUG
    int numActiveListeners = 0; // For debugging, count how many listeners are active.
#endif
    
    Scene::EntityList listeners = parent_entity_->GetScene()->GetEntitiesWithComponent("EC_SoundListener");
    foreach(Scene::EntityPtr listener, listeners)
    {
        EC_SoundListener *ec = listener->GetComponent<EC_SoundListener>().get();
        if (ec->active.Get())
        {
#ifndef _DEBUG
            assert(ec->GetParentEntity());
            return ec->GetParentEntity()->shared_from_this();
#else
            ++numActiveListeners;
#endif
        }
    }

#ifdef _DEBUG
    if (numActiveListeners != 1)
        LogWarning("Warning: When playing back positional 3D audio, " + QString::number(numActiveListeners).toStdString() + " active sound listeners were found!");
#endif
    return Scene::EntityPtr();
}

void EC_Sound::UpdateSignals()
{
    if (!GetParentEntity())
    {
        LogError("Couldn't update singals cause component dont have parent entity set.");
        return;
    }
    Scene::SceneManager *scene = GetParentEntity()->GetScene();
    if(!scene)
    {
        LogError("Fail to update signals cause parent entity's scene is null.");
        return;
    }

    RegisterActions();
}

ComponentPtr EC_Sound::FindPlaceable() const
{
    assert(framework_);
    ComponentPtr comp;
    if(!GetParentEntity())
    {
        LogError("Fail to find a placeable component cause parent entity is null.");
        return comp;
    }
    comp = GetParentEntity()->GetComponent<EC_Placeable>();
    //We need to update sound source position when placeable component has changed it's transformation.
    connect(comp.get(), SIGNAL(PositionChanged(const QVector3D &)),
            SLOT(PositionChange(const QVector3D &)), Qt::UniqueConnection);
    return comp;
}

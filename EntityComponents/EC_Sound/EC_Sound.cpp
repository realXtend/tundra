// For conditions of distribution and use, see copyright notice in license.txt

#include "DebugOperatorNew.h"
#include "EC_Sound.h"
#include "IModule.h"
#include "Framework.h"

#include "Entity.h"
#include "SceneManager.h"

#include "AudioAPI.h"
#include "AudioAsset.h"

#include "AssetAPI.h"
#include "IAsset.h"
#include "IAssetTransfer.h"

#include "EC_Placeable.h"
#include "EC_SoundListener.h"

#include <QStringList>

#include "LoggingFunctions.h"

#include "MemoryLeakCheck.h"

EC_Sound::EC_Sound(IModule *module):
    IComponent(module->GetFramework()),
    soundRef(this, "Sound ref"),
    soundInnerRadius(this, "Sound radius inner", 0.0f),
    soundOuterRadius(this, "Sound radius outer", 20.0f),
    playOnLoad(this, "Play on load", false),
    loopSound(this, "Loop sound", false),
    soundGain(this, "Sound gain", 1.0f),
    spatial(this, "Spatial", true)
{
    static AttributeMetadata metaData("", "0", "1", "0.1");
    soundGain.SetMetadata(&metaData);
    static AttributeMetadata soundRefMetadata;
    AttributeMetadata::ButtonInfoList soundRefButtons;
    soundRefButtons.push_back(AttributeMetadata::ButtonInfo(soundRef.GetName(), "V", "View"));
    soundRefMetadata.buttons = soundRefButtons;
    soundRef.SetMetadata(&soundRefMetadata);

    connect(this, SIGNAL(ParentEntitySet()), SLOT(UpdateSignals()));
    connect(this, SIGNAL(AttributeChanged(IAttribute*, AttributeChange::Type)), SLOT(OnAttributeUpdated(IAttribute*)));
}

EC_Sound::~EC_Sound()
{
    StopSound();
}

void EC_Sound::OnAttributeUpdated(IAttribute *attribute)
{
    if (framework_->IsHeadless())
        return;

    if (attribute == &soundRef)
    {
        AssetTransferPtr tranfer =  framework_->Asset()->RequestAsset(soundRef.Get().ref);
        if (tranfer.get())
            connect(tranfer.get(), SIGNAL(Loaded(AssetPtr)), this, SLOT(AudioAssetLoaded(AssetPtr)), Qt::UniqueConnection);
    }
    else if (attribute == &playOnLoad)
    {
        /// \todo check sound channels audio asset if its different, then play the new one
        if (getplayOnLoad())
        {
            if (soundRef.Get().ref.isEmpty())
                return;
            AssetPtr audioAsset = GetFramework()->Asset()->GetAsset(soundRef.Get().ref);
            if (audioAsset.get())
            {
                // Channel not created yet
                if (!soundChannel.get())
                    PlaySound();
                // Channel created, check if stopped
                else if (soundChannel->GetState() == SoundChannel::Stopped)
                    PlaySound();
            }
        }
    }

    UpdateSoundSettings();
}

void EC_Sound::AudioAssetLoaded(AssetPtr asset)
{
    if (framework_->IsHeadless())
        return;

    if (!asset.get())
        LogError("AudioAssetLoaded: Audio asset ptr null, cannot continue!");

    AudioAsset *audioAsset = dynamic_cast<AudioAsset*>(asset.get());
    if (audioAsset)
    {
        /// \todo check sound channels audio asset if its different, then play the new one
        if (getplayOnLoad())
        {
            // Channel not created yet
            if (!soundChannel.get())
                PlaySound();
            // Channel created, check if stopped
            else if (soundChannel->GetState() == SoundChannel::Stopped)
                PlaySound();
        }
    }
    else
        LogError("Audio asset was loaded but not type 'AudioAsset'.");
}

void EC_Sound::RegisterActions()
{
    if (framework_->IsHeadless())
        return;

    Scene::Entity *entity = GetParentEntity();
    if (entity)
    {
        entity->ConnectAction("PlaySound", this, SLOT(PlaySound()));
        entity->ConnectAction("StopSound", this, SLOT(StopSound()));
    }
}

void EC_Sound::View(const QString &attributeName)
{
    LogWarning("View(const QString &attributeName) not implemented yet!");
}

void EC_Sound::PlaySound()
{
    if (framework_->IsHeadless())
        return;

    // If previous sound is still playing stop it before we apply a new sound.
    if (soundChannel)
    {
        soundChannel->Stop();
        soundChannel.reset();
    }

    if (soundRef.Get().ref.isEmpty())
        return;

    AssetPtr audioAsset = GetFramework()->Asset()->GetAsset(soundRef.Get().ref);
    if (!audioAsset)
    {
        LogWarning("PlaySound called before audio asset was loaded.");
        return;
    }

    bool soundListenerExists = true;
    EC_Placeable *placeable = dynamic_cast<EC_Placeable *>(FindPlaceable().get());

    // If we are going to play back positional audio, check that there is a sound listener enabled that can listen to it.
    // Otherwise, if no SoundListener exists, play back the audio as nonpositional.
    if (placeable && spatial.Get())
        soundListenerExists = (GetActiveSoundListener() != EntityPtr());

    if (placeable && spatial.Get() && soundListenerExists)
    {
        soundChannel = GetFramework()->Audio()->PlaySound3D(placeable->transform.Get().position, audioAsset, SoundChannel::Triggered);
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

EntityPtr EC_Sound::GetActiveSoundListener()
{
#ifdef _DEBUG
    int numActiveListeners = 0; // For debugging, count how many listeners are active.
#endif
    
    EntityList listeners = parent_entity_->GetScene()->GetEntitiesWithComponent("EC_SoundListener");
    foreach(EntityPtr listener, listeners)
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
    return EntityPtr();
}

void EC_Sound::UpdateSignals()
{
    if (!GetParentEntity())
    {
        LogError("Couldn't update signals cause component dont have parent entity set.");
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
    if (comp)
    {
        connect(comp.get(), SIGNAL(AttributeChanged(IAttribute*, AttributeChange::Type)), this, SLOT(PlaceableUpdated(IAttribute*)));
    }
    return comp;
}

void EC_Sound::PlaceableUpdated(IAttribute* attribute)
{
    if (framework_->IsHeadless())
        return;
    
    EC_Placeable* placeable = checked_static_cast<EC_Placeable*>(sender());
    if ((attribute == &placeable->transform) && (soundChannel))
    {
        const Transform& trans = placeable->transform.Get();
        soundChannel->SetPosition(trans.position);
    }
}

// For conditions of distribution and use, see copyright notice in LICENSE

#include "DebugOperatorNew.h"

#include "EC_Sound.h"

#include "Framework.h"
#include "Entity.h"
#include "Scene/Scene.h"
#include "AttributeMetadata.h"
#include "AudioAPI.h"
#include "AudioAsset.h"
#include "AssetAPI.h"
#include "FrameAPI.h"
#include "IAsset.h"
#include "IAssetTransfer.h"
#include "EC_Placeable.h"
#include "EC_SoundListener.h"
#include "LoggingFunctions.h"
#include "Profiler.h"

#include "MemoryLeakCheck.h"

// Win32 API resolves PlaySound to PlaySoundA or PlaySoundW, which isn't used here, so remove it.
#ifdef PlaySound
#undef PlaySound
#endif

EC_Sound::EC_Sound(Scene* scene):
    IComponent(scene),
    soundRef(this, "Sound ref", AssetReference("", "Audio")),
    soundInnerRadius(this, "Sound radius inner", 0.0f),
    soundOuterRadius(this, "Sound radius outer", 20.0f),
    playOnLoad(this, "Play on load", false),
    loopSound(this, "Loop sound", false),
    soundGain(this, "Sound gain", 1.0f),
    spatial(this, "Spatial", true)
{
    static AttributeMetadata metaData("", "0", "1", "0.1");
    soundGain.SetMetadata(&metaData);

    connect(this, SIGNAL(ParentEntitySet()), SLOT(UpdateSignals()));
}

EC_Sound::~EC_Sound()
{
    StopSound();
}

void EC_Sound::AttributesChanged()
{
    if (framework->IsHeadless())
        return;

    if (soundRef.ValueChanged())
    {
        AssetTransferPtr tranfer =  framework->Asset()->RequestAsset(soundRef.Get().ref);
        if (tranfer.get())
            connect(tranfer.get(), SIGNAL(Succeeded(AssetPtr)), this, SLOT(AudioAssetLoaded(AssetPtr)), Qt::UniqueConnection);
    }
    if (playOnLoad.ValueChanged())
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
                else if (soundChannel->State() == SoundChannel::Stopped)
                    PlaySound();
            }
        }
    }

    UpdateSoundSettings();
}

void EC_Sound::AudioAssetLoaded(AssetPtr asset)
{
    if (framework->IsHeadless())
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
            else if (soundChannel->State() == SoundChannel::Stopped)
                PlaySound();
        }
    }
    else
        LogError("Audio asset was loaded but not type 'AudioAsset'.");
}

void EC_Sound::RegisterActions()
{
    if (framework->IsHeadless())
        return;

    Entity *entity = ParentEntity();
    if (entity)
    {
        entity->ConnectAction("PlaySound", this, SLOT(PlaySound()));
        entity->ConnectAction("StopSound", this, SLOT(StopSound()));
    }
}

void EC_Sound::PlaySound()
{
    if (framework->IsHeadless())
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

    EC_Placeable *placeable = dynamic_cast<EC_Placeable *>(FindPlaceable().get());

    if (spatial.Get() && !placeable)
        LogWarning("Warning: Cannot play back EC_Sound as spatial, since the same Entity does not contain a EC_Placeable component to specify a position for the sound source! Add a EC_Placeable to the same Entity with the EC_Sound to fix this. Playing the sound as non-spatial..");

    if (placeable && spatial.Get())
    {
        soundChannel = GetFramework()->Audio()->PlaySound3D(placeable->WorldPosition(), audioAsset, SoundChannel::Triggered);
        if (soundChannel)
            soundChannel->SetRange(soundInnerRadius.Get(), soundOuterRadius.Get(), 2.0f);
        
        // If placeable has a parent, start polling the sound position constantly
        if (!placeable->parentRef.Get().IsEmpty())
            connect(framework->Frame(), SIGNAL(Updated(float)), this, SLOT(ConstantPositionUpdate()), Qt::UniqueConnection);
        else
            disconnect(framework->Frame(), SIGNAL(Updated(float)), this, SLOT(ConstantPositionUpdate()));
    }
    else // Play back sound as a nonpositional sound, if no EC_Placeable was found or if spatial was not set.
    {
        soundChannel = GetFramework()->Audio()->PlaySound(audioAsset, SoundChannel::Ambient);
        disconnect(framework->Frame(), SIGNAL(Updated(float)), this, SLOT(ConstantPositionUpdate()));
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
    
    EntityPtr activeSoundListener;
    EntityList listeners = parentEntity->ParentScene()->GetEntitiesWithComponent("EC_SoundListener");
    foreach(EntityPtr listener, listeners)
    {
        EC_SoundListener *ec = listener->GetComponent<EC_SoundListener>().get();
        if (ec->active.Get())
        {
#ifndef _DEBUG
            assert(ec->ParentEntity());
            return ec->ParentEntity()->shared_from_this();
#else
            ++numActiveListeners;
            activeSoundListener = ec->ParentEntity()->shared_from_this();
            assert(activeSoundListener.get());
#endif
        }
    }

#ifdef _DEBUG
    if (numActiveListeners != 1)
        LogWarning("Warning: When playing back positional 3D audio, " + QString::number(numActiveListeners) + " active sound listeners were found!");
#endif
    return activeSoundListener;
}

void EC_Sound::UpdateSignals()
{
    if (!ParentEntity())
    {
        LogError("Couldn't update signals cause component dont have parent entity set.");
        return;
    }
    Scene *scene = ParentEntity()->ParentScene();
    if(!scene)
    {
        LogError("Fail to update signals cause parent entity's scene is null.");
        return;
    }

    RegisterActions();
}

ComponentPtr EC_Sound::FindPlaceable() const
{
    assert(framework);
    ComponentPtr comp;
    if(!ParentEntity())
    {
        LogError("Fail to find a placeable component cause parent entity is null.");
        return comp;
    }
    comp = ParentEntity()->GetComponent<EC_Placeable>();
    //We need to update sound source position when placeable component has changed it's transformation.
    if (comp)
    {
        connect(comp.get(), SIGNAL(AttributeChanged(IAttribute*, AttributeChange::Type)), this, SLOT(PlaceableUpdated(IAttribute*)), Qt::UniqueConnection);
    }
    return comp;
}

void EC_Sound::PlaceableUpdated(IAttribute* attribute)
{
    if ((framework->IsHeadless()) || (!soundChannel))
        return;
    
    EC_Placeable* placeable = checked_static_cast<EC_Placeable*>(sender());
    if ((attribute == &placeable->transform) && (soundChannel))
        soundChannel->SetPosition(placeable->WorldPosition());
    
    if (attribute == &placeable->parentRef)
    {
        // If placeable has a parent, start polling the sound position constantly
        if (!placeable->parentRef.Get().IsEmpty())
            connect(framework->Frame(), SIGNAL(Updated(float)), this, SLOT(ConstantPositionUpdate()), Qt::UniqueConnection);
    }
}

void EC_Sound::ConstantPositionUpdate()
{
    PROFILE(EC_Sound_ConstantPositionUpdate);
    if (!ParentEntity())
    {
        disconnect(framework->Frame(), SIGNAL(Updated(float)), this, SLOT(ConstantPositionUpdate()));
        return;
    }
    EC_Placeable* placeable = ParentEntity()->GetComponent<EC_Placeable>().get();
    if ((!placeable) || (!soundChannel) || (!spatial.Get()))
    {
        disconnect(framework->Frame(), SIGNAL(Updated(float)), this, SLOT(ConstantPositionUpdate()));
        return;
    }
    
    float3 pos = placeable->WorldPosition();
    
    soundChannel->SetPosition(placeable->WorldPosition());
}

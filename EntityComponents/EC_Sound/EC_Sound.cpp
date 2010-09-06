#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "EC_Sound.h"
#include "ModuleInterface.h"
#include "Framework.h"
#include "Entity.h"
#include "EC_OgrePlaceable.h"
#include "SceneManager.h"
#include "SoundServiceInterface.h"

#include "LoggingFunctions.h"
DEFINE_POCO_LOGGING_FUNCTIONS("EC_Sound")

#include "MemoryLeakCheck.h"

EC_Sound::EC_Sound(Foundation::ModuleInterface *module):
    Foundation::ComponentInterface(module->GetFramework()),
    sound_id_(0),
    soundId_(this, "Sound id"),
    soundInnerRadius_(this, "sound radius inner", 0.0f),
    soundOuterRadius_(this, "Sound radius outer", 20.0f),
    loopSound_(this, "Loop sound", false),
    triggerSound_(this, "Trigger sound", false),
    soundGain_(this, "Sound gain", 1.0f)
{
    static Foundation::AttributeMetadata metaData;
    static bool metadataInitialized = false;
    if(!metadataInitialized)
    {
        metaData.min = "0";
        metaData.max = "1";
        metaData.step = "0.1";
        metadataInitialized = true;
    }
    soundGain_.SetMetadata(&metaData);

    QObject::connect(this, SIGNAL(ParentEntitySet()), this, SLOT(UpdateSignals()));
}

EC_Sound::~EC_Sound()
{
    StopSound();
}

void EC_Sound::AttributeUpdated(Foundation::ComponentInterface *component, Foundation::AttributeInterface *attribute)
{
    if(component != this)
        return;

    if(attribute->GetNameString() == soundId_.GetNameString())
    {
        Foundation::SoundServiceInterface *soundService = framework_->GetService<Foundation::SoundServiceInterface>();
        if(soundService && soundService->GetSoundName(sound_id_) != soundId_.Get().toStdString())
            StopSound();
    }
    else if(attribute->GetNameString() == triggerSound_.GetNameString())
    {
        // Play sound if sound asset id has been setted and if sound has been triggered or looped.
        if(triggerSound_.Get() == true && (!soundId_.Get().isNull() || loopSound_.Get()))
            PlaySound();
    }
    UpdateSoundSettings();
}

void EC_Sound::PlaySound()
{
    triggerSound_.Set(false, AttributeChange::LocalOnly);
    ComponentChanged(AttributeChange::LocalOnly);

    Foundation::SoundServiceInterface *soundService = framework_->GetService<Foundation::SoundServiceInterface>();
    if(!soundService)
        return;

    if(sound_id_)
        StopSound();

    OgreRenderer::EC_OgrePlaceable *placeable = dynamic_cast<OgreRenderer::EC_OgrePlaceable *>(FindPlaceable().get());
    if(placeable)
    {
        sound_id_ = soundService->PlaySound3D(soundId_.Get().toStdString(), Foundation::SoundServiceInterface::Triggered, false, placeable->GetPosition());
        soundService->SetGain(sound_id_, soundGain_.Get());
        soundService->SetLooped(sound_id_, loopSound_.Get());
        soundService->SetRange(sound_id_, soundInnerRadius_.Get(), soundOuterRadius_.Get(), 2.0f);
    }
    else // If entity isn't holding placeable component treat sound as ambient sound.
    {
        sound_id_ = soundService->PlaySound(soundId_.Get().toStdString(), Foundation::SoundServiceInterface::Ambient);
        soundService->SetGain(sound_id_, soundGain_.Get());
    }
}

void EC_Sound::StopSound()
{
    Foundation::SoundServiceInterface *soundService = framework_->GetService<Foundation::SoundServiceInterface>();
    if(!soundService)
        return;

    soundService->StopSound(sound_id_);
    sound_id_ = 0;
}

void EC_Sound::UpdateSoundSettings()
{
    Foundation::SoundServiceInterface *soundService = framework_->GetService<Foundation::SoundServiceInterface>();
    if(!soundService || !sound_id_)
        return;

    soundService->SetGain(sound_id_, soundGain_.Get());
    soundService->SetLooped(sound_id_, loopSound_.Get());
    soundService->SetRange(sound_id_, soundInnerRadius_.Get(), soundOuterRadius_.Get(), 2.0f);
}

void EC_Sound::UpdateSignals()
{
    disconnect(this, SLOT(AttributeUpdated(Foundation::ComponentInterface *, Foundation::AttributeInterface *)));
    if(GetParentEntity())
    {
        Scene::SceneManager *scene = GetParentEntity()->GetScene();
        if(scene)
        connect(scene, SIGNAL(AttributeChanged(Foundation::ComponentInterface*, Foundation::AttributeInterface*, AttributeChange::Type)),
                this, SLOT(AttributeUpdated(Foundation::ComponentInterface*, Foundation::AttributeInterface*))); 
    }
}

Foundation::ComponentPtr EC_Sound::FindPlaceable() const
{
    assert(framework_);
    Foundation::ComponentPtr comp;
    if(!GetParentEntity())
        return comp;
    comp = GetParentEntity()->GetComponent<OgreRenderer::EC_OgrePlaceable>();
    return comp;
}
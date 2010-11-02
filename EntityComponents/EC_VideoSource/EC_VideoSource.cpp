// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "EC_VideoSource.h"

#include "EC_Mesh.h"
#include "EC_OgreCustomObject.h"
#include "IModule.h"
#include "Framework.h"
#include "Entity.h"
#include "EC_Placeable.h"
#include "SceneManager.h"
#include "MediaPlayerService.h"

#include "LoggingFunctions.h"
DEFINE_POCO_LOGGING_FUNCTIONS("EC_VideoSource")

#include "MemoryLeakCheck.h"

#define REFRESH_RATE (15)

EC_VideoSource::EC_VideoSource(IModule *module):
    IComponent(module->GetFramework()),
    videoSourceUrl(this, "Video source url"),
    submeshIndex(this, "Submesh index", 0),
    playbackState(this, "Playback state", PS_Stop),
    triggerVideo(this, "Trigger Video", false),
    audioPlaybackVolume(this, "Audio playback volume", 1.0),
    looping(this, "Looping", false),
    startup_checker_(false)
{
    static AttributeMetadata psAttrData;
    static bool metadataInitialized = false;
    if(!metadataInitialized)
    {
        psAttrData.enums[PS_Play] = "Play";
        psAttrData.enums[PS_Stop] = "Stop";
        psAttrData.enums[PS_Pause] = "Pause";
        metadataInitialized = true;
    }
    playbackState.SetMetadata(&psAttrData);

    static AttributeMetadata volumeMetaData("", "0", "1", "0.1");
    audioPlaybackVolume.SetMetadata(&volumeMetaData);

    player_ = new Phonon::VideoPlayer(Phonon::VideoCategory);
    connect(player_, SIGNAL(finished()), this, SLOT(PlaybackFinished()));
    connect(this, SIGNAL(ParentEntitySet()), this, SLOT(UpdateSignals()));
    connect(this, SIGNAL(OnAttributeChanged(IAttribute*, AttributeChange::Type)),
            this, SLOT(AttributeUpdated(IAttribute*)));
}

EC_VideoSource::~EC_VideoSource()
{
    Stop();
    delete player_;
}

void EC_VideoSource::AttributeUpdated(IAttribute *attribute)
{
    UpdateCanvas();
    
    if(!startup_checker_)
    {
        if ((!videoSourceUrl.Get().isNull() && triggerVideo.Get()) && looping.Get())
        {
            startup_checker_ = true;  
            Play();
        }        
    }
    
    if(attribute->GetNameString() == triggerVideo.GetNameString())
    {
        // Play video if video source url has been setted and if sound has been triggered or looped.
        if(triggerVideo.Get() == true && (!videoSourceUrl.Get().isNull() || looping.Get()))
        {
            
            if(playbackState.Get() != PS_Play)
            {
                LogDebug("Play Video");
                Play();
            }
        }
        if(triggerVideo.Get() == false && (!videoSourceUrl.Get().isNull() || looping.Get()))
        {
            if(playbackState.Get() != PS_Stop)
            {
                LogDebug("Stop Video");
                Stop();
            }
        }
    }
}

void EC_VideoSource::RegisterActions()
{
    Scene::Entity *entity = GetParentEntity();
    assert(entity);
    if (entity)
    {
        entity->ConnectAction("Play", this, SLOT(Play()));
        entity->ConnectAction("Stop", this, SLOT(Stop()));
        entity->ConnectAction("Pause", this, SLOT(Pause()));
    }
}

void EC_VideoSource::Play()
{
    playbackState.Set(PS_Play, AttributeChange::Replicate);
    player_->play(Phonon::MediaSource(videoSourceUrl.Get()));
}

void EC_VideoSource::Stop()
{
    playbackState.Set(PS_Stop, AttributeChange::Replicate);
    player_->stop();
}

void EC_VideoSource::Pause()
{
    player_->pause();
}

void EC_VideoSource::UpdateSignals()
{
    if (!GetParentEntity())
    {
        LogError("Couldn't update singals cause component dont have parent entity setted.");
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

void EC_VideoSource::UpdateCanvas()
{
    Scene::Entity* entity = GetParentEntity();
    if (!entity)
    {
        LogError("No parent entity, cannot create/update 3DCanvas");
        return;
    }

    ComponentPtr comp = entity->GetOrCreateComponent(EC_3DCanvas::TypeNameStatic());
    if (!comp)
    {
        LogError("Could not create/get 3DCanvas component");
        return;
    }

    // If entity has no valid mesh or prim yet, start a retry timer and try to set the canvas later
    if ((!entity->GetComponent(EC_Mesh::TypeNameStatic())) && 
        (!entity->GetComponent(EC_OgreCustomObject::TypeNameStatic())))
    {
        LogError("Mesh or prim did not exist yet, retrying");
    }

    canvas_ = checked_static_cast<EC_3DCanvas*>(comp.get());
    
    canvas_->SetWidget(player_->videoWidget());
    int submesh = getsubmeshIndex();
    if (submesh < 0)
        submesh = 0;
    canvas_->SetSubmesh(submesh);
    canvas_->SetRefreshRate(REFRESH_RATE);
    canvas_->Start();   
}

void EC_VideoSource::PlaybackFinished()
{
    if(looping.Get())
        Play();
}

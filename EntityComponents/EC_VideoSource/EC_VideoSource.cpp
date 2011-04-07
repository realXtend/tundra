// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "Platform.h"

#include "EC_VideoSource.h"

#include "EC_Mesh.h"
#include "EC_OgreCustomObject.h"
#include "IModule.h"
#include "Framework.h"
#include "SceneAPI.h"
#include "Entity.h"
#include "EC_Placeable.h"
#include "SceneManager.h"
//#include "MediaPlayerService.h"
#include "EventManager.h"
#include "SceneManager.h"

#include <QUrl>
#include <QFile>
#include <QCryptographicHash>
#include <QSizePolicy>

#ifdef ENABLE_TAIGA_SUPPORT
#include "RexTypes.h"
#endif

#include "LoggingFunctions.h"

#include "MemoryLeakCheck.h"

#define REFRESH_RATE (15)

EC_VideoSource::EC_VideoSource(IModule *module):
    IComponent(module->GetFramework()),
    videoSourceUrl(this, "Video source url"),
    submeshIndex(this, "Submesh index", 0),
    playbackState(this, "Playback state", PS_Stop),
    audioPlaybackVolume(this, "Audio playback volume", 0.8f),
    looping(this, "Looping", false),
    refreshRate(this, "Refresh per sec", 15),
    scaleDown(this, "Scale to 320x240", false),
    startup_checker_(false),
    start_canvas_(false),
    stop_canvas_(false),
    playing_canvas_(false),
    expecting_resources_(false),
    ready_poller_(new QTimer(this)),
    player_(0),
    video_widget_(0),
    media_object_(0),
//    video_request_tag_(0),
    error_label_(0)
{
    // Init metadata for attributes
    static AttributeMetadata psAttrData;
    static bool metadataInitialized = false;
    if(!metadataInitialized)
    {
        psAttrData.enums[PS_Play] = "Play";
        psAttrData.enums[PS_Pause] = "Pause";
        psAttrData.enums[PS_Stop] = "Stop";
        metadataInitialized = true;
    }
    playbackState.SetMetadata(&psAttrData);

    static AttributeMetadata volumeMetaData("", "0", "1", "0.1");
    audioPlaybackVolume.SetMetadata(&volumeMetaData);

    // Init ready poller and signals
    ready_poller_->setSingleShot(true);

    connect(ready_poller_, SIGNAL(timeout()), SLOT(Play()));
    connect(this, SIGNAL(ParentEntitySet()), SLOT(UpdateSignals()));
    connect(this, SIGNAL(OnAttributeChanged(IAttribute*, AttributeChange::Type)), SLOT(AttributeUpdated(IAttribute*)));

    // Register as a event listener
    EventManager *event_manager = framework_->GetEventManager().get();
    if (event_manager)
    {
        event_manager->RegisterEventSubscriber(this, 99);
        asset_event_category_ = event_manager->QueryEventCategory("Asset");
    }

    playbackState.Set(PS_Stop, AttributeChange::LocalOnly);
}

EC_VideoSource::~EC_VideoSource()
{
    if (ready_poller_)
        ready_poller_->stop();
    if (error_label_)
        error_label_->deleteLater();
    if (player_)
    {
        if (player_->isPlaying())
            player_->stop();
        player_->close();
        player_->deleteLater();
    }
}

bool EC_VideoSource::HandleEvent(event_category_id_t category_id, event_id_t event_id, IEventData* data)
{
    ///\todo Regression. Reimplement using the new Asset API. -jj.
/*
    if (!expecting_resources_)
        return false;

    if (category_id == asset_event_category_)
    {
        if (event_id == Asset::Events::ASSET_READY)
        {
            Asset::Events::AssetReady *in_data = dynamic_cast<Asset::Events::AssetReady*>(data);
            if (in_data)
            {
                if (in_data->tag_ == video_request_tag_)
                {
                    LoadVideo(in_data->asset_);
                    expecting_resources_ = false;
                }
            }
        }
        else if (event_id == Asset::Events::ASSET_CANCELED)
        {
            Asset::Events::AssetCanceled *in_data = dynamic_cast<Asset::Events::AssetCanceled*>(data);
            if (in_data)
            {
                if (in_data->asset_id_ == getvideoSourceUrl().toStdString())
                {
                    LogDebug("Could not download video, URL incorrect?");
                    expecting_resources_ = false;
                }
            }
        }
    }
*/
    return false;    
}

/*
    ///\todo Regression. Reimplement using the new Asset API. -jj.
void EC_VideoSource::LoadVideo(Foundation::AssetInterfacePtr asset)
{
    Foundation::AssetServiceInterface *asset_service = GetFramework()->GetService<Foundation::AssetServiceInterface>();
    if (asset_service)
    {
        current_video_path_ = asset_service->GetAbsoluteAssetPath(asset->GetId(), asset->GetType());
        current_video_path_ = current_video_path_.replace("\\", "/");
        if (!current_video_path_.isEmpty())
            LoadCurrentVideo();
    }
    else
    {
        LogDebug("Could not get asset service");
        return;
    }
}
*/
void EC_VideoSource::LoadCurrentVideo()
{
    if (QFile::exists(current_video_path_))
    {
        if (media_object_->currentSource().fileName() != current_video_path_)
        {
            player_->load(current_video_path_);
            qDebug() << "-- loaded source file in for buffering";
        }
    }
    else
    {
        qDebug() << "-- source file does not exist yet, trying again later";
        QTimer::singleShot(500, this, SLOT(LoadCurrentVideo()));
    }
}

void EC_VideoSource::InitializePhonon()
{
    try
    {
        LogDebug("Initializing phonon components");

        player_ = new Phonon::VideoPlayer(Phonon::VideoCategory);
        player_->setAttribute(Qt::WA_ForceUpdatesDisabled, true);
        media_object_ = player_->mediaObject();
        video_widget_ = player_->videoWidget();
        video_widget_->setAttribute(Qt::WA_ForceUpdatesDisabled, true);

        video_widget_->setAspectRatio(Phonon::VideoWidget::AspectRatioWidget);
        video_widget_->setScaleMode(Phonon::VideoWidget::FitInView);    

        connect(media_object_, SIGNAL(stateChanged(Phonon::State, Phonon::State)), SLOT(PlayerStateChanged(Phonon::State, Phonon::State)), Qt::QueuedConnection);
        connect(media_object_, SIGNAL(bufferStatus(int)), SLOT(BufferStatus(int)), Qt::QueuedConnection);
        connect(media_object_, SIGNAL(finished()), SLOT(PlaybackFinished()), Qt::QueuedConnection);
        connect(media_object_, SIGNAL(hasVideoChanged(bool)), this, SLOT(StartVideoPlayback(bool)), Qt::QueuedConnection);

        playbackState.Set(PS_Stop, AttributeChange::LocalOnly);
    }
    catch(std::exception e)
    {
        LogDebug("Failed to initialize all the mumble components. Exception occurred: " + std::string(e.what()));
    }
}

void EC_VideoSource::AttributeUpdated(IAttribute *attribute)
{
    if (!video_widget_ || !media_object_)
        return;

    bool update_canvas = false;
    if (attribute->GetNameString() == videoSourceUrl.GetNameString())
    {
        if (!getvideoSourceUrl().isEmpty())
        {
            QString source_string = getvideoSourceUrl();
            QUrl source_url(source_string, QUrl::TolerantMode);
            if (source_url.isValid())
            {
                if (media_object_->currentSource().url() != source_url)
                {
    ///\todo Regression. Reimplement using the new Asset API. -jj.
/*
                    Foundation::AssetServiceInterface *asset_service = GetFramework()->GetService<Foundation::AssetServiceInterface>();
                    if (asset_service)
                    {
                        Foundation::AssetInterfacePtr asset = asset_service->GetAsset(source_string.toStdString(), RexTypes::ASSETTYPENAME_VIDEO);
                        if (asset)
                        {
                            LoadVideo(asset);
                        }
                        else
                        {
                            if (expecting_resources_ == false)
                            {
                                video_request_tag_ = asset_service->RequestAsset(source_string.toStdString(), RexTypes::ASSETTYPENAME_VIDEO);
                                if (video_request_tag_)
                                {
                                    expecting_resources_ = true;
                                    LogDebug("Downloading URL source: " + source_string.toStdString());
                                }
                            }
                        }
                    }
*/
                }
            }
            else if (QFile::exists(getvideoSourceUrl()))
            {
                if (media_object_->currentSource().fileName() != getvideoSourceUrl())
                {
                    player_->load(Phonon::MediaSource(getvideoSourceUrl()));
                    LogDebug("Loading file source: " + getvideoSourceUrl().toStdString());
                }
            }
        }
        else
        {
            if (!media_object_->currentSource().fileName().isEmpty())
            {
                media_object_->clear();
                qDebug() << "-- empty source set, clearing media object";
            }
        }
    }
    else if (attribute->GetNameString() == playbackState.GetNameString())
    {
        if (!videoSourceUrl.Get().isEmpty())
        {
            // Play video if video source url has been set and if sound has been triggered or looped.
            if (getplaybackState() == PS_Play && !playing_canvas_)
            {
                Play();
            }
            else if (getplaybackState() == PS_Stop && playing_canvas_)
            {
                Stop();
                stop_canvas_ = true;
                update_canvas = true;
            }
            else if (getplaybackState() == PS_Pause && playing_canvas_)
            {
                Pause();
                stop_canvas_ = true;
                update_canvas = true;
            }
        }
    }
    else if (attribute->GetNameString() == audioPlaybackVolume.GetNameString())
    {
        qreal volume = getaudioPlaybackVolume();
        if (player_->volume() != volume)
        {
            player_->setVolume(volume);
            LogDebug("Volume set to " + QString::number(volume).toStdString());
        }
    }
    else if (attribute->GetNameString() == refreshRate.GetNameString())
    {
        EC_3DCanvas *canvas = Get3DCanvas();
        if (canvas)
        {
            if (getrefreshRate() > 0)
            {
                int ref_rate_msec = 1000 / getrefreshRate();
                if (canvas->GetRefreshRate() != ref_rate_msec)
                {
                    canvas->SetRefreshRate(getrefreshRate());
                    LogDebug("Refresh rate set");
                }
            }
            else if (canvas->GetRefreshRate() != 0)
                canvas->SetRefreshRate(0);
        }
        else
            LogError("Could not get 3D Canvas component to set refresh rate");
    }
    else if (attribute->GetNameString() == scaleDown.GetNameString())
    {
        if (getscaleDown() && player_)
        {
            original_size_ = player_->size();
            if (original_size_.width() < 1 && original_size_.height() < 1)
                original_size_ = QSize(0,0);
            else
                qDebug() << "Original size set: " << original_size_;

            if (original_size_.width() > 1 && original_size_.height() > 1)
            {
                QSize scale_down_size(320, 240);
                if (player_->size().width() > scale_down_size.width() && player_->size().height() > scale_down_size.height())
                {
                    //player_->resize(scale_down_size);
                    //LogDebug("Scaled video widget down to 320x240 for performance");
                }
            }
        }
        else if (!getscaleDown() && player_)
        {
            if (!original_size_.isNull())
            {
                if (original_size_.width() > 0 && original_size_.height() > 0)
                {
                    if (player_->size().width() > original_size_.width() && player_->size().height() > original_size_.height())
                    {
                        //player_->resize(original_size_);
                        //LogDebug("Restored original video widget size");
                    }
                }
                else
                    original_size_ = QSize(0,0);
            }
        }
    }

    if (update_canvas)
        UpdateCanvas();
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
    if (!video_widget_ || !media_object_)
        return;

    if (media_object_->state() == Phonon::LoadingState && getplaybackState() != PS_Stop)
    {
        if (!ready_poller_->isActive())
            ready_poller_->start(1000);
        return;
    }
    else if (ready_poller_->isActive())
        ready_poller_->stop();
    
    if (media_object_->state() != Phonon::PlayingState || media_object_->state() == Phonon::PausedState)
    {
        if (media_object_->hasVideo() && !player_->isPlaying())
        {
            LogDebug("Play");
            player_->play();
            if (original_size_.isNull())
            {
                original_size_ = player_->size();
                if (original_size_.width() < 1 && original_size_.height() < 1)
                    original_size_ = QSize(0,0);
                else
                    qDebug() << "Original size set: " << original_size_;
            }
            

            start_canvas_ = true;
            UpdateCanvas();
        }
        else
            LogDebug("-- source does not have video, cannot play");
    }
    if (getplaybackState() != PS_Play)        
        playbackState.Set(PS_Play, AttributeChange::LocalOnly);
}

void EC_VideoSource::Stop()
{
    
    if (media_object_->state() == Phonon::PlayingState)
    {
        LogDebug("Stop");
        player_->stop();
        player_->seek(0);
    }
    if (getplaybackState() != PS_Stop)
        playbackState.Set(PS_Stop, AttributeChange::LocalOnly);
}

void EC_VideoSource::Pause()
{
    if (media_object_->state() == Phonon::PlayingState)
    {
        LogDebug("Pause");
        player_->pause();
    }
    if (getplaybackState() != PS_Pause)
        playbackState.Set(PS_Pause, AttributeChange::LocalOnly);
}

void EC_VideoSource::UpdateSignals()
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

    // hack hack: seems phonon is more unstable to if we login to a world and instantiate many
    // players ~at the same time, so here we do a random timer to instantiate the objects so the QThreads are not
    // started ~at the same time and we minimize risk or crashing. Yes i know its stupid, blame phonon!
    int rand_time = qrand();
    while(rand_time > 3000)
        rand_time /= 2;
        if (rand_time < 500)
            rand_time = 500;
 
    // The magic number of instances before certain unstability is two, 
    // lets not let instantiate more phonon players than that for now
    Scene::EntityList list = GetFramework()->Scene()->GetDefaultScene()->GetEntitiesWithComponent(TypeNameStatic());
    if (list.size() < 2)
    {
        LogDebug(QString("Launching video ec in %1 ms").arg(rand_time).toStdString());
        QTimer::singleShot(rand_time, this, SLOT(InitializePhonon()));
    }
    else
    {
        LogDebug("Will not instantiate Phonon objects, world already has 2 EC_VideoSources. Limit hit.");
        canvas_ = Get3DCanvas();
        if (!canvas_)
        {
            LogError("Could not get 3D Canvas component");
            return;
        }

        // Tell in the 3D scene that the video cannot be displayed as two videos are already instantiated
        error_label_ = new QLabel("Maximum of inworld videos already in use");
        error_label_->setFont(QFont("Arial", 14));
        error_label_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        error_label_->setAlignment(Qt::AlignCenter);
        QSize size = error_label_->size();
        size.setHeight(size.height()+10);
        size.setWidth(size.width()+10);
        error_label_->resize(size);
  
        canvas_->SetWidget(error_label_);
        canvas_->SetRefreshRate(0);
        canvas_->SetSubmesh(0);
        QTimer::singleShot(100, canvas_, SLOT(Update()));
    }

    RegisterActions();
}

void EC_VideoSource::UpdateCanvas()
{
    if (!video_widget_ || !media_object_)
        return;

    Scene::Entity* entity = GetParentEntity();
    if (!entity)
    {
        LogError("No parent entity, cannot create/update 3DCanvas");
        return;
    }

    // If entity has no valid mesh or prim yet, start a retry timer and try to set the canvas later
    if ((!entity->GetComponent(EC_Mesh::TypeNameStatic())) && 
        (!entity->GetComponent(EC_OgreCustomObject::TypeNameStatic())))
    {
        LogDebug("Mesh or prim did not exist yet, retrying");
        QTimer::singleShot(500, this, SLOT(UpdateCanvas()));
        return;
    }

    canvas_ = Get3DCanvas();
    if (!canvas_)
    {
        LogError("Could not get 3D Canvas component");
        return;
    }

    // Update widget
    if (canvas_->GetWidget() != player_)
    {
        canvas_->SetWidget(player_);
        LogDebug("Widget set");
    }

    // Update submesh
    int submesh = getsubmeshIndex();
    if (submesh < 0)
        submesh = 0;
    if (!canvas_->GetSubMeshes().contains(submesh))
    {
        canvas_->SetSubmesh(submesh);
        LogDebug("Submesh set");
    }

    // Update refresh rate
    if (getrefreshRate() > 0)
    {
        int ref_rate_msec = 1000 / getrefreshRate();
        if (canvas_->GetRefreshRate() != ref_rate_msec)
        {
            canvas_->SetRefreshRate(getrefreshRate());
            LogDebug("Refresh rate set");
        }
    }
    else
        canvas_->SetRefreshRate(0);

    // Scale down widget
    if (getscaleDown() && player_)
    {
        if (original_size_.isNull())
        {
            original_size_ = player_->size();
            if (original_size_.width() < 1 && original_size_.height() < 1)
                original_size_ = QSize(0,0);
            else
                qDebug() << "Original size set: " << original_size_;
        }

        if (!original_size_.isNull())
        {
            QSize scale_down_size(320, 240);
            if (player_->size().width() > scale_down_size.width() && player_->size().height() > scale_down_size.height())
            {
                //player_->resize(scale_down_size);
                //LogDebug("Scaled video widget down to 320x240 for performance");
            }
        }
    }

    if (start_canvas_)
    {
        canvas_->Start();
        start_canvas_ = false;
        playing_canvas_ = true;
        LogDebug("Started rendering updates");

        if (ready_poller_->isActive())
            ready_poller_->stop();
    }

    if (stop_canvas_)
    {
        canvas_->Stop();
        start_canvas_ = false;
        stop_canvas_ = false;
        playing_canvas_ = false;
        LogDebug("Stoppend rendering updates");
    }
}

void EC_VideoSource::BufferStatus(int filled)
{
    LogDebug(QString("Buffering %1 %").arg(filled).toStdString());
}

void EC_VideoSource::PlayerStateChanged(Phonon::State new_state, Phonon::State old_state)
{
    switch (new_state)
    {
        case Phonon::LoadingState:
        {
            break;
        }
        case Phonon::PlayingState:
        {
            break;
        }
        case Phonon::PausedState:
        {
            if (player_->currentTime() == player_->totalTime())
            {
                if (!getlooping())
                {
                    stop_canvas_ = true;
                    UpdateCanvas();
                }
            }
            break;
        }
        case Phonon::StoppedState:
        {
            if (old_state == Phonon::LoadingState && getplaybackState() == PS_Play)
            {
                LogDebug("-- Loading done");
                if (!player_->isPlaying() && !ready_poller_->isActive())
                {
                    Play();
                    UpdateCanvas();
                }
            }
            break;
        }
        case Phonon::ErrorState:
        {
            LogError("Could not open video stream: " + media_object_->errorString().toStdString());
            break;
        }
        case Phonon::BufferingState:
        {
            break;
        }
    }
}

void EC_VideoSource::StartVideoPlayback(bool has_video)
{
    //qDebug() << "-- source has video: " << has_video;
}

void EC_VideoSource::PlaybackFinished()
{
    if (looping.Get())
    {
        LogDebug("Looping enabled, restarting video");
        player_->seek(0);
        Play();
    }
}

EC_3DCanvas *EC_VideoSource::Get3DCanvas()
{
    EC_3DCanvas *canvas = 0;
    Scene::Entity* entity = GetParentEntity();
    if (entity)
        canvas = dynamic_cast<EC_3DCanvas*>(entity->GetOrCreateComponent(EC_3DCanvas::TypeNameStatic()).get());
    else
        LogError("Could not get parent entity");
    return canvas;
}

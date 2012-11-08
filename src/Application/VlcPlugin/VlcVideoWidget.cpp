// For conditions of distribution and use, see copyright notice in LICENSE

#include "VlcVideoWidget.h"
#include "LoggingFunctions.h"

#include <QPainter>
#include <QUrl>
#include <QVarLengthArray>

VlcVideoWidget::VlcVideoWidget(const QList<QByteArray> &args) :
    QFrame(0),
    vlcInstance_(0),
    vlcPlayer_(0),
    vlcMedia_(0),
    hasVideoOut_(false)
{
    /// Convert the arguments into a proper form
    QVarLengthArray<const char*, 64> vlcArgs(args.size());
    for (int i = 0; i < args.size(); ++i)
        vlcArgs[i] = args.at(i).constData();

    // Create new libvlc instance
    vlcInstance_ = libvlc_new(vlcArgs.size(), vlcArgs.constData());

    // Check if instance is running
    if (!vlcInstance_)
    {
        std::string errorMsg;
        if (libvlc_errmsg())
            errorMsg = std::string("VlcVideoWidget: Failed to create VLC instance: ") + libvlc_errmsg();
        else
            errorMsg = "VlcVideoWidget: Failed to create VLC instance";
        LogError(errorMsg);
        return;
    }

    libvlc_set_user_agent(vlcInstance_, "realXtend Tundra Media Player 1.0", "Tundra/1.0");

    /// Create the vlc player and set event callbacks
    vlcPlayer_ = libvlc_media_player_new(vlcInstance_);
    libvlc_event_manager_t *em = libvlc_media_player_event_manager(vlcPlayer_);
    libvlc_event_attach(em, libvlc_MediaPlayerMediaChanged, &VlcEventHandler, this);
    libvlc_event_attach(em, libvlc_MediaPlayerOpening, &VlcEventHandler, this);
    libvlc_event_attach(em, libvlc_MediaPlayerBuffering, &VlcEventHandler, this);
    libvlc_event_attach(em, libvlc_MediaPlayerPlaying, &VlcEventHandler, this);
    libvlc_event_attach(em, libvlc_MediaPlayerPaused, &VlcEventHandler, this);
    libvlc_event_attach(em, libvlc_MediaPlayerStopped, &VlcEventHandler, this);
    libvlc_event_attach(em, libvlc_MediaPlayerEncounteredError, &VlcEventHandler, this);
    libvlc_event_attach(em, libvlc_MediaPlayerTimeChanged, &VlcEventHandler, this);
    libvlc_event_attach(em, libvlc_MediaPlayerPositionChanged, &VlcEventHandler, this);
    libvlc_event_attach(em, libvlc_MediaPlayerSeekableChanged, &VlcEventHandler, this);
    libvlc_event_attach(em, libvlc_MediaPlayerPausableChanged, &VlcEventHandler, this);
    libvlc_event_attach(em, libvlc_MediaPlayerTitleChanged, &VlcEventHandler, this);
    libvlc_event_attach(em, libvlc_MediaPlayerLengthChanged, &VlcEventHandler, this);

    /// register callbacks so that we can implement custom drawing of video frames
    libvlc_video_set_callbacks(
        vlcPlayer_,
        &CallBackLock,
        &CallBackUnlock,
        &CallBackDisplay,
        this);

    // Initialize rendering
    //idleLogo_ = QImage(":/images/vlc-cone.png");
    idleLogo_ = QImage(":/images/play-video.png");
    idleLogo_ = idleLogo_.convertToFormat(QImage::Format_ARGB32);

    //audioLogo_ = QImage(":/images/audio.png");
    audioLogo_ = QImage(":/images/play-audio.png");
    audioLogo_ = audioLogo_.convertToFormat(QImage::Format_ARGB32);
    
    pausePixmap_ = QPixmap(":/images/pause-big.png");
    bufferingPixmap_ = QPixmap(":/images/buffering.png");

    QSize startSize(600, 360);
    renderPixmap_ = QImage(startSize.width(), startSize.height(), QImage::Format_RGB32);
    libvlc_video_set_format(vlcPlayer_, "RV32", renderPixmap_.width(), renderPixmap_.height(), renderPixmap_.bytesPerLine());

    // Initialize widget properties
    setMinimumSize(startSize);
    resize(startSize);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setStyleSheet("background-color: black;");

    // Connect and start status poller
    connect(&pollTimer_, SIGNAL(timeout()), SLOT(StatusPoller()));
    pollTimer_.start(10);
}

VlcVideoWidget::~VlcVideoWidget() 
{
    if (Initialized())
        ShutDown();
}

bool VlcVideoWidget::Initialized() const
{
    if (!vlcInstance_ || !vlcPlayer_)
        return false;
    return true;
}

bool VlcVideoWidget::OpenSource(const QString &videoUrl) 
{
    if (!Initialized())
        return false;

    vlcMedia_ = libvlc_media_new_path(vlcInstance_, videoUrl.toAscii().constData());
    if (vlcMedia_ == 0)
    {
        LogError("VlcVideoWidget: Could not load media from '" + videoUrl + "'");
        return false;
    }

    libvlc_event_manager_t *em = libvlc_media_event_manager(vlcMedia_);
    libvlc_event_attach(em, libvlc_MediaMetaChanged, &VlcEventHandler, this);
    libvlc_event_attach(em, libvlc_MediaSubItemAdded, &VlcEventHandler, this);
    libvlc_event_attach(em, libvlc_MediaDurationChanged, &VlcEventHandler, this);
    libvlc_event_attach(em, libvlc_MediaParsedChanged, &VlcEventHandler, this);
    libvlc_event_attach(em, libvlc_MediaFreed, &VlcEventHandler, this);
    libvlc_event_attach(em, libvlc_MediaStateChanged, &VlcEventHandler, this);

    libvlc_state_t state = libvlc_media_get_state(vlcMedia_);

    if (state != libvlc_Error)
    {
        // Reset playback
        Stop();
        hasVideoOut_ = false;

        libvlc_media_player_set_media(vlcPlayer_, vlcMedia_);      

        statusAccess.lock();
        status.Reset();
        status.source = videoUrl;
        status.change = PlayerStatus::MediaSource;
        statusAccess.unlock();

        return true;
    }
    else
    {
        std::string err = "Unknown error";
        if (libvlc_errmsg())
        {
            err = libvlc_errmsg();
            libvlc_clearerr();
        }
        LogError("VlcVideoWidget: " + err);
    }
    return false;
}

libvlc_state_t VlcVideoWidget::GetMediaState() const
{
    if (!Initialized() || !vlcMedia_)
        return libvlc_Error;
    return libvlc_media_get_state(vlcMedia_);
}

s64 VlcVideoWidget::GetMediaLenght()
{
    s64 len = 0;
    if (!Initialized() || !vlcMedia_)
        return len;

    if (statusAccess.tryLock(5))
    {
        len = status.lenght;
        statusAccess.unlock();
    }
    return len;
}

s64 VlcVideoWidget::GetMediaTime()
{
    s64 time = 0;
    if (!Initialized() || !vlcMedia_)
        return time;

    if (statusAccess.tryLock(5))
    {
        time = status.time;
        statusAccess.unlock();
    }
    return time;
}

bool VlcVideoWidget::TogglePlay()
{
    statusAccess.lock();
    bool stoppedNow = status.stopped;
    statusAccess.unlock();
    if (stoppedNow)
    {
        Play();
        return true;
    }
    else
    {
        Pause();
        return (libvlc_media_player_is_playing(vlcPlayer_) > 0 ? true : false);
    }
}

void VlcVideoWidget::Play() 
{
    if (!vlcPlayer_)
        return;
    if (!libvlc_media_player_is_playing(vlcPlayer_))
        libvlc_media_player_play(vlcPlayer_);
}

void VlcVideoWidget::Pause() 
{
    if (!vlcPlayer_)
        return;
    if (libvlc_media_player_can_pause(vlcPlayer_))
        libvlc_media_player_pause(vlcPlayer_);
}

void VlcVideoWidget::Stop() 
{
    if (vlcPlayer_)
    {
        libvlc_state_t state = GetMediaState();
        if (state == libvlc_Playing || state == libvlc_Paused || state == libvlc_Ended)
        {
            /** @bug @todo These should not be here and is not actually doing anything. 
                Take a fresh look at the threading in this object and remove these hacks. */
            if (onScreenPixmapMutex_.tryLock(50))
                onScreenPixmapMutex_.unlock();
            if (renderPixmapMutex_.tryLock(50))
                renderPixmapMutex_.unlock();
            if (statusAccess.tryLock(50))
                statusAccess.unlock();

            libvlc_media_player_stop(vlcPlayer_);
        }
        update();
        ForceUpdateImage();
    }
}

bool VlcVideoWidget::Seek(s64 time)
{
    if (vlcPlayer_)
    {
        if (libvlc_media_player_is_playing(vlcPlayer_))
        {
            if (libvlc_media_player_is_seekable(vlcPlayer_))
            {
                libvlc_media_player_set_time(vlcPlayer_, time);
                return true;
            }
        }
    }
    return false;
}

void VlcVideoWidget::ForceUpdateImage()
{
    QImage image;
    QPixmap addition;
    QPoint additionPos(10, 10);

    // No media or stopped
    statusAccess.tryLock();
    bool paused = status.paused;
    bool buffering = status.buffering;
    bool stopped = status.stopped;
    QString source = status.source;
    statusAccess.unlock();

    onScreenPixmapMutex_.lock();
    if (source.isEmpty() || stopped)
        image = idleLogo_;
    // Has media and is being played/paused/buffered
    else
    {
        // Has video
        if (hasVideoOut_)
            image = rgbaBuffer_;
        // Has audio
        else
            image = audioLogo_;
        // Addition
        if (paused)
            addition = pausePixmap_;
        if (buffering)
            addition = bufferingPixmap_;
    }
    onScreenPixmapMutex_.unlock();

    // Add additional image if there is one
    if (!addition.isNull())
    {
        QPainter p(&image);
        p.drawPixmap(additionPos, addition, addition.rect());
        p.end();
    }
    // Emit image
    emit FrameUpdate(image);

    update();
}

void VlcVideoWidget::ShutDown()
{
    if (vlcPlayer_ && vlcInstance_)
    {
        /** @bug @todo These should not be here and is not actually doing anything. 
            Take a fresh look at the threading in this object and remove these hacks. */
        if (onScreenPixmapMutex_.tryLock(50))
            onScreenPixmapMutex_.unlock();
        if (renderPixmapMutex_.tryLock(50))
            renderPixmapMutex_.unlock();
        if (statusAccess.tryLock(50))
            statusAccess.unlock();

        libvlc_media_release(vlcMedia_);
        libvlc_media_player_stop(vlcPlayer_);
        libvlc_media_player_release(vlcPlayer_);
        libvlc_release(vlcInstance_);

        vlcMedia_ = 0;
        vlcPlayer_ = 0;
        vlcInstance_ = 0;

        close();
    }
}

void VlcVideoWidget::DetermineVideoSize()
{
    if (!status.sourceSize.isNull())
        return;

    uint w, h;
    if (libvlc_video_get_size(vlcPlayer_, 0, &w, &h) == 0)
    {
        status.sourceSize = QSize(w,h);        
        status.playing = false;
        status.paused = false;
        status.stopped = false;
        status.doRestart = true;
        status.doStop = true;
        status.change = PlayerStatus::MediaSize;
    }
}

void VlcVideoWidget::RestartPlayback()
{
    if (!status.doRestart)
        return;
    status.doRestart = false;
    
    vlcPlayer_ = libvlc_media_player_new_from_media(vlcMedia_);
    
    libvlc_event_manager_t *em = libvlc_media_player_event_manager(vlcPlayer_);
    libvlc_event_attach(em, libvlc_MediaPlayerMediaChanged, &VlcEventHandler, this);
    libvlc_event_attach(em, libvlc_MediaPlayerOpening, &VlcEventHandler, this);
    libvlc_event_attach(em, libvlc_MediaPlayerBuffering, &VlcEventHandler, this);
    libvlc_event_attach(em, libvlc_MediaPlayerPlaying, &VlcEventHandler, this);
    libvlc_event_attach(em, libvlc_MediaPlayerPaused, &VlcEventHandler, this);
    libvlc_event_attach(em, libvlc_MediaPlayerStopped, &VlcEventHandler, this);
    libvlc_event_attach(em, libvlc_MediaPlayerEncounteredError, &VlcEventHandler, this);
    libvlc_event_attach(em, libvlc_MediaPlayerTimeChanged, &VlcEventHandler, this);
    libvlc_event_attach(em, libvlc_MediaPlayerPositionChanged, &VlcEventHandler, this);
    libvlc_event_attach(em, libvlc_MediaPlayerSeekableChanged, &VlcEventHandler, this);
    libvlc_event_attach(em, libvlc_MediaPlayerPausableChanged, &VlcEventHandler, this);
    libvlc_event_attach(em, libvlc_MediaPlayerTitleChanged, &VlcEventHandler, this);
    libvlc_event_attach(em, libvlc_MediaPlayerLengthChanged, &VlcEventHandler, this);

    libvlc_video_set_callbacks(
        vlcPlayer_,
        &CallBackLock,
        &CallBackUnlock,
        &CallBackDisplay,
        this);

    renderPixmap_ = QImage(status.sourceSize.width(), status.sourceSize.height(), QImage::Format_RGB32);
    libvlc_video_set_format(vlcPlayer_, "RV32", renderPixmap_.width(), renderPixmap_.height(), renderPixmap_.bytesPerLine());

    if (!status.doNotPlayAfterRestart)
    {
        Play();
        status.doNotPlayAfterRestart = false;
    }
}

void VlcVideoWidget::StatusPoller()
{
    if (statusAccess.tryLock())
    {
        if (status.doStop)
        {
            Stop();
            libvlc_media_player_release(vlcPlayer_);
            status.doStop = false;
        }
        else if (status.doRestart && !status.playing)
        {
            RestartPlayback();
            status.doRestart = false;
        }

        // Emit a status update if there is one
        if (status.change != PlayerStatus::NoChange)
        {
            emit StatusUpdate(status);
            status.change = PlayerStatus::NoChange;
        }

        statusAccess.unlock();
    }
}

void* VlcVideoWidget::InternalLock(void** pixelPlane) 
{
    renderPixmapMutex_.lock();
    *pixelPlane = renderPixmap_.bits();
    return &renderPixmap_;
}

void VlcVideoWidget::InternalUnlock(void* picture, void*const *pixelPlane) 
{
    renderPixmapMutex_.unlock();
}

void VlcVideoWidget::InternalRender(void* picture) 
{
    bool paused = false;
    QSize sourceSize = QSize();
    if (statusAccess.tryLock(5))
    {
        paused = status.paused;
        sourceSize = status.sourceSize;
        statusAccess.unlock();
    }
    else
        return;

    // Lock on screen pixmap and update it.
    // It is ok to miss some frames, so use tryLock() instead.
    // Otherwise paintEvent and this may trigger some nasty dead locks.
    if (onScreenPixmapMutex_.tryLock())
    {
        hasVideoOut_ = true;
        onScreenPixmap_ = QPixmap::fromImage(renderPixmap_);
        rgbaBuffer_ = renderPixmap_.convertToFormat(QImage::Format_ARGB32);

        // Paused
        if (paused)
        {
            QPainter p(&rgbaBuffer_);
            p.drawPixmap(QPoint(10, 10), pausePixmap_, pausePixmap_.rect());
            p.end();
        }
        // Waiting to determine true size.
        else if (sourceSize.isNull())
        {
            rgbaBuffer_.fill(QColor(242,242,242).rgb());
            QPoint centerPos = rgbaBuffer_.rect().center();
            QRect center(centerPos.x() - (bufferingPixmap_.width()/2), centerPos.y() - (bufferingPixmap_.height()/2),
                         bufferingPixmap_.width(), bufferingPixmap_.height());
            QPainter p(&rgbaBuffer_);
            p.setPen(Qt::black);
            p.drawPixmap(center, bufferingPixmap_, bufferingPixmap_.rect());
            p.drawText(5, 12, "Loading Media");
            p.end();
        }

        emit FrameUpdate(rgbaBuffer_);
        onScreenPixmapMutex_.unlock();

        // Ask the widget to render itself, should trigger paintEvent
        update();
    }  
}

void VlcVideoWidget::paintEvent(QPaintEvent *e) 
{
    if (!Initialized())
        return;

    statusAccess.lock();
    bool stopped = status.stopped;
    bool paused = status.paused;
    bool buffering = status.buffering;
    statusAccess.unlock();

    QSize widgetSize = size();
    QRect targetRect = rect();

    QPainter p(this);

    if (!stopped)
    {
        // Playing/paused/buffering state, try to get a lock, but skip if cant.
        // This lock might fail occasionally, this is when we lose a frame in the 2D preview widget.
        if (onScreenPixmapMutex_.tryLock())
        {
            QSize videoSize = (hasVideoOut_ ? onScreenPixmap_.size() : audioLogo_.size());

            double sourceAspectRatio = double(videoSize.height()) / videoSize.width();
            double displayAspectRatio = double(widgetSize.height()) / widgetSize.width();

            QSize scaled = videoSize;
            scaled.scale(widgetSize, Qt::KeepAspectRatio);

            if (displayAspectRatio >= sourceAspectRatio)
                targetRect = QRect(0, (widgetSize.height() - scaled.height()) / 2, widgetSize.width(), scaled.height());
            else
                targetRect = QRect((widgetSize.width() - scaled.width()) / 2, 0, scaled.width(), widgetSize.height());

            // Render audio logo for non video sources, the video image for others.
            if (hasVideoOut_)
                p.drawPixmap(targetRect, onScreenPixmap_, onScreenPixmap_.rect());
            else
                p.drawPixmap(targetRect, QPixmap::fromImage(audioLogo_), audioLogo_.rect());

            // Add some eye candy for paused and buffering states
            if (paused)
                p.drawPixmap(QPoint(10,10), pausePixmap_, pausePixmap_.rect());
            if (buffering)
                p.drawPixmap(QPoint(10,10), bufferingPixmap_, bufferingPixmap_.rect());

            onScreenPixmapMutex_.unlock();
        }
    }
    else
    {
        // Stopped state, render the default idle state logo
        QSize videoSize = idleLogo_.size();

        double sourceAspectRatio = double(videoSize.height()) / videoSize.width();
        double displayAspectRatio = double(widgetSize.height()) / widgetSize.width();

        QSize scaled = videoSize;
        scaled.scale(widgetSize, Qt::KeepAspectRatio);

        if (displayAspectRatio >= sourceAspectRatio)
            targetRect = QRect(0, (widgetSize.height() - scaled.height()) / 2, widgetSize.width(), scaled.height());
        else
            targetRect = QRect((widgetSize.width() - scaled.width()) / 2, 0, scaled.width(), widgetSize.height());

        p.drawPixmap(targetRect, QPixmap::fromImage(idleLogo_), idleLogo_.rect());
    }

    p.end();
}

/// Vlc callbacks

void* VlcVideoWidget::CallBackLock(void* widget, void** pixelPlane)
{
    VlcVideoWidget* w = reinterpret_cast<VlcVideoWidget*>(widget);
    return w->InternalLock(pixelPlane);
}

void VlcVideoWidget::CallBackUnlock(void* widget, void* picture, void*const *pixelPlane)
{
    VlcVideoWidget* w = reinterpret_cast<VlcVideoWidget*>(widget);
    w->InternalUnlock(picture, pixelPlane);
}

void VlcVideoWidget::CallBackDisplay(void* widget, void* picture)
{
    VlcVideoWidget* w = reinterpret_cast<VlcVideoWidget*>(widget);
    w->InternalRender(picture);
}

void VlcVideoWidget::VlcEventHandler(const libvlc_event_t *event, void *widget)
{
    // Return on certain events
    switch (event->type)
    {
        case libvlc_MediaPlayerTitleChanged:
        case libvlc_MediaMetaChanged:
        case libvlc_MediaSubItemAdded:
        case libvlc_MediaDurationChanged:
        case libvlc_MediaParsedChanged:
        case libvlc_MediaFreed:
            return;
        default:
            break;
    }

    VlcVideoWidget* w = reinterpret_cast<VlcVideoWidget*>(widget);
    w->statusAccess.lock();

    switch (event->type)
    {
        // Media player events
        case libvlc_MediaPlayerMediaChanged:
        {
            w->DetermineVideoSize();
            break;
        }
        case libvlc_MediaPlayerOpening:
        {
            w->DetermineVideoSize();
            break;
        }
        case libvlc_MediaPlayerBuffering:
        {
            w->DetermineVideoSize();
            w->status.buffering = true;
            w->status.playing = false;
            w->status.paused = false;
            w->status.stopped = false;
            w->status.change = PlayerStatus::MediaState;
            break;
        }
        case libvlc_MediaPlayerPlaying:
        {
            w->DetermineVideoSize();
            w->status.buffering = false;
            w->status.playing = true;
            w->status.paused = false;
            w->status.stopped = false;
            w->status.change = PlayerStatus::MediaState;
            break;
        }
        case libvlc_MediaPlayerPaused:
        {
            w->status.buffering = false;
            w->status.playing = false;
            w->status.paused = true;
            w->status.stopped = false;
            w->status.change = PlayerStatus::MediaState;
            break;
        }
        case libvlc_MediaPlayerStopped:
        {
            w->status.buffering = false;
            w->status.playing = false;
            w->status.paused = false;
            w->status.stopped = true;
            w->status.time = 0.0;
            w->status.change = PlayerStatus::MediaState;
            break;
        }
        case libvlc_MediaPlayerEncounteredError:
        {
            std::string err = "Unknown error";
            if (libvlc_errmsg())
            {
                err = libvlc_errmsg();
                libvlc_clearerr();
            }
            w->status.error = QString("Media player encountered error: ") + err.c_str();
            w->status.change = PlayerStatus::PlayerError;
            break;
        }
        case libvlc_MediaPlayerTimeChanged:
        {   
            w->DetermineVideoSize();
            w->status.time = event->u.media_player_time_changed.new_time;
            w->status.change = PlayerStatus::MediaTime;
            break;
        }
        case libvlc_MediaPlayerPositionChanged:
        {
            w->DetermineVideoSize();
            w->status.position = event->u.media_player_position_changed.new_position;
            w->status.change = PlayerStatus::MediaTime;
            break;
        }
        case libvlc_MediaPlayerSeekableChanged:
        {
            w->DetermineVideoSize();
            w->status.isSeekable = (event->u.media_player_seekable_changed.new_seekable > 0 ? true : false);
            w->status.change = PlayerStatus::MediaProperty;
            break;
        }
        case libvlc_MediaPlayerPausableChanged:
        {
            w->DetermineVideoSize();
            w->status.isPausable = (event->u.media_player_pausable_changed.new_pausable > 0 ? true : false);
            w->status.change = PlayerStatus::MediaProperty;
            break;
        }
        case libvlc_MediaPlayerLengthChanged:
        {
            w->DetermineVideoSize();
            w->status.lenght = event->u.media_player_length_changed.new_length;
            w->status.change = PlayerStatus::MediaTime;
            break;
        }
        // Media events
        case libvlc_MediaStateChanged:
        {
            if (event->u.media_state_changed.new_state == libvlc_Buffering)
            {
                w->DetermineVideoSize();
                w->status.buffering = true;
                w->status.playing = false;
                w->status.paused = false;
                w->status.stopped = false;
                w->status.change = PlayerStatus::MediaState;
            }

            if (event->u.media_state_changed.new_state == libvlc_Ended)
            {
                w->status.doStop = true;
                w->status.doNotPlayAfterRestart = true;
                w->status.doRestart = true;
                w->status.buffering = false;
                w->status.playing = false;
                w->status.paused = false;
                w->status.stopped = true;
                w->status.time = 0.0;
                w->status.change = PlayerStatus::MediaState;
            }
            break;
        }
        // Default
        default:
            break;
    }

    w->statusAccess.unlock();
}

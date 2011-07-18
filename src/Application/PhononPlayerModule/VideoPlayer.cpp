// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include <QUrl>
#include <phonon/VideoPlayer>
#include <phonon/MediaObject>
#include <phonon/VideoWidget>
#include "PhononPlayerModule.h"
#include "VideoPlayer.h"

#include "MemoryLeakCheck.h"

namespace PhononPlayer
{
    VideoPlayer::VideoPlayer(const QString &url) : video_widget_(0), error_handled_(false)
    {
        this->setLayout(new QBoxLayout(QBoxLayout::LeftToRight, this));
        PlayVideo(url);
    }

    VideoPlayer::~VideoPlayer()
    {
        media_object_.disconnect(this, SLOT(RestartVideoPlayback()));
        media_object_.disconnect(this, SLOT(StartVideoPlayback(bool)));
        media_object_.disconnect(this, SLOT(CheckState(Phonon::State, Phonon::State)));
        media_object_.clear();
    }

    void VideoPlayer::PlayVideo(const QString &url)
    {
        video_widget_ = new Phonon::VideoWidget(this);
        Phonon::createPath(&media_object_, video_widget_);
        
        QObject::connect(&media_object_, SIGNAL(stateChanged(Phonon::State, Phonon::State)), this, SLOT(CheckState(Phonon::State, Phonon::State)));
        QObject::connect(&media_object_, SIGNAL(hasVideoChanged(bool)), this, SLOT(StartVideoPlayback(bool)));
        QObject::connect(&media_object_, SIGNAL(aboutToFinish()), this, SLOT(RestartVideoPlayback()));
    
        media_object_.setCurrentSource(QUrl(url));
        media_object_.play();
    }

    void VideoPlayer::StartVideoPlayback(bool has_video)
    {
        if (!has_video)
            return;

        if (video_widget_)
        {
            video_widget_->setAspectRatio(Phonon::VideoWidget::AspectRatioWidget);
            video_widget_->setScaleMode(Phonon::VideoWidget::FitInView);    
        }

        this->layout()->addWidget(video_widget_);
        this->layout()->setContentsMargins(0, 0, 0, 0);
    }
       
    void VideoPlayer::RestartVideoPlayback()
    {
        media_object_.enqueue(media_object_.currentSource());
    }

    void VideoPlayer::CheckState(Phonon::State new_state, Phonon::State old_state)
    {
        if (new_state == Phonon::ErrorState)
        {
            if (!error_handled_)
            {
                error_handled_ = true;
                QString message = QString("Videoplayback error [%1] for %2").arg(media_object_.errorString()).arg(media_object_.currentSource().url().toString());
                PhononPlayerModule::LogError(message.toStdString());
                media_object_.clear();
            }
            return;
        }
    }
} // PhononPlayer

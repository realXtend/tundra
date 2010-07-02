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

namespace PlayerService
{
    VideoPlayer::VideoPlayer(const QString &url) : media_object_(0), video_widget_(0)
    {
        Play(url);
    }

    VideoPlayer::~VideoPlayer()
    {
        if (media_object_)
        {
            if (media_object_->state() == Phonon::PlayingState)
                media_object_->stop();

            media_object_->clear();
        }
    }

    void VideoPlayer::Play(const QString &url)
    {
        media_object_ = new Phonon::MediaObject(this);
        video_widget_ = new Phonon::VideoWidget(this);
        video_widget_->setAspectRatio(Phonon::VideoWidget::AspectRatioWidget);

        Phonon::createPath(media_object_, video_widget_);
        QObject::connect(media_object_, SIGNAL(finished()), this, SLOT(Restart()));
        QObject::connect(media_object_, SIGNAL(stateChanged(Phonon::State, Phonon::State)), this, SLOT(CheckState()));

        media_object_->setCurrentSource(url);
        media_object_->play();
    }
       
    void VideoPlayer::Restart()
    {
        if (media_object_ && (media_object_->state() == Phonon::StoppedState || media_object_->state() == Phonon::PausedState))
        {
            if (media_object_->isSeekable())
                media_object_->seek(0);
            media_object_->play();
        }
    }

    void VideoPlayer::CheckState()
    {
        if (media_object_->state() == Phonon::ErrorState)
        {
            QString message = QString("Videoplayback error [%1] for %2").arg(media_object_->errorString()).arg(media_object_->currentSource().url().toString());
            PhononPlayerModule::LogError(message.toStdString());
            return;
        }
        if (media_object_->state() == Phonon::StoppedState || media_object_->state() == Phonon::PausedState)
            Restart();
    }
}

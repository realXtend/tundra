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
    VideoPlayer::VideoPlayer(const QString &url) : media_object_(0), video_widget_(0), error_handled_(false), layout_(0)
    {
        //layout_ = new QBoxLayout(QBoxLayout::LeftToRight, this);
//        this->setLayout(new QBoxLayout(QBoxLayout::LeftToRight, this));
//        Play("http://inventaaustralia.zftp.com/videos/MPEGIO_MPEG2_176X144_Video1024Kbps.mpg");
        Play(url);
    }

    VideoPlayer::~VideoPlayer()
    {
        if (media_object_)
        {
            media_object_->disconnect(this);
            media_object_->clear();
            delete media_object_;
        }
    }

    void VideoPlayer::Play(const QString &url)
    {
        media_object_ = new Phonon::MediaObject(0);
        video_widget_ = new Phonon::VideoWidget(this);
        Phonon::createPath(media_object_, video_widget_);
        
        QObject::connect(media_object_, SIGNAL(stateChanged(Phonon::State, Phonon::State)), this, SLOT(CheckState(Phonon::State, Phonon::State)));
        QObject::connect(media_object_, SIGNAL(hasVideoChanged(bool)), this, SLOT(StartVideoPlayback(bool)));
        QObject::connect(media_object_, SIGNAL(aboutToFinish()), this, SLOT(RestartVideoPlayback()));
    
        media_object_->setCurrentSource(url);
        media_object_->stop();
        media_object_->play();
    }

    void VideoPlayer::StartVideoPlayback(bool has_video)
    {
        if (!has_video)
            return;

        video_widget_->setAspectRatio(Phonon::VideoWidget::AspectRatioWidget);
        video_widget_->setScaleMode(Phonon::VideoWidget::ScaleAndCrop);
        
        if (!video_widget_->parent())
        {
            //    this->layout()->addWidget(video_widget_);
                //media_object_->setParent(this);
//            layout_->addWidget(video_widget_);
//            video_widget_->setParent(this);
            //video_widget_->layout()->setMargin(0);
            //this->layout()->addWidget(video_widget_);
        }

        if (media_object_->isSeekable() && media_object_->currentTime() != 0)
            media_object_->seek(0);

        //media_object_->stop();
        //media_object_->play();
    }
       
    void VideoPlayer::RestartVideoPlayback()
    {
        media_object_->enqueue(media_object_->currentSource());
    }

    void VideoPlayer::CheckState(Phonon::State new_state, Phonon::State old_state)
    {
        if (new_state == Phonon::ErrorState)
        {
            if (!error_handled_)
            {
                error_handled_ = true;
                QString message = QString("Videoplayback error [%1] for %2").arg(media_object_->errorString()).arg(media_object_->currentSource().url().toString());
                PhononPlayerModule::LogError(message.toStdString());
                media_object_->clear();
//                video_widget_->close();
            }
            return;
        }
    }
}

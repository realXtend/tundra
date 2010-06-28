// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "Service.h"
#include <QWidget>
#include <QSignalMapper>
#include <Phonon>
#include <Phonon/BackendCapabilities>
#include "PhononPlayerModule.h"

#include "MemoryLeakCheck.h"

namespace PlayerService
{
    Service::Service()
    {
        destroyed_signal_mapper_ = new QSignalMapper();
    }

    Service::~Service()
    {
        foreach(Phonon::VideoPlayer* player, video_players_)
        {
            player->deleteLater();
        }
        video_players_.clear();
        SAFE_DELETE(destroyed_signal_mapper_);
    }

    bool Service::IsMimeTypeSupported(const QString mime_type)
    {
        return Phonon::BackendCapabilities::isMimeTypeAvailable(QString(mime_type));
    }

    QWidget* Service::GetPlayer(const QString &url)
    {
        if (video_players_.contains(url))
            return dynamic_cast<QWidget*>(video_players_[url]);

        QString message = QString("Created a player for %1").arg(url);
        PhononPlayerModule::LogDebug(message.toStdString());
        
        Phonon::VideoPlayer* player = new Phonon::VideoPlayer(Phonon::VideoCategory, 0); /// this can block for a long time !!!
        video_players_[url] = player;
        destroyed_signal_mapper_->setMapping(player, url);
        QObject::connect(player, SIGNAL(finished()), this, SLOT(UpdatePlayers()));
        QObject::connect(player->mediaObject(), SIGNAL(stateChanged(Phonon::State, Phonon::State)), this, SLOT(UpdatePlayers()));
        QObject::connect(player, SIGNAL(destroyed()), destroyed_signal_mapper_, SLOT(map()));
        QObject::connect(destroyed_signal_mapper_, SIGNAL(mapped(const QString &)), this, SLOT(PlayerDestroyed(const QString &)));

        player->setVolume(0); // We are just playing the video
        player->load(url);
        player->play();

        return player;
    }

    void Service::DeletePlayer(const QString &url)
    {
        if (!video_players_.contains(url))
            return;
        
        Phonon::VideoPlayer* player = video_players_[url];
        video_players_.remove(url);
        player->stop();
        //delete player; <-- cannot be used, this crash...
        player->deleteLater();
    }

    void Service::PlayerDestroyed(const QString &url)
    {
        QString message = QString("Destroyed player for %1").arg(url);
        PhononPlayerModule::LogDebug(message.toStdString());
    }

    void Service::UpdatePlayers()
    {
        foreach(Phonon::VideoPlayer* p, video_players_)
        {
            if (!p->isPlaying())
            {
                if (p->mediaObject()->isSeekable())
                    p->seek(0);
                p->play();
            }
        }
    }

} // PlayerService

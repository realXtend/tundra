// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
//#include "MemoryLeakCheck.h"

#include "Service.h"
#include "PhononPlayerModule.h" // needed ?
#include <QWidget>
#include <Phonon>

namespace PlayerService
{
    Service::Service()
    {
    }

    Service::~Service()
    {
        foreach(Phonon::VideoPlayer* player, video_players_)
        {
            player->deleteLater();
        }
        video_players_.clear();
    }

    
    bool Service::IsMimeTypeSupported(const QString mime_type)
    {
        return Phonon::BackendCapabilities::isMimeTypeAvailable(QString(mime_type));
    }

    QWidget* Service::GetPlayer(const QString &url)
    {
        if (video_players_.contains(url))
            return dynamic_cast<QWidget*>(video_players_[url]);
        Phonon::VideoPlayer* player = new Phonon::VideoPlayer(Phonon::VideoCategory, 0);
        /// @todo return 0 if cannot create the player
        video_players_[url] = player;
        QObject::connect(player, SIGNAL(finished()), this, SLOT(UpdatePlayers()));
        QObject::connect(player->mediaObject(), SIGNAL(stateChanged(Phonon::State, Phonon::State)), this, SLOT(UpdatePlayers()));
        QObject::connect(player, SIGNAL(destroyed()), this, SLOT(PlayerDestroyed()));

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
        if (player)
            delete player;
    }

    void Service::PlayerDestroyed()
    {
        // debug...
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

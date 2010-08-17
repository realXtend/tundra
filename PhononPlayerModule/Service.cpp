// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "Service.h"
#include <QWidget>
#include <Phonon/BackendCapabilities>
#include <QApplication>
#include "PhononPlayerModule.h"
#include "VideoPlayer.h"

#include "MemoryLeakCheck.h"

namespace PhononPlayer
{
    Service::Service()
    {
    }

    Service::~Service()
    {
        foreach(VideoPlayer* player, video_players_)
        {
            SAFE_DELETE(player);
        }
        video_players_.clear();
    }

    bool Service::IsMimeTypeSupported(const QString mime_type)
    {
        return Phonon::BackendCapabilities::isMimeTypeAvailable(QString(mime_type));
    }

    QWidget* Service::GetPlayerWidget(const QString &url)
    {
        if (video_players_.contains(url))
            return dynamic_cast<QWidget*>(video_players_[url]);

        QString message = QString("Created a player for %1").arg(url);
        PhononPlayerModule::LogDebug(message.toStdString());
        
        video_players_[url] = new VideoPlayer(url);

        return video_players_[url];
    }

    void Service::DeletePlayerWidget(const QString &url)
    {
        if (!video_players_.contains(url))
            return;
 
        VideoPlayer* player = video_players_[url];
        video_players_.remove(url);
        SAFE_DELETE(player);
    }

} // PhononPlayer

// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#include "MediaPlayerService.h"
#include <QMap>
#include <QWidget>

namespace PhononPlayer
{
    class VideoPlayer;

    class Service : public MediaPlayer::ServiceInterface
    {
        Q_OBJECT
    public:
        /// Default constructor
        Service();

        /// Default deconstructor
        virtual ~Service();

    public slots:
        virtual bool IsMimeTypeSupported(const QString &mime_type);
        virtual QWidget* GetPlayerWidget(const QString &url);
        virtual void DeletePlayerWidget(const QString &url);

    private:
        QMap<QString, VideoPlayer*> video_players_;
    };

} // PhononPlayer

// incl_PhononPlayer_Service_h

// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_PhononPlayer_Service_h
#define incl_PhononPlayer_Service_h

#include "PlayerService.h"
#include <QMap>
#include <QWidget>

namespace PlayerService
{
    class VideoPlayer;

    class Service : public Player::PlayerServiceInterface
    {
        Q_OBJECT
    public:
        /// Default constructor
        Service();

        /// Default deconstructor
        virtual ~Service();

        virtual bool IsMimeTypeSupported(const QString mime_type);
        virtual QWidget* GetPlayer(const QString &url);
        virtual void DeletePlayer(const QString &url);

    private:
        QMap<QString, VideoPlayer*> video_players_;
    };

} // PlayerService

#endif // incl_PhononPlayer_Service_h

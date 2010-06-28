// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_PhononPlayer_Service_h
#define incl_PhononPlayer_Service_h

#include "PlayerService.h"
#include <QMap>

class QWidget;
class QSignalMapper;

namespace Phonon
{
    class VideoPlayer;
}

namespace PlayerService
{
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

    private slots:
        /// Restart evey player object which is not playing state
        virtual void UpdatePlayers();

        virtual void PlayerDestroyed(const QString &);

    private:
        QMap<QString, Phonon::VideoPlayer*> video_players_;
        QSignalMapper* destroyed_signal_mapper_;
    };

} // PlayerService

#endif // incl_PhononPlayer_Service_h

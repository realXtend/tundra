// For conditions of distribution and use, see copyright notice in license.txt

/// @todo Rename to MediaPlayer ?
#ifndef incl_Interfaces_PlayerService_h
#define incl_Interfaces_PlayerService_h

#include "ServiceInterface.h"
// todo: Make these forward declarations if possible
#include <QObject>
#include <QString>

class QWidget;

namespace MediaPlayer
{
    /// Service offer player widget for video content.
    ///
    class ServiceInterface : public QObject, public Foundation::ServiceInterface 
    {
        Q_OBJECT
    public:
        virtual ~ServiceInterface() {};
    public slots:
        virtual bool IsMimeTypeSupported(const QString mime_type) = 0;

        /// Gives player widget playing given media in eternal loop.
        /// If player is asked more than once for same url only one instance of player 
        /// widget is created and returned.
        /// @param url url of video content
        /// @return pointer to player widget if success otherwise return 0.
        virtual QWidget* GetPlayerWidget(const QString &url) = 0;

        /// Delete existing video widget associated with given url.
        /// Be sure that you are not using the player widget after you have called this method.
        /// @param url url of video content
        virtual void DeletePlayerWidget(const QString &url) = 0;
    };
} // MediaPlayer

#endif // incl_Interfaces_PlayerService_h

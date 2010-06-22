// For conditions of distribution and use, see copyright notice in license.txt

/// @todo Rename to MediaPlayer ?
#ifndef incl_Interfaces_PlayerService_h
#define incl_Interfaces_PlayerService_h

#include "ServiceInterface.h"
// todo: Make these forward declarations if possible
#include <QObject>
#include <QString>

class QWidget;

namespace Player
{
    class PlayerServiceInterface : public QObject, public Foundation::ServiceInterface 
    {
        Q_OBJECT
    public:
        virtual ~PlayerServiceInterface() {};
        virtual bool IsMimeTypeSupported(const QString mime_type) = 0;
        virtual QWidget* GetPlayer(const QString &url) = 0;
        virtual void DeletePlayer(const QString &url) = 0;
    };
} // Player

#endif // incl_Interfaces_PlayerService_h

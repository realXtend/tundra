// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Communication_UiDefines_h
#define incl_Communication_UiDefines_h

#include <QObject>

namespace UiDefines
{
    class UiStates : public QObject
    {
    
    Q_OBJECT 
    
    public:
        enum ConnectionState
        {
            NoStateChange = 0,
            Disconnected = 1,
            Connecting = 2,
            Connected = 3,
            Exit = 4
        };
    
    };

    class PresenceStatus : public QObject
    {

    Q_OBJECT 
    
    public:
        enum PresenceState
        {
            NoStatus = 0,
            Available = 1,
            Away = 2,
            Busy = 3,
            Offline = 4
        };

    };
}
#endif // incl_Communication_UiDefines_h
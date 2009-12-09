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
            Connected = 3
        };
    
    };
}
#endif // incl_Communication_UiDefines_h
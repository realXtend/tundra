// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_UiDefines_h
#define incl_UiModule_UiDefines_h

#include <QString>
#include <QMap>

namespace UiServices
{
    enum ConnectionState
    {
        Connected,
        Disconnected,
        Failed,
        Kicked
    };

    enum ControlButtonType
    {
        Unknown,
        Ether,
        Build,
        Quit,
        Settings,
        Notifications,
        Teleport,
        Avatar
    };
}

#endif

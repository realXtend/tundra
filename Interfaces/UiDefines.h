// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_UiDefines_h
#define incl_Interfaces_UiDefines_h

#include <QString>
#include <QMap>

namespace UiDefines
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
        Teleport
    };

    enum MenuNodeStyle
    {
        IconNormal,
        IconHover,
        IconPressed
    };

    enum MenuGroup
    {
        NoGroup = 0,
        RootGroup = 1, 
        ServerToolsGroup = 2,
        WorldToolsGroup = 3
    };

    typedef QMap<UiDefines::MenuNodeStyle, QString> MenuNodeStyleMap;
}

#endif

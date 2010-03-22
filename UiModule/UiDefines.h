// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_UiDefines_h
#define incl_UiModule_UiDefines_h

namespace UiDefines
{
    enum ConnectionState 
    { 
        Connected, 
        Disconnected, 
        Failed 
    };

    enum ControlButtonType
    {
        Unknown,
        Ether,
        Quit,
        Settings,
        Notifications
    };
}

#endif
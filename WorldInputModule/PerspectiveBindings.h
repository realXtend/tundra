// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Input_KeyBindings_h
#define incl_Input_KeyBindings_h

#include <QKeySequence>

namespace Input
{
    // maps Qt::Keys to Input::Events
    typedef std::map <QKeySequence, std::pair <int,int> > KeyBindingMap;

    struct KeyBinding
    {
        KeyBinding ();
        KeyBindingMap map;
    };

    struct FirstPersonBindings : public KeyBinding
    {
        FirstPersonBindings ();
    };

    struct ThirdPersonBindings : public KeyBinding
    {
        ThirdPersonBindings ();
    };

    struct FreeCameraBindings : public KeyBinding
    {
        FreeCameraBindings ();
    };
}

#endif

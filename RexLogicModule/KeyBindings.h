// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexLogic_KeyBindings_h
#define incl_RexLogic_KeyBindings_h

// maps Qt::Keys to Input::Events
typedef std::map <int, int> KeyBindingMap;

namespace RexLogic
{
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

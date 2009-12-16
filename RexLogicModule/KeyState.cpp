// For conditions of distribution and use, see copyright notice in license.txt

#include "KeyState.h"

namespace RexLogic
{
    //=============================================================================
    // Keyboard State

    KeyInfo::KeyInfo () : 
        key (0), 
        modifiers (0)
    {
    }

    KeyInfo::KeyInfo (int k, unsigned int m) : 
        key (k), 
        modifiers (m)
    {
    }

    KeyInfo::KeyInfo (const KeyInfo &c) : 
        key (c.key), 
        modifiers (c.modifiers)
    {
    }

    KeyInfo::KeyInfo (const QKeyEvent *e) : 
        key (e-> key()), 
        modifiers (e-> modifiers())
    {
    }

    KeyInfo &KeyInfo::operator= (const KeyInfo &rhs) 
    { 
        key = rhs.key; 
        modifiers = rhs.modifiers; 
        return *this; 
    }

    bool KeyInfo::operator== (const KeyInfo &rhs) const
    {
        return (key == rhs.key);
    }

    bool KeyInfo::operator== (int k) const
    {
        return (key == k);
    }

    //=============================================================================
    // Current State of the Keyboard

    KeyStateListener::KeyStateListener (Foundation::EventManagerPtr eventmgr) :
        has_focus_ (false), eventmgr_ (eventmgr)
    {
        eid_ = eventmgr_-> QueryEventCategory ("Input");
    }

    bool KeyStateListener::IsKeyDown (const KeyInfo &key) 
    { 
        return is_pressed_ (key); 
    }

    bool KeyStateListener::IsKeyDown (int keycode, unsigned int modifiers)
    {
        KeyInfo key (keycode, modifiers);
        return is_pressed_ (key);
    }

    bool KeyStateListener::IsKeyDown (char c)
    {
        KeyInfo key (ascii_to_key_ (c));
        return is_pressed_ (key);
    }

    void KeyStateListener::Update (bool has_focus)
    {
        has_focus_ = has_focus;

        send_queued_pressed_events_ ();
        send_queued_released_events_ ();
        clear_completed_presses_ ();
    }

    bool KeyStateListener::eventFilter (QObject *obj, QEvent *event)
    {
        switch (event-> type())
        {
            case QEvent::KeyPress:
                {
                    QKeyEvent *e (static_cast <QKeyEvent *> (event));

                    if (has_focus_)
                    {
                        e-> accept();

                        if (!e-> isAutoRepeat())
                        {
                            KeyInfo key (e);
                            key_pressed_ (key);
                        }
                        
                        return true;
                    }
                }
                break;


            case QEvent::KeyRelease:
                {
                    QKeyEvent *e (static_cast <QKeyEvent *> (event));

                    if (has_focus_)
                    {
                        e-> accept();

                        if (!e-> isAutoRepeat())
                        {
                            KeyInfo key (e);
                            key_released_ (key);
                        }
                        
                        return true;
                    }
                }
        }
        
        return QObject::eventFilter (obj, event);
    }

    char KeyStateListener::key_to_ascii_ (const KeyInfo &k)
    {
        if ((k.key > 0x1f) && (k.key < 0x7f))
        { // ASCII
            char c (k.key);
            if ((c > 0x40) && (c < 0x5b)) // upper
                if (!(k.modifiers & 0x02000000))
                    c |= 0x20; // to lower
            return c;
        }
        else return 0;
    }

    KeyInfo KeyStateListener::ascii_to_key_ (char c)
    {
        KeyInfo k;
        if ((c > 0x1f) && (c < 0x7f))
        { // printable characters
            if ((c > 0x40) && (c < 0x5b)) // upper
                k.modifiers |= 0x02000000;

            if ((c > 0x61) && (c < 0x7b)) // lower
                c &= 0xdf; // to upper
        }
        return k;
    }

    bool KeyStateListener::is_pressed_ (const KeyInfo &k)
    {
        KeyInfoList::iterator i = std::find (pressed_.begin(), pressed_.end(), k);
        return i != pressed_.end();
    }

    void KeyStateListener::key_pressed_ (const KeyInfo &k)
    {
        KeyInfoList::iterator i = std::find (pressed_.begin(), pressed_.end(), k);
        if (i == pressed_.end()) pressed_.push_back (k);
    }

    void KeyStateListener::key_released_ (const KeyInfo &k)
    {
        KeyInfoList::iterator i = std::find (released_.begin(), released_.end(), k);
        if (i == released_.end()) released_.push_back (k);
    }

    int KeyStateListener::get_event_id_ (const KeyInfo &k)
    {
        KeyBindingMap::const_iterator i = binding_-> find (k.key);
        if (i != binding_-> end())
            return i-> second;
        else 
            return 0;
    }

    void KeyStateListener::send_queued_pressed_events_ ()
    {
        int event;
        KeyInfoList::const_iterator i = pressed_.begin();

        for (; i != pressed_.end(); ++i)
        {
            event = get_event_id_ (*i);
            if (event) eventmgr_-> SendEvent (eid_, event, 0);
        }
    }

    void KeyStateListener::send_queued_released_events_ ()
    {
        int event;
        KeyInfoList::const_iterator i = released_.begin();

        for (; i != released_.end(); ++i)
        {
            event = get_event_id_ (*i);
            if (event) eventmgr_-> SendEvent (eid_, event+1, 0);
        }
    }

    void KeyStateListener::clear_completed_presses_ ()
    {
        KeyInfoList::iterator i = released_.begin();
        KeyInfoList::iterator end = pressed_.end();

        for (; i != released_.end(); ++i)
            end = std::remove (pressed_.begin(), end, *i);

        pressed_.erase (end, pressed_.end());
        released_.erase (released_.begin(), released_.end());
    }
}


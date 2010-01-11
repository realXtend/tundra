// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexLogic_KeyState_h
#define incl_RexLogic_KeyState_h

//#include "StableHeaders.h"
#include "KeyBindings.h"

#include <QObject>
#include <QTime>
#include <QKeyEvent>

namespace RexLogic
{
    // Used for "unbuffered" keyboard state from "buffered" Qt Events
    // key codes and modifiers are from QKeyEvent
    struct KeyInfo
    {
        KeyInfo ();
        KeyInfo (int k, unsigned int m);
        KeyInfo (const KeyInfo &c);
        KeyInfo (const QKeyEvent *e);
        KeyInfo &operator= (const KeyInfo &rhs);
        bool operator== (const KeyInfo &rhs) const;
        bool operator== (int k) const;

        int key; // Qt::Key codes
        unsigned int modifiers; // modifiers
    };

    // Install this even filter on the root window (QGraphicsView)
    // to capture all Qt keyboard events and translate to internal events
    class KeyStateListener : public QObject
    {
        public:
            typedef std::vector <KeyInfo> KeyInfoList;

            KeyStateListener (Foundation::EventManagerPtr eventmgr);

            void SetKeyBindings (const KeyBindingMap &bindings) { binding_ = &bindings; }

            bool IsKeyDown (const KeyInfo &key);
            bool IsKeyDown (int key, unsigned int modifiers = 0);
            bool IsKeyDown (char c);
            
            void Update (bool has_focus = false);

        protected:
            bool eventFilter (QObject *obj, QEvent *event);

        private:
            char key_to_ascii_ (const KeyInfo &k);
            KeyInfo ascii_to_key_ (char c);

        private:
            bool is_pressed_ (const KeyInfo &k);
            void key_pressed_ (const KeyInfo &k);
            void key_released_ (const KeyInfo &k);

            int get_event_id_ (const KeyInfo &k);
            void send_queued_pressed_events_ ();
            void send_queued_released_events_ ();
            void clear_completed_presses_ ();

        private:
            bool                    has_focus_;

            KeyInfoList             pressed_;
            KeyInfoList             released_;

            const KeyBindingMap     *binding_;

            event_category_id_t     eid_;
            Foundation::EventManagerPtr   eventmgr_;
    };
}

#endif

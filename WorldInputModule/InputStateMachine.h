// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Input_InputStateMachine_h
#define incl_Input_InputStateMachine_h

#include "State.h"
#include "InputEvents.h"
#include "KeyBindings.h"
#include "InputServiceInterface.h"

#include <QGraphicsView>

#include <QState>
#include <QFinalState>
#include <QEventTransition>
#include <QStateMachine>

#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>

#ifdef Q_WS_X11
#include <QX11Info>

extern "C" 
{
    typedef struct {
        int key_click_percent;
        int bell_percent;
        unsigned int bell_pitch, bell_duration;
        unsigned long led_mask;
        int global_auto_repeat;
        char auto_repeats[32];
    } XKeyboardState;

    extern int XGetKeyboardControl(Display*,XKeyboardState*);
    extern int XAutoRepeatOff(Display*);
    extern int XAutoRepeatOn(Display*);
}

#define AutoRepeatModeOn	1
#endif

namespace Foundation
{
    class EventManager;
    class Framework;
}

namespace Input
{
    struct KeyInfo;
    struct MouseInfo;
    struct DragInfo;
    struct KeyState;

    // list of state structure codes
    typedef std::vector <KeyState *> KeyStateList;

    // maps Qt::Key codes to info and state structures
    typedef std::map <QKeySequence, KeyState *> KeyStateMap;

    struct MouseInfo
    {
        MouseInfo ();
        MouseInfo (const MouseInfo &m);
        MouseInfo (const QMouseEvent *e);
        MouseInfo (const QWheelEvent *e);
        MouseInfo &operator= (const MouseInfo &m);
        MouseInfo &operator= (const QMouseEvent *e);
        MouseInfo &operator= (const QWheelEvent *e);

        int buttons;
        int wheel_delta;
        int wheel_orientation;
        int global_x;
        int global_y;
        int x;
        int y;
    };

    typedef std::list <MouseInfo> DragList;

    struct GestureInfo
    {
        //DragList drag;
        int last_x;
        int last_y;
    };

    struct KeyBindingInfo
    {
        KeyBindingInfo (QString g, QString s, event_id_t n, event_id_t x)
            : group (g), sequence (s), enter (n), exit (x)
        {}

        QString group;
        QString sequence;
        event_id_t enter;
        event_id_t exit;

        private:
        KeyBindingInfo ();

    };

    typedef std::list <KeyBindingInfo> KeyBindingInfoList;

    struct InputState : public Foundation::State
    {
        InputState (QString name, QState *parent = 0);
        InputState (QString name, QState::ChildMode mode, QState *parent = 0);
    };

    struct KeyState : public InputState
    {
        KeyState (const QKeySequence &s, QState *p = 0);
        KeyState (const QKeySequence &s, KeyBindingMap **b, Foundation::EventManager* m, QState *p = 0);
        ~KeyState ();

        void onEntry (QEvent *e);
        void onExit (QEvent *event);

        std::pair <int,int> get_event_ids ();

        bool operator== (const KeyState &rhs);

        QKeySequence                sequence;
        KeyBindingMap               **bindings;

        event_category_id_t         catid;
        Foundation::EventManager*   eventmgr;
    };

    struct InputActiveState : public InputState
    {
        InputActiveState (QString name, QGraphicsView *v, QState::ChildMode m, QState *p = 0);

        void onEntry (QEvent *event);
        void onExit (QEvent *event);

        QGraphicsView   *view;
#ifdef Q_WS_X11
        XKeyboardState  x11_key_state;
#endif
    };

    struct KeyboardActiveState : public InputState
    {
        KeyboardActiveState (QString name, QState::ChildMode m, QState *p = 0);

        void onEntry (QEvent *event);
        void onExit (QEvent *event);

        KeyStateList    active;
    };

    struct MouseActiveState : public InputState
    {
        MouseActiveState (QString name, QGraphicsScene *s, QState::ChildMode m, QState *p = 0);

        QGraphicsScene  *scene;
    };

    struct LeftButtonActiveState : public InputState
    {
        LeftButtonActiveState (QString name, Foundation::EventManager* m, QState *p = 0);

        void onEntry (QEvent *event);
        void onExit (QEvent *event);

        Input::Events::Movement     movement;

        event_category_id_t         catid;
        Foundation::EventManager*   eventmgr;
    };

    struct RightButtonActiveState : public InputState
    {
        RightButtonActiveState (QString name, Foundation::EventManager* m, QState *p = 0);

        void onEntry (QEvent *event);
        void onExit (QEvent *event);

        Input::Events::Movement     movement;

        event_category_id_t         catid;
        Foundation::EventManager*   eventmgr;
    };

    struct MidButtonActiveState : public InputState
    {
        MidButtonActiveState (QString name, Foundation::EventManager* m, QState *p = 0);

        void onEntry (QEvent *event);
        void onExit (QEvent *event);

        Input::Events::Movement     movement;

        event_category_id_t         catid;
        Foundation::EventManager*   eventmgr;
    };

    struct WheelActiveState : public InputState
    {
        WheelActiveState (QString name, Foundation::EventManager* m, QState *p = 0);

        void onEntry (QEvent *event);
        void onExit (QEvent *event);

        Input::Events::SingleAxisMovement   scroll;

        event_category_id_t         catid;
        Foundation::EventManager*   eventmgr;
    };

    struct GestureActiveState : public InputState
    {
        GestureActiveState (QString name, GestureInfo &g, Foundation::EventManager* m, QState *p = 0);

        void onEntry (QEvent *event);
        void onExit (QEvent *event);

        GestureInfo &gesture;

        Input::Events::Movement     movement;

        event_category_id_t         catid;
        Foundation::EventManager*   eventmgr;
    };

    struct KeyBindingActiveState : public InputState
    {
        KeyBindingActiveState (QString name, KeyBindingMap **m, QState *p = 0);
        KeyBindingMap   **map;
    };

    struct FirstPersonActiveState : public KeyBindingActiveState
    {
        FirstPersonActiveState (QString name, KeyBindingMap **m, QState *p = 0);

        void onEntry (QEvent *event);

        FirstPersonBindings bindings;
    };

    struct ThirdPersonActiveState : public KeyBindingActiveState
    {
        ThirdPersonActiveState (QString name, KeyBindingMap **m, QState *p = 0);

        void onEntry (QEvent *event);

        ThirdPersonBindings bindings;
    };

    struct FreeCameraActiveState : public KeyBindingActiveState
    {
        FreeCameraActiveState (QString name, KeyBindingMap **m, QState *p = 0);

        void onEntry (QEvent *event);

        FreeCameraBindings  bindings;
    };

    enum InputEventType
    {
        FirstPersonEventType = QEvent::User,
        ThirdPersonEventType,
        FreeCameraEventType
    };

    struct FirstPersonEvent : public QEvent
    {
        FirstPersonEvent () : QEvent ((QEvent::Type) FirstPersonEventType) {}
    };

    struct ThirdPersonEvent : public QEvent
    {
        ThirdPersonEvent () : QEvent ((QEvent::Type) ThirdPersonEventType) {}
    };

    struct FreeCameraEvent : public QEvent
    {
        FreeCameraEvent () : QEvent ((QEvent::Type) FreeCameraEventType) {}
    };

    struct UnconditionalTransition : public QAbstractTransition
    {
        UnconditionalTransition (QState *p = 0) : QAbstractTransition (p) {}

        bool eventTest (QEvent *event) { return true; }
        void onTransition (QEvent *event) {}
    };

    template <int EventType>
        struct EventTransition : public QAbstractTransition
    {
        EventTransition (QState *p = 0) : QAbstractTransition (p) {}

        virtual bool eventTest (QEvent *event) { return (event-> type() == EventType); }
        virtual void onTransition (QEvent *event) {}
    };

    struct LeftButtonPress : public EventTransition <QEvent::MouseButtonPress>
    {
        LeftButtonPress (QState *p = 0) : EventTransition <QEvent::MouseButtonPress> (p) {}

        bool eventTest (QEvent *e) 
        { 
            return (e-> type() == QEvent::MouseButtonPress) && 
                (static_cast <QMouseEvent *> (e)-> button() & Qt::LeftButton);
        }
    };

    struct RightButtonPress : public EventTransition <QEvent::MouseButtonPress>
    {
        RightButtonPress (QState *p = 0) : EventTransition <QEvent::MouseButtonPress> (p) {}

        bool eventTest (QEvent *e) 
        { 
            return (e-> type() == QEvent::MouseButtonPress) && 
                (static_cast <QMouseEvent *> (e)-> button() & Qt::RightButton);
        }
    };

    struct MidButtonPress : public EventTransition <QEvent::MouseButtonPress>
    {
        MidButtonPress (QState *p = 0) : EventTransition <QEvent::MouseButtonPress> (p) {}

        bool eventTest (QEvent *e) 
        { 
            return (e-> type() == QEvent::MouseButtonPress) && 
                (static_cast <QMouseEvent *> (e)-> button() & Qt::MidButton);
        }
    };

    struct LeftButtonRelease : public EventTransition <QEvent::MouseButtonRelease>
    {
        LeftButtonRelease (QState *p = 0) : EventTransition <QEvent::MouseButtonRelease> (p) {}

        bool eventTest (QEvent *e) 
        { 
            return (e-> type() == QEvent::MouseButtonRelease) && 
                (static_cast <QMouseEvent *> (e)-> button() & Qt::LeftButton);
        }
    };

    struct RightButtonRelease : public EventTransition <QEvent::MouseButtonRelease>
    {
        RightButtonRelease (QState *p = 0) : EventTransition <QEvent::MouseButtonRelease> (p) {}

        bool eventTest (QEvent *e) 
        { 
            return (e-> type() == QEvent::MouseButtonRelease) && 
                (static_cast <QMouseEvent *> (e)-> button() & Qt::RightButton);
        }
    };

    struct MidButtonRelease : public EventTransition <QEvent::MouseButtonRelease>
    {
        MidButtonRelease (QState *p = 0) : EventTransition <QEvent::MouseButtonRelease> (p) {}

        bool eventTest (QEvent *e) 
        { 
            return (e-> type() == QEvent::MouseButtonRelease) && 
                (static_cast <QMouseEvent *> (e)-> button() & Qt::MidButton);
        }
    };

    struct GestureActive : public EventTransition <QEvent::MouseMove>
    {
        GestureActive (GestureInfo &g, Foundation::EventManager* m, QState *p = 0);

        void onTransition (QEvent *e);

        GestureInfo &gesture;

        Input::Events::Movement     movement;

        event_category_id_t         catid;
        Foundation::EventManager*   eventmgr;
    };

    struct KeyListener : public QAbstractTransition 
    {
        KeyListener (KeyStateMap &s, KeyBindingMap **b, Foundation::EventManager* m, QState *p = 0);

        bool eventTest (QEvent *event);
        void onTransition (QEvent *event);

        KeyState *get_key_state (const QKeySequence &s);

        void press_active (KeyState *e);
        void release_active (KeyState *e);

        KeyboardActiveState *parent;

        KeyStateMap         &key_states;
        KeyBindingMap       **bindings;

        Foundation::EventManager* eventmgr;
    };


    class WorldInputLogic : public QStateMachine, public Foundation::InputServiceInterface
    {
        public:
            explicit WorldInputLogic (Foundation::Framework *fw);

            void Update (f64 frametime);

            Foundation::State *GetState (QString name);

            void AddKeyEvent (QString group, QString key_sequence, event_id_t enter, event_id_t exit);

        protected:
            bool eventFilter (QObject *obj, QEvent *event);

        private:
            void init_statemachine_ ();

            QEvent *clone_event_ (QEvent *event);

            void post_simulated_focus_click ();

            void update_dynamic_key_bindings_ ();

        private:
            Foundation::Framework       *framework_;
            Foundation::EventManager    *eventmgr_;

            QGraphicsView   *view_;
            bool            has_focus_;

            MouseInfo       mouse_state_;
            GestureInfo     gesture_state_;

            KeyStateMap     key_states_;
            KeyBindingMap   *key_binding_;
    };
}

#endif

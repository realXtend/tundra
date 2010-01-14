// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexLogicInputStateMachine_h
#define incl_RexLogicInputStateMachine_h

#include "InputEvents.h"
#include "KeyBindings.h"

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

namespace RexLogic
{

    struct KeyInfo;
    struct MouseInfo;
    struct DragInfo;
    struct KeyState;

    // maps Qt::Key codes to info and state structures
    typedef std::map <int, KeyState *> KeyStateMap;

    // maps Qt::Key codes to Naali events
    typedef std::map <int, int> KeyEventMap;

    
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
        DragList drag;
    };


    struct State : public QState
    {
        State (QString name, QState *parent = 0);
        State (QString name, QState::ChildMode mode, QState *parent = 0);

        virtual void onEntry (QEvent *e);
        virtual void onExit (QEvent *e);
    };

    struct FinalState : public QFinalState
    {
        FinalState (QString name, QState *p = 0);

        virtual void onEntry (QEvent *e);
        virtual void onExit (QEvent *e);
    };


    struct KeyState : public State
    {
        KeyState (QKeyEvent *event, QState *p = 0);
        KeyState (QKeyEvent *event, KeyEventMap **b, Foundation::EventManagerPtr m, QState *p = 0);
        ~KeyState ();

        void onEntry (QEvent *event);
        void onExit (QEvent *event);

        int get_event_id ();

        bool operator== (const KeyState &rhs);

        bool                        active;
        std::auto_ptr <QKeyEvent>   event;
        
        KeyEventMap                 **bindings;

        event_category_id_t         catid;
        Foundation::EventManagerPtr eventmgr;
    };


    struct InputActiveState : public State
    {
        InputActiveState (QString name, QGraphicsView *v, QState::ChildMode m, QState *p = 0);

        void onEntry (QEvent *e);
        void onExit (QEvent *e);

        QGraphicsView   *view;
#ifdef Q_WS_X11
        XKeyboardState  x11_key_state;
#endif
    };

    
    struct ButtonActiveState : public State
    {
        ButtonActiveState (QString name, MouseInfo &m, QState *p = 0);

        void onEntry (QEvent *event);
        void onExit (QEvent *event);

        MouseInfo   &mouse;
    };


    struct LeftButtonActiveState : public State
    {
        LeftButtonActiveState (QString name, Foundation::EventManagerPtr m, QState *p = 0);

        void onEntry (QEvent *event);
        void onExit (QEvent *event);
    
        Input::Events::Movement     movement;

        event_category_id_t         catid;
        Foundation::EventManagerPtr eventmgr;
    };


    struct RightButtonActiveState : public State
    {
        RightButtonActiveState (QString name, Foundation::EventManagerPtr m, QState *p = 0);

        void onEntry (QEvent *event);
        void onExit (QEvent *event);

        Input::Events::Movement     movement;

        event_category_id_t         catid;
        Foundation::EventManagerPtr eventmgr;
    };


    struct MidButtonActiveState : public State
    {
        MidButtonActiveState (QString name, Foundation::EventManagerPtr m, QState *p = 0);

        void onEntry (QEvent *event);
        void onExit (QEvent *event);
        
        Input::Events::Movement     movement;

        event_category_id_t         catid;
        Foundation::EventManagerPtr eventmgr;
    };


    struct WheelActiveState : public State
    {
        WheelActiveState (QString name, Foundation::EventManagerPtr m, QState *p = 0);

        void onEntry (QEvent *event);
        void onExit (QEvent *event);

        Input::Events::SingleAxisMovement   scroll;

        event_category_id_t         catid;
        Foundation::EventManagerPtr eventmgr;
    };


    struct GestureActiveState : public State
    {
        GestureActiveState (QString name, GestureInfo &g, Foundation::EventManagerPtr m, QState *p = 0);

        void onEntry (QEvent *event);
        void onExit (QEvent *event);

        GestureInfo &gesture;
        
        Input::Events::Movement     movement;

        event_category_id_t         catid;
        Foundation::EventManagerPtr eventmgr;
    };


    struct FirstPersonActiveState : public State
    {
        FirstPersonActiveState (QString name, KeyEventMap **m, QState *p = 0);

        void onEntry (QEvent *event);

        KeyEventMap         **map;
        FirstPersonBindings bindings;
    };


    struct ThirdPersonActiveState : public State
    {
        ThirdPersonActiveState (QString name, KeyEventMap **m, QState *p = 0);

        void onEntry (QEvent *event);

        KeyEventMap         **map;
        ThirdPersonBindings bindings;
    };


    struct FreeCameraActiveState : public State
    {
        FreeCameraActiveState (QString name, KeyEventMap **m, QState *p = 0);

        void onEntry (QEvent *event);

        KeyEventMap         **map;
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
        GestureActive (GestureInfo &g, Foundation::EventManagerPtr m, QState *p = 0);

        void onTransition (QEvent *e);

        GestureInfo &gesture;
        
        Input::Events::Movement     movement;
        
        event_category_id_t         catid;
        Foundation::EventManagerPtr eventmgr;
    };
    
    struct KeyListener : public QAbstractTransition 
    {
        KeyListener (KeyStateMap &s, KeyEventMap **b, Foundation::EventManagerPtr m, QState *p = 0);

        bool eventTest (QEvent *event);
        void onTransition (QEvent *event);

        KeyState *get_key_state (QKeyEvent *e);

        KeyStateMap &key_states;
        KeyEventMap **bindings;

        Foundation::EventManagerPtr eventmgr;
    };

    
    class WorldInputLogic : public QStateMachine
    {
        public:
            WorldInputLogic (Foundation::Framework *fw);

            void Update (f64 frametime);

        protected:
            bool eventFilter (QObject *obj, QEvent *event);

        private:
            void init_statemachine_ ();

            QEvent *clone_event_ (QEvent *event);

        private:
            Foundation::Framework       *framework_;
            Foundation::EventManagerPtr eventmgr_;

            QGraphicsView   *view_;
            bool            has_focus_;

            MouseInfo       mouse_state_;
            GestureInfo     gesture_state_;

            KeyStateMap     key_states_;
            KeyEventMap     *key_binding_;
    };
}

#endif

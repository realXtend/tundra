
#include "StableHeaders.h"
#include "InputStateMachine.h"

namespace RexLogic
{
    //=========================================================================
    //
    MouseInfo::MouseInfo () 
        : buttons (0), 
        wheel_delta (0), wheel_orientation (0), 
        global_x (0), global_y (0), x (0), y (0) 
    {}

    MouseInfo::MouseInfo (const MouseInfo &rhs) 
        : buttons (rhs.buttons), 
        wheel_delta (rhs.wheel_delta), wheel_orientation (rhs.wheel_orientation),
        global_x (rhs.global_x), global_y (rhs.global_y), x (rhs.x), y (rhs.y) 
    {}

    MouseInfo::MouseInfo (const QMouseEvent *e) 
        : buttons (e->buttons()), 
        wheel_delta (0), wheel_orientation (0),
        global_x (e->globalX()), global_y (e->globalY()), x (e->x()), y (e->y()) 
    {}

    MouseInfo::MouseInfo (const QWheelEvent *e) 
        : buttons (e->buttons()), 
        wheel_delta (e->delta()), wheel_orientation (e->orientation()),
        global_x (e->globalX()), global_y (e->globalY()), x (e->x()), y (e->y()) 
    {}

    MouseInfo &MouseInfo::operator= (const MouseInfo &rhs) 
    {
        buttons = rhs.buttons;
        wheel_delta = rhs.wheel_delta; wheel_orientation = rhs.wheel_orientation;
        global_x = rhs.global_x; global_y = rhs.global_y; x = rhs.x; y = rhs.y;
		return *this;
    }

    MouseInfo &MouseInfo::operator= (const QMouseEvent *e)
    {
        buttons = e-> buttons();
        global_x = e-> globalX(); global_y = e-> globalY(); x = e-> x(); y = e-> y();
		return *this;
    }

    MouseInfo &MouseInfo::operator= (const QWheelEvent *e)
    {
        buttons = e-> buttons();
        wheel_delta = e-> delta(); wheel_orientation = e-> orientation();
        global_x = e-> globalX(); global_y = e-> globalY(); x = e-> x(); y = e-> y();
		return *this;
    }

    //=========================================================================
    //
    State::State (QString name, QState *parent) 
        : QState (parent) 
    { 
        setObjectName (name); 
    }

    State::State (QString name, QState::ChildMode mode, QState *parent) 
        : QState (mode, parent) 
    { 
        setObjectName (name); 
    }

    void State::onEntry (QEvent *e)
    {
        std::cout << "State::onEntry: " << qPrintable (objectName()) << std::endl;
    }

    void State::onExit (QEvent *e)
    {
        std::cout << "State::onExit: " << qPrintable (objectName()) << std::endl;
    }

    //=========================================================================
    //
    FinalState::FinalState (QString name, QState *p) 
        : QFinalState (p) 
    { 
        setObjectName (name); 
    }

    void FinalState::onEntry (QEvent *e)
    {
        QFinalState::onEntry (e);
        std::cout << "FinalState::onEntry: " << qPrintable (objectName()) << std::endl;
    }

    void FinalState::onExit (QEvent *e)
    {
        std::cout << "FinalState::onExit: " << qPrintable (objectName()) << std::endl;
        QFinalState::onExit (e);
    }

    //=========================================================================
    //
    KeyState::KeyState (QKeyEvent *e, QState *p)
        : State ("KeyState: "+e->text(), p), 
        active (false), 
        event (new QKeyEvent (*e)), 
        bindings (0)
    {
        catid = eventmgr-> QueryEventCategory ("Input");
    }

    KeyState::KeyState (QKeyEvent *e, KeyEventMap **b, Foundation::EventManagerPtr m, QState *p)
        : State ("KeyState: "+e->text(), p), 
        active (false), 
        event (new QKeyEvent (*e)), 
        bindings (b), 
        eventmgr (m)
    {
        catid = eventmgr-> QueryEventCategory ("Input");
    }

    KeyState::~KeyState ()
    {
    }

    void KeyState::onEntry (QEvent *event)
    {
        int eid;

        active = true;
        if (bindings && (eid = get_event_id ()))
            eventmgr-> SendEvent (catid, eid, 0);

        State::onEntry (event);
    }

    void KeyState::onExit (QEvent *event)
    {
        int eid;

        active = false;
        if (bindings && (eid = get_event_id ()))
            eventmgr-> SendEvent (catid, eid+1, 0);

        State::onExit (event);
    }

    int KeyState::get_event_id ()
    {
        KeyEventMap::const_iterator i = (*bindings)-> find (event-> key());
        KeyEventMap::const_iterator e = (*bindings)-> end ();
        return (i != e)? i-> second : 0;
    }
        
    bool KeyState::operator== (const KeyState &rhs)
    {
        return event-> key() == rhs.event-> key();
    }

    
    //=========================================================================
    //
    LeftButtonActiveState::LeftButtonActiveState (QString name, Foundation::EventManagerPtr m, QState *p)
        : State (name, p), eventmgr (m)
    {
        catid = eventmgr-> QueryEventCategory ("Input");
    }

    void LeftButtonActiveState::onEntry (QEvent *event)
    {
        State::onEntry (event);

        QMouseEvent *e = static_cast <QMouseEvent *> (event);
        movement.x_.rel_ = e-> x();
        movement.y_.rel_ = e-> y();
        movement.x_.abs_ = e-> globalX();
        movement.y_.abs_ = e-> globalY();

        eventmgr-> SendEvent (catid, Input::Events::INWORLD_CLICK, &movement);
        eventmgr-> SendEvent (catid, Input::Events::LEFT_MOUSECLICK_PRESSED, &movement);
    }
    
    void LeftButtonActiveState::onExit (QEvent *event)
    {
        QMouseEvent *e = static_cast <QMouseEvent *> (event);
        movement.x_.rel_ = e-> x();
        movement.y_.rel_ = e-> y();
        movement.x_.abs_ = e-> globalX();
        movement.y_.abs_ = e-> globalY();

        eventmgr-> SendEvent (catid, Input::Events::LEFT_MOUSECLICK_RELEASED, &movement);

        State::onExit (event);
    }

    //=========================================================================
    //
    RightButtonActiveState::RightButtonActiveState (QString name, Foundation::EventManagerPtr m, QState *p)
        : State (name, p), eventmgr (m)
    {
        catid = eventmgr-> QueryEventCategory ("Input");
    }

    void RightButtonActiveState::onEntry (QEvent *event)
    {
        State::onEntry (event);

        QMouseEvent *e = static_cast <QMouseEvent *> (event);
        movement.x_.rel_ = e-> x();
        movement.y_.rel_ = e-> y();
        movement.x_.abs_ = e-> globalX();
        movement.y_.abs_ = e-> globalY();

        eventmgr-> SendEvent (catid, Input::Events::RIGHT_MOUSECLICK_PRESSED, &movement);
    }
    
    void RightButtonActiveState::onExit (QEvent *event)
    {
        eventmgr-> SendEvent (catid, Input::Events::RIGHT_MOUSECLICK_RELEASED, &movement);

        QMouseEvent *e = static_cast <QMouseEvent *> (event);
        movement.x_.rel_ = e-> x();
        movement.y_.rel_ = e-> y();
        movement.x_.abs_ = e-> globalX();
        movement.y_.abs_ = e-> globalY();

        State::onExit (event);
    }

    //=========================================================================
    //
    MidButtonActiveState::MidButtonActiveState (QString name, Foundation::EventManagerPtr m, QState *p)
        : State (name, p), eventmgr (m)
    {
        catid = eventmgr-> QueryEventCategory ("Input");
    }

    void MidButtonActiveState::onEntry (QEvent *event)
    {
        State::onEntry (event);
    }
    
    void MidButtonActiveState::onExit (QEvent *event)
    {
        State::onExit (event);
    }

    //=========================================================================
    //
    WheelActiveState::WheelActiveState (QString name, Foundation::EventManagerPtr m, QState *p)
        : State (name, p), eventmgr (m)
    {
        catid = eventmgr-> QueryEventCategory ("Input");
    }

    void WheelActiveState::onEntry (QEvent *event)
    {
        State::onEntry (event);

        QWheelEvent *e = static_cast <QWheelEvent *> (event);

        scroll.z_.rel_ = e-> delta();
        scroll.z_.abs_ = e-> delta();

        eventmgr-> SendEvent (catid, Input::Events::SCROLL, &scroll);
    }
    
    void WheelActiveState::onExit (QEvent *event)
    {
        State::onExit (event);
    }


    //=========================================================================
    //
    InputActiveState::InputActiveState (QString name, QGraphicsView *v, QState::ChildMode m, QState *p) 
        : State (name, m, p), view (v)
    { 
#ifdef Q_WS_X11
        XGetKeyboardControl (view-> x11Info().display(), &x11_key_state);
#endif
    }

    void InputActiveState::onEntry (QEvent *e)
    {
        State::onEntry (e);
#ifdef Q_WS_X11
        if (x11_key_state.global_auto_repeat == AutoRepeatModeOn)
            XAutoRepeatOff (view-> x11Info().display());
#endif
    }

    void InputActiveState::onExit (QEvent *e)
    {
#ifdef Q_WS_X11
        //if (x11_key_state.global_auto_repeat == AutoRepeatModeOn)
        XAutoRepeatOn (view-> x11Info().display());
#endif
        State::onExit (e);
    }

    QGraphicsView   *view;
#ifdef Q_WS_X11
    XKeyboardState  x11_key_state;
#endif


    //=========================================================================
    //
    ButtonActiveState::ButtonActiveState (QString name, MouseInfo &m, QState *p)
        : State (name, p), mouse (m) 
    {}

    void ButtonActiveState::onEntry (QEvent *event)
    {
        State::onEntry (event);
        
        QMouseEvent *e = static_cast <QMouseEvent *> (event);
        mouse = e;
    }

    void ButtonActiveState::onExit (QEvent *event)
    {
        QMouseEvent *e = static_cast <QMouseEvent *> (event);
        mouse = e;

        QState::onExit (event);
    }

    //=========================================================================
    //
    GestureActiveState::GestureActiveState (QString name, GestureInfo &g, Foundation::EventManagerPtr m, QState *p)
        : State (name, p), gesture (g), eventmgr (m)
    {
        catid = eventmgr-> QueryEventCategory ("Input");
    }

    void GestureActiveState::onEntry (QEvent *event)
    {
        State::onEntry (event);

        //MouseInfo info (static_cast <QMouseEvent *> (event));
        //gesture.drag.push_back (info);
        
        QMouseEvent *e = static_cast <QMouseEvent *> (event);
        movement.x_.rel_ = e-> x();
        movement.y_.rel_ = e-> y();
        movement.x_.abs_ = e-> globalX();
        movement.y_.abs_ = e-> globalY();

        eventmgr-> SendEvent (catid, Input::Events::MOUSELOOK, &movement);
    }

    void GestureActiveState::onExit (QEvent *event)
    {
        //MouseInfo info (static_cast <QMouseEvent *> (event));
        //gesture.drag.push_back (info);

        eventmgr-> SendEvent (catid, Input::Events::MOUSELOOK_STOPPED, 0);

        QState::onExit (event);
    }

    //=========================================================================
    //
    FirstPersonActiveState::FirstPersonActiveState (QString name, KeyEventMap **s, QState *p)
        : State (name, p), map (s)
    {}

    void FirstPersonActiveState::onEntry (QEvent *event)
    {
        State::onEntry (event);
        *map = &bindings.map;
    }
    
    //=========================================================================
    //
    ThirdPersonActiveState::ThirdPersonActiveState (QString name, KeyEventMap **s, QState *p)
        : State (name, p), map (s)
    {}

    void ThirdPersonActiveState::onEntry (QEvent *event)
    {
        State::onEntry (event);
        *map = &bindings.map;
    }
    
    //=========================================================================
    //
    FreeCameraActiveState::FreeCameraActiveState (QString name, KeyEventMap **s, QState *p)
        : State (name, p), map (s)
    {}

    void FreeCameraActiveState::onEntry (QEvent *event)
    {
        State::onEntry (event);
        *map = &bindings.map;
    }
    
    //=========================================================================
    //
    GestureActive::GestureActive (GestureInfo &g, Foundation::EventManagerPtr m, QState *p)
        : EventTransition <QEvent::MouseMove> (p), gesture (g), eventmgr (m)
    {
        catid = eventmgr-> QueryEventCategory ("Input");
    }

    void GestureActive::onTransition (QEvent *event)
    {
        //MouseInfo info (static_cast <QMouseEvent *> (event)); 
        //gesture.drag.push_back (info);
        
        QMouseEvent *e = static_cast <QMouseEvent *> (event);
        movement.x_.rel_ = e-> x();
        movement.y_.rel_ = e-> y();
        movement.x_.abs_ = e-> globalX();
        movement.y_.abs_ = e-> globalY();

        eventmgr-> SendEvent (catid, Input::Events::MOUSELOOK, &movement);
    }
    
    //=========================================================================
    //
    
    KeyListener::KeyListener (KeyStateMap &s, KeyEventMap **b, Foundation::EventManagerPtr m, QState *p)
        : QAbstractTransition (p), key_states (s), bindings (b), eventmgr (m)
    {
        setTargetState (0);
    }

    bool KeyListener::eventTest (QEvent *event)
    {
        // this is preferrable to 3 states + 2 transitions for each key
        // plus a look-up tree if you want to discover the current state any way 
        
        QKeyEvent *e; KeyState *s;
        switch (event-> type())
        {
            case QEvent::KeyPress:
                e = static_cast <QKeyEvent *> (event);
                s = get_key_state (e);
                s-> active = true;
                s-> onEntry (e);
                return true;

            case QEvent::KeyRelease:
                e = static_cast <QKeyEvent *> (event);
                s = get_key_state (e);
                s-> active = false;
                s-> onExit (e);
                return true;
        }

        return false;
    }

    void KeyListener::onTransition (QEvent *event) 
    {
    }

    KeyState *KeyListener::get_key_state (QKeyEvent *event)
    {
        int code = event-> key();

        KeyState *state;
        KeyStateMap::const_iterator i = key_states.find (code);
        KeyStateMap::const_iterator e = key_states.end ();

        if (i == e) 
        {
            state = new KeyState (event, bindings, eventmgr);
            key_states.insert (std::make_pair (code, state));
        }
        else
            state = i-> second;

        return state;
    }

    //=========================================================================
    //
    WorldInputLogic::WorldInputLogic (Foundation::Framework *fw)
        : framework_ (fw), 
        view_ (framework_-> GetUIView()), 
        eventmgr_ (framework_-> GetEventManager()),
        has_focus_ (false), 
        key_binding_ (0)
    {
        init_statemachine_();

        view_-> installEventFilter (this);
        view_-> viewport()-> installEventFilter (this);
    }


    bool WorldInputLogic::eventFilter (QObject *obj, QEvent *event)
    {
        switch (event-> type())
        {
            // route select Qt events to the state machine
            case QEvent::Close:
            case QEvent::MouseMove:
            case QEvent::MouseButtonPress:
            case QEvent::MouseButtonRelease:
            case QEvent::Wheel:
            case QEvent::KeyPress:
            case QEvent::KeyRelease:
                postEvent (clone_event_ (event));
                break;
        }

        return QObject::eventFilter (obj, event);
    }

    void WorldInputLogic::Update (f64 frametime)
    {
        if (view_-> scene()-> focusItem())
        {
            if (has_focus_)
            {
                postEvent (new QFocusEvent (QEvent::FocusOut));
                has_focus_ = false;
            }
        }
        else 
        {
            if (!has_focus_)
            {
                postEvent (new QFocusEvent (QEvent::FocusIn));
                has_focus_ = true;
            }
        }
    }

    void WorldInputLogic::init_statemachine_ ()
    {
        FinalState *exit;

        State *active, *unfocused, 
              *mouse, *keyboard, *perspective, 
              *wheel, *wheel_waiting,
              *left_button, *right_button, *mid_button,
              *left_button_waiting, *right_button_waiting, *mid_button_waiting,
              *button, *button_waiting, *gesture_waiting;

        InputActiveState        *focused;
        ButtonActiveState       *button_active;
        WheelActiveState        *wheel_active;
        LeftButtonActiveState   *left_button_active;
        RightButtonActiveState  *right_button_active;
        MidButtonActiveState    *mid_button_active;
        GestureActiveState      *gesture_active;

        FirstPersonActiveState  *first_person;
        ThirdPersonActiveState  *third_person;
        FreeCameraActiveState   *free_camera;

        exit = new FinalState ("exit", this); 
        active = new State ("active", this);
        this-> setInitialState (active);

        unfocused = new State ("unfocused", active);
        active-> setInitialState (unfocused);

        focused = new InputActiveState ("focused", view_, QState::ParallelStates, active);

        mouse = new State ("mouse", QState::ParallelStates, focused);
        keyboard = new State ("keyboard", QState::ParallelStates, focused);
        perspective = new State ("perspective", focused);

        wheel = new State ("wheel", mouse);
        wheel_active = new WheelActiveState ("wheel active", eventmgr_, wheel);
        wheel_waiting = new State ("wheel waiting", wheel);
        wheel-> setInitialState (wheel_waiting);

        left_button = new State ("left button", mouse);
        left_button_active = new LeftButtonActiveState ("left button active", eventmgr_, left_button);
        left_button_waiting = new State ("left button waiting", left_button);
        left_button-> setInitialState (left_button_waiting);

        right_button = new State ("right button", mouse);
        right_button_active = new RightButtonActiveState ("right button active", eventmgr_, right_button);
        right_button_waiting = new State ("right button waiting", right_button);
        right_button-> setInitialState (right_button_waiting);

        mid_button = new State ("middle button", mouse);
        mid_button_active = new MidButtonActiveState ("middle button active", eventmgr_, mid_button);
        mid_button_waiting = new State ("middle button waiting", mid_button);
        mid_button-> setInitialState (mid_button_waiting);

        button = new State ("button", mouse);
        button_active = new ButtonActiveState ("button active", mouse_state_, button);
        button_waiting = new State ("mouse waiting", button);
        button-> setInitialState (button_waiting);

        gesture_active = new GestureActiveState ("gesture active", gesture_state_, eventmgr_, button_active);
        gesture_waiting = new State ("gesture waiting", button_active);
        button_active-> setInitialState (gesture_waiting);

        first_person = new FirstPersonActiveState ("first person", &key_binding_, perspective);
        third_person = new ThirdPersonActiveState ("third person", &key_binding_, perspective);
        free_camera = new FreeCameraActiveState ("free camera", &key_binding_, perspective);
        perspective-> setInitialState (third_person);

        (new EventTransition <QEvent::Close> (active))-> setTargetState (exit);
        (new EventTransition <QEvent::FocusIn> (unfocused))-> setTargetState (focused);
        (new EventTransition <QEvent::FocusOut> (focused))-> setTargetState (unfocused);

        (new KeyListener (key_states_, &key_binding_, eventmgr_, keyboard));

        (new EventTransition <QEvent::Wheel> (wheel_waiting))-> setTargetState (wheel_active);
        (new UnconditionalTransition (wheel_active))-> setTargetState (wheel_waiting);

        (new LeftButtonPress (left_button_waiting))-> setTargetState (left_button_active);
        (new LeftButtonRelease (left_button_active))-> setTargetState (left_button_waiting);
        
        (new MidButtonPress (mid_button_waiting))-> setTargetState (mid_button_active);
        (new MidButtonRelease (mid_button_active))-> setTargetState (mid_button_waiting);
        
        (new RightButtonPress (right_button_waiting))-> setTargetState (right_button_active);
        (new RightButtonRelease (right_button_active))-> setTargetState (right_button_waiting);
        
        (new EventTransition <QEvent::MouseButtonPress> (button_waiting))-> setTargetState (button_active);
        (new EventTransition <QEvent::MouseMove> (gesture_waiting))-> setTargetState (gesture_active);

        (new EventTransition <QEvent::MouseButtonRelease> (button_active))-> setTargetState (button_waiting);
        (new EventTransition <QEvent::MouseButtonRelease> (gesture_active))-> setTargetState (button_waiting);
        (new GestureActive (gesture_state_, eventmgr_, gesture_active));

        (new EventTransition <FirstPersonEventType> (third_person))-> setTargetState (first_person);
        (new EventTransition <FirstPersonEventType> (free_camera))-> setTargetState (first_person);
        (new EventTransition <ThirdPersonEventType> (first_person))-> setTargetState (third_person);
        (new EventTransition <ThirdPersonEventType> (free_camera))-> setTargetState (third_person);
        (new EventTransition <FreeCameraEventType> (first_person))-> setTargetState (free_camera);
        (new EventTransition <FreeCameraEventType> (third_person))-> setTargetState (free_camera);
    }

    QEvent *WorldInputLogic::clone_event_ (QEvent *event)
    {
        // state machine will take ownership of cloned events
        switch (event-> type())
        {
            case QEvent::MouseMove:
            case QEvent::MouseButtonPress:
            case QEvent::MouseButtonRelease:
                return new QMouseEvent (*static_cast <QMouseEvent *> (event));

            case QEvent::Wheel:
                return new QWheelEvent (*static_cast <QWheelEvent *> (event));

            case QEvent::KeyPress:
            case QEvent::KeyRelease:
                return new QKeyEvent (*static_cast <QKeyEvent *> (event));

            case QEvent::Close:
                return new QCloseEvent (*static_cast <QCloseEvent *> (event));
        }

		return 0;
    }
}

// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "InputStateMachine.h"
#include "EventManager.h"
#include "Framework.h"
#include "ModuleManager.h"
#include "ConfigManager.h"

namespace Input
{
    // keep a manifest of all known named states
    // used by WorldInputLogic to fetch states
    static Foundation::StateMap input_state_registry_;

    // remember key bindings sent when state machine is inactive
    // this is required because QApp is started after Module init
    static KeyBindingInfoList key_binding_cache_;


    //=========================================================================
    //
    MouseInfo::MouseInfo ()
        : buttons (0),
        wheel_delta (0), wheel_orientation (0),
        global_x (0), global_y (0), x (0), y (0)
    {
    }

    MouseInfo::MouseInfo (const MouseInfo &rhs) 
        : buttons (rhs.buttons), 
        wheel_delta (rhs.wheel_delta), wheel_orientation (rhs.wheel_orientation),
        global_x (rhs.global_x), global_y (rhs.global_y), x (rhs.x), y (rhs.y) 
    {
    }

    MouseInfo::MouseInfo (const QMouseEvent *e) 
        : buttons (e->buttons()), 
        wheel_delta (0), wheel_orientation (0),
        global_x (e->globalX()), global_y (e->globalY()), x (e->x()), y (e->y()) 
    {
    }

    MouseInfo::MouseInfo (const QWheelEvent *e) 
        : buttons (e->buttons()), 
        wheel_delta (e->delta()), wheel_orientation (e->orientation()),
        global_x (e->globalX()), global_y (e->globalY()), x (e->x()), y (e->y()) 
    {
    }

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
    InputState::InputState (QString name, QState *parent) 
        : State (name, input_state_registry_, parent)
    {
    }

    InputState::InputState (QString name, QState::ChildMode mode, QState *parent) 
        : State (name, input_state_registry_, mode, parent)
    {
    }

    //=========================================================================
    //
    KeyState::KeyState (const QKeySequence &s, QState *p)
        : InputState (s.toString(), p), 
        sequence (s),
        bindings (0)
    {
        catid = eventmgr-> QueryEventCategory ("Input");
    }

    KeyState::KeyState (const QKeySequence &s, Foundation::KeyBindings **b, Foundation::EventManager* m, QState *p)
        : InputState (s.toString(), p), 
        sequence (s),
        bindings (b), 
        eventmgr (m)
    {
        catid = eventmgr-> QueryEventCategory ("Input");
    }

    KeyState::~KeyState ()
    {
    }

    void KeyState::onEntry (QEvent *e)
    {
        std::pair <int,int> eid = get_event_ids ();

        if (bindings && eid.first)
            eventmgr-> SendEvent (catid, eid.first, 0);
        
        State::onEntry (e);
    }

    void KeyState::onExit (QEvent *e)
    {
        std::pair <int,int> eid = get_event_ids ();

        if (bindings && eid.second)
            eventmgr-> SendEvent (catid, eid.second, 0);
        
        State::onEntry (e);
    }

    std::pair <int,int> KeyState::get_event_ids ()
    {
        return (*bindings)->GetEventPair(sequence);
    }

    bool KeyState::operator== (const KeyState &rhs)
    {
        return sequence == rhs.sequence;
    }

    
    //=========================================================================
    //
    InputActiveState::InputActiveState (QString name, QGraphicsView *v, QState::ChildMode m, QState *p) 
        : InputState (name, m, p), view (v)
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
    KeyboardActiveState::KeyboardActiveState (QString name, QState::ChildMode m, QState *p)
        : InputState (name, m, p)
    {
    }

    void KeyboardActiveState::onEntry (QEvent *event)
    {
        State::onEntry (event);
    }
    
    void KeyboardActiveState::onExit (QEvent *event)
    {
        // cancel any active key presses
        KeyStateList::iterator i = active.begin();
        KeyStateList::iterator e = active.end();
        for (; i != e; ++i)
            (*i)-> onExit (event);
        active.erase (active.begin(), active.end());

        State::onExit (event);
    }

    //=========================================================================
    //
    MouseActiveState::MouseActiveState (QString name, QGraphicsScene *s, QState::ChildMode m, QState *p)
        : InputState (name, m, p), scene (s)
    {
    }

    //=========================================================================
    //
    WheelActiveState::WheelActiveState (QString name, Foundation::EventManager* m, QState *p)
        : InputState (name, p), eventmgr (m)
    {
        catid = eventmgr-> QueryEventCategory ("Input");
    }

    void WheelActiveState::onEntry (QEvent *event)
    {
        State::onEntry (event);

        QWheelEvent *e = static_cast <QWheelEvent *> (event);

        scroll.z_.rel_ = e-> delta();
        scroll.z_.abs_ = e-> orientation();

        eventmgr-> SendEvent (catid, Input::Events::SCROLL, &scroll);
    }
    
    void WheelActiveState::onExit (QEvent *event)
    {
        State::onExit (event);
    }

    //=========================================================================
    //
    MoveActiveState::MoveActiveState (QString name, Foundation::EventManager* m, QState *p)
        : InputState (name, p), eventmgr (m)
    {
        catid = eventmgr-> QueryEventCategory ("Input");
    }

    void MoveActiveState::onEntry (QEvent *event)
    {
        State::onEntry (event);

        QMouseEvent *e = static_cast <QMouseEvent *> (event);

        movement.x_.rel_ = 0;
        movement.y_.rel_ = 0;
        movement.x_.abs_ = e-> x();
        movement.y_.abs_ = e-> y();
        movement.x_.screen_ = e-> globalX();
        movement.y_.screen_ = e-> globalY();

        eventmgr-> SendEvent (catid, Input::Events::MOUSEMOVE, &movement);
    }
    
    void MoveActiveState::onExit (QEvent *event)
    {
        State::onExit (event);
    }

    //=========================================================================
    //
    LeftButtonActiveState::LeftButtonActiveState (QString name, Foundation::EventManager* m, QState *p)
        : InputState (name, p), eventmgr (m)
    {
        catid = eventmgr-> QueryEventCategory ("Input");
    }

    void LeftButtonActiveState::onEntry (QEvent *event)
    {
        State::onEntry (event);

        QMouseEvent *e = static_cast <QMouseEvent *> (event);
        movement.x_.rel_ = 0;
        movement.y_.rel_ = 0;
        movement.x_.abs_ = e-> x();
        movement.y_.abs_ = e-> y();
        movement.x_.screen_ = e-> globalX();
        movement.y_.screen_ = e-> globalY();

        // click may cause loss of focus
        QState *p = parentState()-> parentState(); // "mouse active" state has scene
        if (!static_cast <MouseActiveState *> (p)-> scene-> focusItem())
            eventmgr-> SendEvent (catid, Input::Events::INWORLD_CLICK, &movement);

        eventmgr-> SendEvent (catid, Input::Events::LEFT_MOUSECLICK_PRESSED, &movement);
    }
    
    void LeftButtonActiveState::onExit (QEvent *event)
    {
        QMouseEvent *e = static_cast <QMouseEvent *> (event);
        movement.x_.rel_ = 0;
        movement.y_.rel_ = 0;
        movement.x_.abs_ = e-> x();
        movement.y_.abs_ = e-> y();
        movement.x_.screen_ = e-> globalX();
        movement.y_.screen_ = e-> globalY();

        eventmgr-> SendEvent (catid, Input::Events::LEFT_MOUSECLICK_RELEASED, &movement);

        State::onExit (event);
    }

    //=========================================================================
    //
    RightButtonActiveState::RightButtonActiveState (QString name, Foundation::EventManager* m, QState *p)
        : InputState (name, p), eventmgr (m)
    {
        catid = eventmgr-> QueryEventCategory ("Input");
    }

    void RightButtonActiveState::onEntry (QEvent *event)
    {
        State::onEntry (event);

        QMouseEvent *e = static_cast <QMouseEvent *> (event);
        movement.x_.rel_ = 0;
        movement.y_.rel_ = 0;
        movement.x_.abs_ = e-> x();
        movement.y_.abs_ = e-> y();
        movement.x_.screen_ = e-> globalX();
        movement.y_.screen_ = e-> globalY();

        eventmgr-> SendEvent (catid, Input::Events::RIGHT_MOUSECLICK_PRESSED, &movement);
    }

    void RightButtonActiveState::onExit (QEvent *event)
    {
        eventmgr-> SendEvent (catid, Input::Events::RIGHT_MOUSECLICK_RELEASED, &movement);

        QMouseEvent *e = static_cast <QMouseEvent *> (event);
        movement.x_.rel_ = 0;
        movement.y_.rel_ = 0;
        movement.x_.abs_ = e-> x();
        movement.y_.abs_ = e-> y();
        movement.x_.screen_ = e-> globalX();
        movement.y_.screen_ = e-> globalY();

        State::onExit (event);
    }

    //=========================================================================
    //
    MidButtonActiveState::MidButtonActiveState (QString name, Foundation::EventManager* m, QState *p)
        : InputState (name, p), eventmgr (m)
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
    GestureActiveState::GestureActiveState (QString name, GestureInfo &g, Foundation::EventManager* m, QState *p)
        : InputState (name, p), gesture (g), eventmgr (m)
    {
        catid = eventmgr-> QueryEventCategory ("Input");
    }

    void GestureActiveState::onEntry (QEvent *event)
    {
        State::onEntry (event);

        //MouseInfo info (static_cast <QMouseEvent *> (event));
        //gesture.drag.push_back (info);

        QMouseEvent *e = static_cast <QMouseEvent *> (event);
        gesture.last_x = e-> x();
        gesture.last_y = e-> y();

        // Do not send relative movement yet on gesture start
        movement.x_.rel_ = 0;
        movement.y_.rel_ = 0;
        movement.x_.abs_ = e-> x();
        movement.y_.abs_ = e-> y();
        movement.x_.screen_ = e-> globalX();
        movement.y_.screen_ = e-> globalY();

        if (e->buttons() == Qt::LeftButton)
            eventmgr-> SendEvent (catid, Input::Events::MOUSEDRAG, &movement);
        if (e->buttons() == Qt::RightButton)
            eventmgr-> SendEvent (catid, Input::Events::MOUSELOOK, &movement);
    }

    //=========================================================================
    //
    GestureMotionState::GestureMotionState (QString name, GestureInfo &g, Foundation::EventManager* m, QState *p)
        : InputState (name, p), gesture (g), eventmgr (m)
    {
        catid = eventmgr-> QueryEventCategory ("Input");
    }

    void GestureMotionState::onEntry (QEvent *event)
    {
        State::onEntry (event);

        QMouseEvent *e = static_cast <QMouseEvent *> (event);
        
        movement.x_.rel_ = e-> x() - gesture.last_x;
        movement.y_.rel_ = e-> y() - gesture.last_y;
        movement.x_.abs_ = e-> x();
        movement.y_.abs_ = e-> y();
        movement.x_.screen_ = e-> globalX();
        movement.y_.screen_ = e-> globalY();
        
        gesture.last_x = e-> x();
        gesture.last_y = e-> y();

        if (e->buttons() == Qt::LeftButton)
            eventmgr-> SendEvent (catid, Input::Events::MOUSEDRAG, &movement);
        if (e->buttons() == Qt::RightButton)
            eventmgr-> SendEvent (catid, Input::Events::MOUSELOOK, &movement);
    }

    //=========================================================================
    //
    GestureCompleteState::GestureCompleteState (QString name, GestureInfo &g, Foundation::EventManager* m, QState *p)
        : InputState (name, p), gesture (g), eventmgr (m)
    {
        catid = eventmgr-> QueryEventCategory ("Input");
    }

    void GestureCompleteState::onEntry (QEvent *event)
    {
        //MouseInfo info (static_cast <QMouseEvent *> (event));
        //gesture.drag.push_back (info);

        eventmgr-> SendEvent (catid, Input::Events::MOUSELOOK_STOPPED, 0);
        eventmgr-> SendEvent (catid, Input::Events::MOUSEDRAG_STOPPED, 0);

        State::onEntry (event);
    }

    //=========================================================================
    //
    KeyBindingActiveState::KeyBindingActiveState (QString name, Foundation::KeyBindings **m, QState *p)
        : InputState (name, p), map (m)
    {
    }

    //=========================================================================
    //
    FirstPersonActiveState::FirstPersonActiveState (QString name, Foundation::KeyBindings **m, QState *p)
        : KeyBindingActiveState (name, m, p)
    {
        bindings = m;
    }

    void FirstPersonActiveState::onEntry (QEvent *event)
    {
        State::onEntry (event);
        //*map = &(bindings.map);
        map = bindings;
    }

    //=========================================================================
    //
    ThirdPersonActiveState::ThirdPersonActiveState (QString name, Foundation::KeyBindings **m, QState *p)
        : KeyBindingActiveState (name, m, p)
    {
        bindings = m;
    }

    void ThirdPersonActiveState::onEntry (QEvent *event)
    {
        State::onEntry (event);
        //*map = &(bindings.map);
        map = bindings;
    }

    //=========================================================================
    //
    FreeCameraActiveState::FreeCameraActiveState (QString name, Foundation::KeyBindings **m, QState *p)
        : KeyBindingActiveState (name, m, p)
    {
        bindings = m;
    }

    void FreeCameraActiveState::onEntry (QEvent *event)
    {
        State::onEntry (event);
        //*map = &(bindings.map);
        map = bindings;
    }

    //=========================================================================
    //

    KeyListener::KeyListener (KeyStateMap &s, Foundation::KeyBindings **b, Foundation::EventManager* m, QState *p)
        : QAbstractTransition (p), key_states (s), bindings (b), eventmgr (m)
    {
        parent = static_cast <KeyboardActiveState *> (p);
        setTargetState (0);
    }

    bool KeyListener::eventTest (QEvent *event)
    {
        // this is preferrable to 3 states + 2 transitions for each key
        // plus a look-up tree if you want to discover the current state any way 
        
        switch (event-> type())
        {
            case QEvent::KeyPress:
                {
                    QKeyEvent *e = static_cast <QKeyEvent *> (event);
                    KeyState *s = get_key_state 
                        (QKeySequence (e-> key() + (e-> modifiers() & ~Qt::ShiftModifier)));

                    s-> onEntry (e);
                    press_active (s);
                }
                return true;

            case QEvent::KeyRelease:
                {
                    QKeyEvent *e = static_cast <QKeyEvent *> (event);
                    KeyState *s = get_key_state 
                        (QKeySequence (e-> key() + (e-> modifiers() & ~Qt::ShiftModifier)));

                    s-> onExit (e);
                    release_active (s);

                    return true;
                }

            default:
                return false;
        }
    }

    void KeyListener::onTransition (QEvent *e)
    {
    }

    KeyState *KeyListener::get_key_state (const QKeySequence &sequence)
    {
        KeyState *state;
        KeyStateMap::const_iterator i = key_states.find (sequence);
        KeyStateMap::const_iterator e = key_states.end ();

        if (i == e) 
        {
            state = new KeyState (sequence, bindings, eventmgr);
            key_states.insert (std::make_pair (sequence, state));
        }
        else
            state = i-> second;

        return state;
    }

    void KeyListener::press_active (KeyState *state)
    {
        parent-> active.push_back (state);
    }

    void KeyListener::release_active (KeyState *state)
    {
        KeyStateList::iterator b = parent-> active.begin();
        KeyStateList::iterator e = parent-> active.end();
        KeyStateList::iterator i = std::find (b, e, state);
        if (i != e)
            parent-> active.erase (i);
    }

    //=========================================================================
    //
    WorldInputLogic::WorldInputLogic (Foundation::Framework *fw)
        : framework_ (fw),
        view_ (framework_-> GetUIView()),
        eventmgr_ (framework_-> GetEventManager().get()),
        has_focus_ (false),
        config_manager_ (new ConfigManager(this)),
        key_bindings_ (0)
    {
        init_statemachine_();
        
        view_-> installEventFilter (this);
        view_-> viewport()-> installEventFilter (this);
    }

    WorldInputLogic::~WorldInputLogic()
    {
        SAFE_DELETE(key_bindings_);
    }

    bool WorldInputLogic::eventFilter (QObject *obj, QEvent *event)
    {
        // route select Qt events to the state machine
        switch (event-> type())
        {
            case QEvent::KeyPress:
            case QEvent::KeyRelease:
                {
                    QKeyEvent *e = static_cast <QKeyEvent *> (event);

                    if (has_focus_ && !e->isAutoRepeat())
                        postEvent (clone_event_ (event));
                }
                break;

            case QEvent::MouseButtonPress:
            case QEvent::MouseButtonRelease:
                {
                    QMouseEvent *e = static_cast <QMouseEvent *> (event);
                    mouse_state_ = e; // record mouse state for focus handling
                }
                // fall through
            
            case QEvent::MouseMove:
            case QEvent::Wheel:
            case QEvent::Close:
                {
                    if (has_focus_) 
                        postEvent (clone_event_ (event));
                }
                break;
        }

        return QObject::eventFilter (obj, event);
    }

    Foundation::State *WorldInputLogic::GetState (QString name)
    {
        using Foundation::StateMap;
        StateMap::iterator i = input_state_registry_.find (QString (name));
        StateMap::iterator e = input_state_registry_.end ();
        return (i != e)? i-> second : 0;
    }

    void WorldInputLogic::AddKeyEvent (QString group, QString sequence, event_id_t enter, event_id_t exit)
    {
        key_binding_cache_.push_back (KeyBindingInfo (group, sequence, enter, exit));
    }

    Foundation::KeyBindings *WorldInputLogic::GetBindings()
    {
        return key_bindings_;
    }

    void WorldInputLogic::SetBindings(Foundation::KeyBindings *bindings)
    {
        SAFE_DELETE(key_bindings_);
        key_bindings_ = bindings;
        config_manager_->WriteCustomConfig(key_bindings_);
    }

    void WorldInputLogic::RestoreDefaultBindings()
    {
        SAFE_DELETE(key_bindings_);
        key_bindings_ = config_manager_->ParseConfig("Bindings.Default");
        config_manager_->ClearUserConfig();
    }

    void WorldInputLogic::Update (f64 frametime)
    {
        update_dynamic_key_bindings_ ();

        if (view_-> scene()-> focusItem())
        {
            if (has_focus_)
            {
                has_focus_ = false;
                postEvent (new QFocusEvent (QEvent::FocusOut));
            }
        }
        else
        {
            if (!has_focus_)
            {
                has_focus_ = true;
                postEvent (new QFocusEvent (QEvent::FocusIn));
                post_simulated_focus_click();
            }
        }
    }

    void WorldInputLogic::init_statemachine_ ()
    {
        QFinalState *exit;

        InputState *active, *unfocused,
            *perspective, *move, *wheel, *gesture,
            *left_button, *right_button, *mid_button,
            *move_waiting, *wheel_waiting,
            *left_button_waiting, *right_button_waiting, *mid_button_waiting,
            *gesture_waiting, *gesture_enabled, *gesture_pause;

        InputActiveState        *focused;
        KeyboardActiveState     *keyboard_active;
        MouseActiveState        *mouse_active;
        WheelActiveState        *wheel_active;
        MoveActiveState         *move_active;
        LeftButtonActiveState   *left_button_active;
        RightButtonActiveState  *right_button_active;
        MidButtonActiveState    *mid_button_active;
        GestureActiveState      *gesture_active;
        GestureMotionState      *gesture_motion;
        GestureCompleteState    *gesture_complete;

        FirstPersonActiveState  *first_person;
        ThirdPersonActiveState  *third_person;
        FreeCameraActiveState   *free_camera;

        key_bindings_ = config_manager_->GetUsedKeyBindings();

        // ====================================================================
        // States

        // Scope
        exit = new QFinalState (this); 
        active = new InputState ("active", this);
        this-> setInitialState (active);

        unfocused = new InputState ("unfocused", active);
        active-> setInitialState (unfocused);

        focused = new InputActiveState ("focused", view_, QState::ParallelStates, active);
        perspective = new InputState ("perspective", focused);

        keyboard_active = new KeyboardActiveState ("keyboard", QState::ParallelStates, focused);
        mouse_active = new MouseActiveState ("mouse active", view_-> scene(), QState::ParallelStates, focused);

        // Movement
        move = new InputState ("move", mouse_active);
        move_active = new MoveActiveState ("move active", eventmgr_, move);
        move_waiting = new InputState ("move waiting", move);
        move-> setInitialState (move_waiting);

        // Wheel
        wheel = new InputState ("wheel", mouse_active);
        wheel_active = new WheelActiveState ("wheel active", eventmgr_, wheel);
        wheel_waiting = new InputState ("wheel waiting", wheel);
        wheel-> setInitialState (wheel_waiting);

        // Buttons
        left_button = new InputState ("left button", mouse_active);
        left_button_active = new LeftButtonActiveState ("left button active", eventmgr_, left_button);
        left_button_waiting = new InputState ("left button waiting", left_button);
        left_button-> setInitialState (left_button_waiting);

        right_button = new InputState ("right button", mouse_active);
        right_button_active = new RightButtonActiveState ("right button active", eventmgr_, right_button);
        right_button_waiting = new InputState ("right button waiting", right_button);
        right_button-> setInitialState (right_button_waiting);

        mid_button = new InputState ("middle button", mouse_active);
        mid_button_active = new MidButtonActiveState ("middle button active", eventmgr_, mid_button);
        mid_button_waiting = new InputState ("middle button waiting", mid_button);
        mid_button-> setInitialState (mid_button_waiting);

        // Gestures
        gesture = new InputState ("gesture", mouse_active);
        gesture_active = new GestureActiveState ("gesture active", gesture_state_, eventmgr_, gesture);
        gesture_complete = new GestureCompleteState ("gesture complete", gesture_state_, eventmgr_, gesture);
        gesture_enabled = new InputState ("gesture enabled", gesture);
        gesture_waiting = new InputState ("gesture waiting", gesture);
        gesture-> setInitialState (gesture_waiting);
        
        gesture_motion = new GestureMotionState ("gesture motion", gesture_state_, eventmgr_, gesture_active);
        gesture_pause = new InputState ("gesture pause", gesture_active);
        gesture_active-> setInitialState (gesture_pause);

        // Camera Focus (for keybindings)
        first_person = new FirstPersonActiveState ("first person", &key_bindings_, perspective);
        third_person = new ThirdPersonActiveState ("third person", &key_bindings_, perspective);
        free_camera = new FreeCameraActiveState ("free camera", &key_bindings_, perspective);
        perspective-> setInitialState (third_person);

        // ====================================================================
        // Transitions

        // Scope
        (new EventTransition <QEvent::Close> (active))-> setTargetState (exit);
        (new EventTransition <QEvent::FocusIn> (unfocused))-> setTargetState (focused);
        (new EventTransition <QEvent::FocusOut> (focused))-> setTargetState (unfocused);

        // Keys
        (new KeyListener (key_states_, &key_bindings_, eventmgr_, keyboard_active));

        // Movement
        (new EventTransition <QEvent::MouseMove> (move_waiting))-> setTargetState (move_active);
        (new UnconditionalTransition (move_active))-> setTargetState (move_waiting);

        // Wheel
        (new EventTransition <QEvent::Wheel> (wheel_waiting))-> setTargetState (wheel_active);
        (new UnconditionalTransition (wheel_active))-> setTargetState (wheel_waiting);

        // Buttons
        (new LeftButtonPress (left_button_waiting))-> setTargetState (left_button_active);
        (new LeftButtonRelease (left_button_active))-> setTargetState (left_button_waiting);
        
        (new MidButtonPress (mid_button_waiting))-> setTargetState (mid_button_active);
        (new MidButtonRelease (mid_button_active))-> setTargetState (mid_button_waiting);
        
        (new RightButtonPress (right_button_waiting))-> setTargetState (right_button_active);
        (new RightButtonRelease (right_button_active))-> setTargetState (right_button_waiting);
        
        // Gestures
        (new EventTransition <QEvent::MouseButtonPress> (gesture_waiting))-> setTargetState (gesture_enabled);
        (new EventTransition <QEvent::MouseMove> (gesture_enabled))-> setTargetState (gesture_active);
        (new EventTransition <QEvent::MouseButtonRelease> (gesture_active))-> setTargetState (gesture_complete);
        (new UnconditionalTransition (gesture_complete))-> setTargetState (gesture_waiting);

        (new EventTransition <QEvent::MouseMove> (gesture_pause))-> setTargetState (gesture_motion);
        (new UnconditionalTransition (gesture_motion))-> setTargetState (gesture_pause);

        // Camera Focus (for keybindings)
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

            default:
                return 0;
        }
    }
    
    void WorldInputLogic::post_simulated_focus_click ()
    {
        // called when focus is gained, which replays the mouse click that delivered the focus
        // (which has been lost because we weren't focused)

        postEvent (new QMouseEvent (QEvent::MouseButtonPress, QPoint (mouse_state_.x, mouse_state_.y), 
                    Qt::LeftButton, (Qt::MouseButtons)mouse_state_.buttons, Qt::NoModifier));
        
        postEvent (new QMouseEvent (QEvent::MouseButtonRelease, QPoint (mouse_state_.x, mouse_state_.y), 
                    Qt::LeftButton, (Qt::MouseButtons)mouse_state_.buttons, Qt::NoModifier));
    }

    void WorldInputLogic::update_dynamic_key_bindings_ ()
    {
        Foundation::KeyBindings *bindings;
        KeyBindingActiveState *state;
        KeyBindingInfoList::iterator kb, i, b = key_binding_cache_.begin();
        KeyBindingInfoList::iterator e = key_binding_cache_.end();

        for (i = b; i != e;)
        {
            kb = i++;
            state = dynamic_cast <KeyBindingActiveState *> (GetState (kb-> group));
            bindings = (state)? *(state-> map) : 0; 

            if (bindings)
            {
                bindings->BindKey(QKeySequence (kb-> sequence), std::make_pair (kb-> enter, kb-> exit));
                key_binding_cache_.erase (kb);
            }
        }
    }
}

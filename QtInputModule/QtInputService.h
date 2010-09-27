#ifndef incl_QtInputModule_QtInputService_h
#define incl_QtInputModule_QtInputService_h

#include "QtInputModuleApi.h"
#include "InputServiceInterface.h"
#include "ForwardDefines.h"
#include <QTime>
/// QtInputService provides other modules with different methods of acquiring keyboard and mouse input.
/** The input service works with the notion of 'input contexts', which are objects that modules acquire
    to receive input events. The contexts have a priority that determine the order in which the input 
    events are posted.

    Input events are processed in the following order:
        1) Very first, when a new input event is received, it is posted to the top level input context.
           See QtInputService::TopLevelInputContext(). This is already before any Qt widgets get the
           chance to process the event.
        2) If the event is a mouse event that occurs on top of a Qt widget, or the event is a key event
           and a Qt widget has keyboard focus, the event is passed to Qt, and suppressed from going
           further.
        3) The event is posted to all the registered input contexts in their order of priority. See 
           QtInputService::RegisterInputContext().
        4) The event is posted to the system-wide event tree. See the QtInputEvents namespace.

    At any level, the handler may set the handled member of a KeyEvent or MouseEvent to true to suppress
    the event from going forward to the lower levels.

    In addition to the abovementioned methods, a module may use a polling API to directly query input 
    state. This API operates on the input level (1) above. See the functions IsKeyDown, IsKeyPressed, 
    IsKeyReleased, IsMouseButtonDown, IsMouseButtonPressed and IsMouseButtonReleased.

    The InputContext -based API utilizes Qt signals. The polling API can be used by any object that
    has access to QtInputService, and the event tree -based API can be used by all modules.
*/
class QTINPUT_MODULE_API QtInputService : public InputServiceInterface
{
    Q_OBJECT

public:
    /// Initializes the service and hooks it into the main application window.
    explicit QtInputService(Foundation::Framework *owner);

    /// The dtor saves the settings to QSettings.
    ~QtInputService();

    /// Proceeds the input system one application frame forward (Ages all double-buffered input data).
    /// Called by QtInputModule. Not for client use.
    void Update(f64 frametime);

    /// Creates a new input context with the given name. The name is not an ID, i.e. it does not have to be unique with 
    /// existing contexts (although it is encouraged). When you no longer need the context, free all refcounts to it.
    /// Remember to hold on to a shared_ptr of the input context as long as you are using the context.
    boost::shared_ptr<InputContext> RegisterInputContext(const char *name, int priority);

    /// Prints the list of input contexts to std::cout, for debugging purposes.
    void DumpInputContexts();

    typedef std::map<std::string, QKeySequence> KeyActionsMap;

public slots:
    /// Returns the topmost visible QGraphicsItem in the given application main window coordinates.
	QGraphicsItem *GetVisibleItemAtCoords(int x, int y);

    /// Sets the mouse cursor in absolute (the usual default) or relative movement (FPS-like) mode.
    /// @param visible If true, shows mouse cursor and allows free movement. If false, hides the mouse cursor 
    ///                and switches into relative mouse movement input mode.
    void SetMouseCursorVisible(bool visible);

    /// @return True if we are in absolute movement mode, and false if we are in relative mouse movement mode.
    bool IsMouseCursorVisible() const;

    /// Returns true if the given key is physically held down (to the best knowledge of the input service, which may
    /// be wrong depending on whether Qt has managed to successfully deliver the information). This ignores all
    /// the grabs and contexts, e.g. you will get true even if a text edit has focus in a Qt widget.
    /// @param keyCode The Qt::Key to test, http://doc.trolltech.com/4.6/qt.html#Key-enum
    bool IsKeyDown(Qt::Key keyCode) const;

    /// Returns true if the given key was pressed down during this frame. A frame in this context means a period
    /// between two subsequent calls to QtInputService::Update. During a single frame, calling this function
    /// several times will always return true if the key was pressed down this frame, i.e. the pressed-bit is not
    /// cleared after the first query.
    /// Key repeats will not be reported through this function.
    /// @param keyCode The Qt::Key to test, http://doc.trolltech.com/4.6/qt.html#Key-enum
    bool IsKeyPressed(Qt::Key keyCode) const;

    /// Functions like IsKeyPressed, but for key releases.
    /// @param keyCode The Qt::Key to test, http://doc.trolltech.com/4.6/qt.html#Key-enum
    bool IsKeyReleased(Qt::Key keyCode) const;

    /// Returns true if the given mouse button is being held down. This ignores all mousegrabbers and contexts
    /// and returns the actual state of the given button. 
    /// @param mouseButton The Qt mouse button flag to query for. Go to http://doc.trolltech.com/4.6/qt.html#MouseButton-enum 
    ///    for more information. Do not pass in a combination of the bit fields in the enum, just a single value.
    bool IsMouseButtonDown(int mouseButton) const;

    /// Returns true if the given mouse button was pressed down during this frame. Behaves like IsKeyPressed,
    /// but for mouse presses.
    /// @param mouseButton The Qt mouse button flag to query for. Go to http://doc.trolltech.com/4.6/qt.html#MouseButton-enum 
    ///    for more information. Do not pass in a combination of the bit fields in the enum, just a single value.
    bool IsMouseButtonPressed(int mouseButton) const;

    /// Returns true if the given mouse button was released during this frame.
    /// @param mouseButton The Qt mouse button flag to query for. Go to http://doc.trolltech.com/4.6/qt.html#MouseButton-enum 
    ///    for more information. Do not pass in a combination of the bit fields in the enum, just a single value.
    bool IsMouseButtonReleased(int mouseButton) const;

    /// Returns the mouse coordinates in local client coordinate frame denoting where the given mouse button was last pressed
    /// down. Note that this does not tell whether the mouse button is currently held down or not.
    QPoint MousePressedPos(int mouseButton) const;

    /// Called by QtInputService internally for each generate KeyEvent. This function passes the event forward to all registered
    /// input contexts. You may generate KeyEvent objects yourself and call this function directly to inject a custom KeyEvent
    /// to the system.
    void TriggerKeyEvent(KeyEvent &key);

    /// This is the same as OnKeyEvent, but for mouse events.
    void TriggerMouseEvent(MouseEvent &mouse);

    /// Returns the highest-priority input context that gets all events first to handle (even before going to Qt widgets).
    /// You may register your own keyboard and mouse handlers in this context and block events from going to the main window
    /// (by setting the .handled member of the event to true), but be very careful when doing so.
    InputContext &TopLevelInputContext() { return topLevelInputContext; }

    /// Associates the given custom action with the given key.
    void SetKeyBinding(const QString &actionName, QKeySequence key);

    /// Returns the key associated with the given action.
    /// @param actionName The custom action name to query. The convention is to use two-part names, separated with a period, i.e.
    ///        "category.name". For example, RexLogicModule has an action "Avatar.WalkForward" to control avatar movement.
    /// If the action does not exist, null sequence is returned.
    QKeySequence KeyBinding(const QString &actionName) const;

    /// Returns the key associated with the given action. This is the same function as KeyBinding(const QString &actionName),
    /// but in this form, if the action does not exist, the default key sequence is registered for it and returned.
    QKeySequence KeyBinding(const QString &actionName, QKeySequence defaultKey);

    void LoadKeyBindingsFromFile();

    void SaveKeyBindingsToFile();

    const KeyActionsMap &GetKeyBindings() const { return keyboardMappings; }

    void SetKeyBindings(const KeyActionsMap &actionMap) { keyboardMappings = actionMap; }

private:
    bool eventFilter(QObject *obj, QEvent *event);

    /// Sends key release messages for each currently tracked pressed key and clears the record of all pressed keys.
    void SceneReleaseAllKeys();
    /// Sends mouse button release messages for each mouse button that was held down.
    void SceneReleaseMouseButtons();
    /// Moves the mouse to the center of the client area. Used in relative mouse movement mode to force the mouse cursor
    /// to stay at screen center at all times.
    void RecenterMouse();
    /// Goes through the list of input contexts and removes from the list all contexts that have been destroyed.
    void PruneDeadInputContexts();
    /// Takes the given point in the coordinate frame of source and maps it to the coordinate space of the main graphics view.
    QPoint MapPointToMainGraphicsView(QObject *source, const QPoint &point);

    typedef std::list<boost::weak_ptr<InputContext> > InputContextList;

    /// Starting from the input context 'start', triggers key release events to that context and all lower ones.
    void TriggerSceneKeyReleaseEvent(InputContextList::iterator start, Qt::Key keyCode);

    // The coordinates in window client coordinate space denoting where the mouse left [0] /middle [1] /right [2] /XButton1 [3] /XButton2 [4] 
    // buttons were pressed down.
    MouseEvent::PressPositions mousePressPositions;

    // The last known mouse coordinates in window client coordinate space. These are not necessarily the coordinates from previous frame,
    // but from the previous Qt mouse input event.
    int lastMouseX;
    int lastMouseY;
    QTime lastMouseButtonReleaseTime;
    bool doubleClickDetected;

    /// If true, the mouse cursor is visible and free to move around as usual.
    /// If false, we use mouse in relative movement mode, meaning we hide the cursor and force it to stay in the middle of the application screen.
    /// In relative mode, only mouse relative coordinate updates are posted as events.
    bool mouseCursorVisible;

/*  ///\todo This is currently disabled, since it would be problematic in drag-n-drop between scene and UI.
    /// Specifies which part of the system has mouse capture focus.
    enum
    {
        NoMouseCapture, ///< Nothing is capturing the mouse - no mouse buttons are being held down.
        QtMouseCapture, ///< Qt's QGraphicsView is capturing the mouse.
        SceneMouseCapture ///< Our 3D inworld scene is capturing the mouse.
    } sceneMouseCapture;
*/ 
    // When mouse mode is transitioned from absolute to relative, we store the mouse coordinates of where that happened so that
    // we can nicely restore the mouse to the original coordinates when relative->absolute transition is made again.
    int mouseFPSModeEnterX;
    int mouseFPSModeEnterY;

    event_category_id_t inputCategory;

    /// Stores all the currently registered input contexts. The order these items are stored in the list corresponds to the
    /// priority at which each context will get the events.
    InputContextList registeredInputContexts;

    /// This input context is processed immediately as the first thing in the system. I.e. before even checking whether the
    /// input event should be passed to Qt.
    InputContext topLevelInputContext;

    /// Stores all the keyboard mappings we have gathered.
    KeyActionsMap keyboardMappings;

    /// Stores the currently held down keyboard buttons.
    std::map<Qt::Key, KeyPressInformation> heldKeys;

    /// Lists the keycodes of buttons that are taken to have been pressed down during this Update() cycle.
    std::vector<Qt::Key> pressedKeys;

    /// Lists the keycodes of buttons that are taken to have been pressed down during this Update() cycle.
    std::vector<Qt::Key> releasedKeys;

    /// An internal queue where all new Qt keypress events are stored before they are advertised for callers of IsKeyPressed().
    /// This queue is used to avoid key presses being missed between Update() cycles, i.e. so that the module Updates
    /// may get called in any order.
	std::vector<Qt::Key> newKeysPressedQueue;

    /// An internal queue where all new Qt keyrelease events are stored before they are advertised for callers of IsKeyReleased().
    /// This queue is used to avoid key releases being missed between Update() cycles, i.e. so that the module Updates
    /// may get called in any order.
	std::vector<Qt::Key> newKeysReleasedQueue;

    /// A bitmask of the currently held down mouse buttons.
	unsigned long heldMouseButtons;
    /// A bitmask for the mouse buttons pressed down this frame.
	unsigned long pressedMouseButtons;
    /// A bitmask for the mouse buttons released this frame.
	unsigned long releasedMouseButtons;

    /// Keep track of key modifiers to be sent with mouse pressed/released
    unsigned long current_modifiers_;

    // The following variables double-buffer the mouse button presses and releases, so that a module will not lose these
    // events depending on what order the module Updates are called.
	unsigned long newMouseButtonsPressedQueue;
	unsigned long newMouseButtonsReleasedQueue;

	Foundation::EventManagerPtr eventManager;

    QGraphicsView *mainView;
    QWidget *mainWindow;
    
    Foundation::Framework *framework;

    // QtInputService is noncopyable.
    QtInputService(const QtInputService &);
    void operator =(const QtInputService &);
};

#endif

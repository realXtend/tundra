// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_QtInputModule_QtInputModule_h
#define incl_QtInputModule_QtInputModule_h

#include "ModuleInterface.h"
#include "ModuleLoggingFunctions.h"
#include "EventDataInterface.h"

#include "QtInputModuleApi.h"

#include "QtInputKeyEvent.h"
#include "QtInputMouseEvent.h"
#include "InputContext.h"

#include <vector>

#include <QObject>

class QGraphicsItem;
class QGraphicsView;
class QWidget;

class QTINPUT_MODULE_API QtInputModule : public QObject, public Foundation::ModuleInterfaceImpl
{
    Q_OBJECT

    MODULE_LOGGING_FUNCTIONS

public:
    QtInputModule();
    ~QtInputModule();

    void Initialize();
    void Update(f64 frametime);

    static const std::string &NameStatic();

    /// Returns the topmost visible QGraphicsItem in the given client coordinates.
	QGraphicsItem *GetVisibleItemAtCoords(int x, int y);

    /// Sets the mouse cursor in absolute (the usual default) or relative movement (FPS-like) mode.
    /// @param visible If true, shows mouse cursor and allows free movement. If false, hides the mouse cursor 
    ///                and switches into relative mouse movement input mode.
    void SetMouseCursorVisible(bool visible);

    /// @return True if we are in absolute movement mode, and false if we are in relative mouse movement mode.
    bool IsMouseCursorVisible() const;

    /// Returns true if the given key is physically held down (to the best knowledge of the input module, which may
    /// be wrong depending on whether Qt has managed to successfully deliver the information). This ignores all
    /// the grabs and contexts, e.g. you will get true even if a text edit has focus in a Qt widget.
    /// @param keyCode The Qt::Key to test, http://doc.trolltech.com/4.6/qt.html#Key-enum
    bool IsKeyDown(int keyCode) const;

    /// Returns true if the given key was pressed down during this frame. A frame in this context means a period
    /// between two subsequent calls to QtInputModule::Update. During a single frame, calling this function
    /// several times will always return true if the key was pressed down this frame, i.e. the pressed-bit is not
    /// cleared after the first query.
    /// @param keyCode The Qt::Key to test, http://doc.trolltech.com/4.6/qt.html#Key-enum
    bool IsKeyPressed(int keyCode) const;

    /// Functions like IsKeyPressed, but for key releases.
    /// @param keyCode The Qt::Key to test, http://doc.trolltech.com/4.6/qt.html#Key-enum
    bool IsKeyReleased(int keyCode) const;

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

    /// Creates a new input context with the given name. The name is not an ID, i.e. it does not have to be unique with 
    /// existing contexts (although it is encouraged). When you no longer need the context, free all refcounts to it.
    boost::shared_ptr<InputContext> RegisterInputContext(const char *name, int priority);

    /// Called by QtInputModule internally for each generate KeyEvent. This function passes the event forward to all registered
    /// input contexts. You may generate KeyEvent objects yourself and call this function directly to inject a custom KeyEvent
    /// to the system.
    void OnKeyEvent(KeyEvent &key);

    /// This is the same as OnKeyEvent, but for mouse events.
    void OnMouseEvent(MouseEvent &mouse);

private:
    bool eventFilter(QObject *obj, QEvent *event);

    /// Sends key release messages for each currently tracked pressed key and clears the record of all pressed keys.
    void ReleaseAllKeys();
    /// Sends mouse button release messages for each mouse button that was held down.
    void ReleaseMouseButtons();
    /// Moves the mouse to the center of the client area. Used in relative mouse movement mode to force the mouse cursor
    /// to stay at screen center at all times.
    void RecenterMouse();
    /// Goes through the list of input contexts and removes from the list all contexts that have been destroyed.
    void PruneDeadInputContexts();

    // The coordinates in window client coordinate space denoting where the mouse left [0] /middle [1] /right [2] /XButton1 [3] /XButton2 [4] 
    // buttons were pressed down.
    MouseEvent::PressPositions mousePressPositions;

    // The last known mouse coordinates in window client coordinate space.
    int lastMouseX;
    int lastMouseY;

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

    typedef std::list<boost::weak_ptr<InputContext> > InputContextList;
    /// Stores all the currently registered input contexts. The order these items are stored in the list corresponds to the
    /// priority at which each context will get the events.
    InputContextList registeredInputContexts;

    /// This input context is processed immediately as the first thing in the system. I.e. before even checking whether the
    /// input event should be passed to Qt.
    InputContext topLevelInputContext;

    /// Stores the currently held down keyboard buttons.
    std::vector<Qt::Key> heldKeys;

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

    // The following variables double-buffer the mouse button presses and releases, so that a module will not lose these
    // events depending on what order the module Updates are called.
	unsigned long newMouseButtonsPressedQueue;
	unsigned long newMouseButtonsReleasedQueue;

	Foundation::EventManagerPtr eventManager;

    QGraphicsView *mainView;
    QWidget *mainWindow;
};

#endif

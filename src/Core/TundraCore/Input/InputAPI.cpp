// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "InputAPI.h"
#include "Framework.h"
#include "UiAPI.h"
#include "UiMainWindow.h"
#include "UiGraphicsView.h"
#include "LoggingFunctions.h"
#include "CoreDefines.h"
#include "ConfigAPI.h"
#include "Profiler.h"

#include <QList>
#include <QVector>
#include <QGraphicsItem>
#include <QGraphicsView>
#include <QKeyEvent>
#include <QGestureEvent>
#include <QTouchEvent>
#include <QApplication>
#include <QSettings>

#include <sstream>

#include "MemoryLeakCheck.h"

InputAPI::InputAPI(Framework *framework_)
:lastMouseX(0),
lastMouseY(0),
mouseCursorVisible(true),
gesturesEnabled(false),
//sceneMouseCapture(NoMouseCapture),
mouseFPSModeEnterX(0),
mouseFPSModeEnterY(0),
topLevelInputContext(this, "TopLevel", 100000), // The priority value for the top level context does not really matter, just put an arbitrary big value for display.
heldMouseButtons(0),
pressedMouseButtons(0),
releasedMouseButtons(0),
newMouseButtonsPressedQueue(0),
newMouseButtonsReleasedQueue(0),
currentModifiers(0),
mainView(0),
mainWindow(0),
numTouchPoints(0),
framework(framework_)
{
    assert(framework_);

    // Set up the global widget event filters that we will use to read our scene input from.
    // Note: Since we set up this object as an event filter to multiple widgets, we will receive
    //       the same events several times, so care has to be taken to ignore those duplicate events.
    // Qt oddities: 
    // 1. If we set setMouseTracking(true) to either the main QGraphicsView or its viewport,
    //    we do not still receive mouse move events if we install an event filter to those widgets.
    //    So, we set mouse tracking enabled to the application main window.
    // 2. Mouse wheel events are taken from the application main window.
    // 3. Key presses are taken from the main QGraphicsView and release events are taken from the main window.
    // 4. Mouse press and release events do not work if taken from the main window. Presses come through fine,
    //    but the releases are never received. Therefore, we take all our mouse presses and releases from
    //    the QGraphicsView viewport.
    // 5. Mouse moves are passed to the main window, but if a mouse button is down, the viewport will start receiving
    //    the mouse moves instead of the main window. (This is due to Qt's mouse grabbing feature).
    // In the event filter, we take care that we only take each event from the widget we intended to take it from,
    // avoiding duplicates.

    if (framework_->IsHeadless())
        return;

    mainView = framework_->Ui()->GraphicsView();
    assert(mainView);
    assert(mainView->viewport());

    // For key press events.
    mainView->installEventFilter(this);
    // For mouse presses and releases, as well as mouse moves when a button is being held down.
    mainView->viewport()->installEventFilter(this);
    mainView->viewport()->setAttribute(Qt::WA_AcceptTouchEvents, true);
    
    // Find the top-level widget that the QGraphicsView is contained in. We need 
    // to track mouse move events from that window.
    mainWindow = mainView;

    while(mainWindow->parentWidget())
        mainWindow = mainWindow->parentWidget();

    mainWindow->setMouseTracking(true);
    // For Mouse wheel events, key releases, and mouse moves (no button down).
    mainWindow->installEventFilter(this);
    mainWindow->setAttribute(Qt::WA_AcceptTouchEvents, true);

    LoadKeyBindingsFromFile();

    // Accept gestures
    // [Mac OS X] bug: Pan gesture is triggered if the (left) mouse button is long-pressed and not moved at all. Disabling gestures for now on Mac
#ifndef Q_WS_MAC
    QList<Qt::GestureType> gestures;
    gestures << Qt::PanGesture << Qt::PinchGesture << Qt::TapAndHoldGesture;
    foreach(Qt::GestureType type, gestures)
        mainWindow->grabGesture(type);
#endif
}

InputAPI::~InputAPI()
{
    Reset();
}

void InputAPI::Reset()
{
    untrackedInputContexts.clear();
    registeredInputContexts.clear();
    keyboardMappings.clear();
}

void InputAPI::SetMouseCursorVisible(bool visible)
{
    if (framework->IsHeadless())
        return;
        
    if (mouseCursorVisible == visible)
        return;

    mouseCursorVisible = visible;
    if (mouseCursorVisible)
    {
        // We're showing the previously hidden mouse cursor. Restore the mouse cursor to the position where it
        // was when mouse was hidden.
        QApplication::restoreOverrideCursor();
        QCursor::setPos(mouseFPSModeEnterX, mouseFPSModeEnterY);
        
        ApplyMouseCursorOverride();
    }
    else
    {
        // Hide the mouse cursor and save up the coordinates where the mouse cursor was hidden.
        QApplication::setOverrideCursor(QCursor(Qt::BlankCursor));
        mouseFPSModeEnterX = QCursor::pos().x();
        mouseFPSModeEnterY = QCursor::pos().y();
    }
    
    assert(framework->Ui() && framework->Ui()->GraphicsView());
    QGraphicsView *view = framework->Ui()->GraphicsView();
    QPoint mousePos = view->mapFromGlobal(QCursor::pos());
    lastMouseX = mousePos.x();
    lastMouseY = mousePos.y();
}

bool InputAPI::IsMouseCursorVisible() const
{ 
    return mouseCursorVisible;
}

bool InputAPI::IsKeyDown(Qt::Key keyCode) const
{
    return heldKeys.find(keyCode) != heldKeys.end();
}

bool InputAPI::IsKeyPressed(Qt::Key keyCode) const
{
    return std::find(pressedKeys.begin(), pressedKeys.end(), keyCode) != pressedKeys.end();
}

bool InputAPI::IsKeyReleased(Qt::Key keyCode) const
{
    return std::find(releasedKeys.begin(), releasedKeys.end(), keyCode) != releasedKeys.end();
}

bool InputAPI::IsMouseButtonDown(int mouseButton) const
{
    assert((mouseButton & (mouseButton-1)) == 0); // Must only contain a single '1' bit.

    return (heldMouseButtons & mouseButton) != 0;
}

bool InputAPI::IsMouseButtonPressed(int mouseButton) const
{
    assert((mouseButton & (mouseButton-1)) == 0); // Must only contain a single '1' bit.

    return (pressedMouseButtons & mouseButton) != 0;
}

bool InputAPI::IsMouseButtonReleased(int mouseButton) const
{
    assert((mouseButton & (mouseButton-1)) == 0); // Must only contain a single '1' bit.

    return (releasedMouseButtons & mouseButton) != 0;
}

QPoint InputAPI::MousePressedPos(int mouseButton) const
{
    return mousePressPositions.Pos(mouseButton);
}

QPoint InputAPI::MousePos() const
{
    return QPoint(lastMouseX, lastMouseY);
}

void InputAPI::DumpInputContexts()
{
    int idx = 0;

    foreach(const weak_ptr<InputContext> &inputContext, registeredInputContexts)
    {
        InputContextPtr ic = inputContext.lock();
        if (ic)
            LogInfo("Context " + QString::number(idx++) + ": \"" + ic->Name() + "\", priority " + QString::number(ic->Priority()));
        else
            LogInfo("Context " + QString::number(idx++) + ": expired weak_ptr.");
    }

    if (untrackedInputContexts.size() > 0 )
        LogInfo("Untracked input contexts: ");
    foreach(const InputContextPtr &ic, untrackedInputContexts)
        LogInfo("Context " + QString::number(idx++) + ": \"" + ic->Name() + "\", priority " + QString::number(ic->Priority()));
}

void InputAPI::SetPriority(InputContextPtr inputContext, int newPriority)
{
    if (!inputContext)
        return;

    // When the priority of the input context changes, it must be re-inserted in sorted order into the input context list.
    for(InputContextList::iterator iter = registeredInputContexts.begin();
        iter != registeredInputContexts.end(); ++iter)
        if ((*iter).lock() == inputContext)
        {
            registeredInputContexts.erase(iter);
            break;
        }

    // Do a sorted insert: Iterate and skip through all the input contexts that have a higher
    // priority than the desired new priority.
    InputContextList::iterator iter = registeredInputContexts.begin();
    for(; iter != registeredInputContexts.end(); ++iter)
    {
        shared_ptr<InputContext> inputContext = iter->lock();
        if (!inputContext)
            continue;

        if (inputContext->Priority() <= newPriority)
            break;
    }

    // iter now points to the proper spot w.r.t the priority order. Insert there.
    registeredInputContexts.insert(iter, weak_ptr<InputContext>(inputContext));

    inputContext->priority = newPriority;
}

InputContextPtr InputAPI::RegisterInputContext(const QString &name, int priority)
{
    shared_ptr<InputContext> newInputContext = MAKE_SHARED(InputContext, this, name.toStdString().c_str(), priority);
    SetPriority(newInputContext, priority);
    return newInputContext;
}

InputContext *InputAPI::RegisterInputContextRaw(const QString &name, int priority)
{
    InputContextPtr context = RegisterInputContext(name, priority);
    untrackedInputContexts.push_back(context);
    return context.get();
}

void InputAPI::UnregisterInputContextRaw(const QString &name)
{
    for(std::list<InputContextPtr>::iterator iter = untrackedInputContexts.begin();
        iter != untrackedInputContexts.end(); ++iter)
        if ((*iter)->Name() == name)
        {
            untrackedInputContexts.erase(iter);
            return;
        }
    LogWarning("Failed to delete non-refcounted Input Context \"" + name + "\": an Input Context with that name doesn't exist!");
}

void InputAPI::ApplyMouseCursorOverride()
{
    if (!IsMouseCursorVisible())
        return;

    // If we don't have a main window (e.g. in headless mode), this doesn't do anything.
    UiGraphicsView *gv = framework->Ui()->GraphicsView();
    if (!gv)
        return;

    bool is2DUiUnderMouse = gv->GetVisibleItemAtCoords(lastMouseX, lastMouseY) != 0;

    for(InputContextList::iterator iter = registeredInputContexts.begin(); 
        iter != registeredInputContexts.end(); ++iter)
    {
        InputContext *context = (*iter).lock().get();
        if (context && context->MouseCursorOverride() && (!is2DUiUnderMouse || context->TakesMouseEventsOverQt()))
        {
            if (QApplication::overrideCursor() == 0)
                QApplication::setOverrideCursor(*context->MouseCursorOverride());
            else
                QApplication::changeOverrideCursor(*context->MouseCursorOverride());
            return;
        }
    }

    // No context currently has anything to change on the mouse cursor, so restore the original Qt cursor.

    // Note: This logic assumes exclusive control of the QApplication singleton override cursor behavior.
    while(QApplication::overrideCursor() != 0)
        QApplication::restoreOverrideCursor();
}

void InputAPI::SceneReleaseAllKeys()
{
    for(InputContextList::iterator iter = registeredInputContexts.begin(); iter != registeredInputContexts.end(); ++iter)
    {
        shared_ptr<InputContext> inputContext = iter->lock();
        if (inputContext)
            inputContext->ReleaseAllKeys();
    }
}

void InputAPI::SceneReleaseMouseButtons()
{
    for(int i = 1; i < MouseEvent::MaxButtonMask; i <<= 1)
        if ((heldMouseButtons & i) != 0)
        {
            // Just like with key release events, we send a very bare-bone release message here as well.
            MouseEvent mouseEvent;
            mouseEvent.eventType = MouseEvent::MouseReleased;
            mouseEvent.button = (MouseEvent::MouseButton)i;
            mouseEvent.x = lastMouseX;
            mouseEvent.y = lastMouseY;
            mouseEvent.z = 0;
            mouseEvent.relativeX = 0;
            mouseEvent.relativeY = 0;
            mouseEvent.relativeZ = 0;

            mouseEvent.globalX = 0;
            mouseEvent.globalY = 0;

            mouseEvent.otherButtons = 0;
        }
}

/// \bug Due to not being able to restrict the mouse cursor to the window client are in any cross-platform means,
/// it is possible that if the screen is resized to very small and if the mouse is moved very fast, the cursor
/// escapes the window client area and will not get recentered.
void InputAPI::RecenterMouse()
{
    QGraphicsView *view = framework->Ui()->GraphicsView();
    // If we don't have a main window to recenter to, or if the main window is not active (user has ALT-TABbed to another app),
    // don't recenter.
    if (!view || !framework->Ui()->MainWindow() || !framework->Ui()->MainWindow()->isActiveWindow())
        return;
    QPoint centeredCursorPosLocal = QPoint(view->size().width()/2, view->size().height()/2);
    
    lastMouseX = centeredCursorPosLocal.x();
    lastMouseY = centeredCursorPosLocal.y();
    // This call might trigger an immediate mouse move message to the window, so set the mouse coordinates above.
    QPoint centeredCursorPosGlobal = view->mapToGlobal(centeredCursorPosLocal);
    if (centeredCursorPosGlobal == QCursor::pos())
        return; // If the mouse cursor already is at the center, don't do anything.
    QCursor::setPos(centeredCursorPosGlobal);

    // Double-check that the mouse cursor did end up where we wanted it to go.
    QPoint mousePos = view->mapFromGlobal(QCursor::pos());
    lastMouseX = mousePos.x();
    lastMouseY = mousePos.y();
}

void InputAPI::UpdateTouchPoints(QTouchEvent* touchEvent)
{
    if (touchEvent)
        numTouchPoints = touchEvent->touchPoints().size();
    else
        numTouchPoints = 0;
}

void InputAPI::PruneDeadInputContexts()
{
    InputContextList::iterator iter = registeredInputContexts.begin();

    while(iter != registeredInputContexts.end())
    {
        if (iter->expired())
            iter = registeredInputContexts.erase(iter);
        else
            ++iter;
    }
}

void InputAPI::TriggerSceneKeyReleaseEvent(InputContextList::iterator start, Qt::Key keyCode)
{
    for(; start != registeredInputContexts.end(); ++start)
    {
        shared_ptr<InputContext> context = start->lock();
        if (context)
            context->TriggerKeyReleaseEvent(keyCode);
    }

    if (heldKeys.find(keyCode) != heldKeys.end())
    {
        KeyEvent keyEvent;
        keyEvent.keyCode = keyCode;
        keyEvent.eventType = KeyEvent::KeyReleased;
    }
}

void InputAPI::TriggerKeyEvent(KeyEvent &key)
{
    assert(key.eventType != KeyEvent::KeyEventInvalid);
    assert(key.handled == false);

    // First, we pass the key to the global top level input context, which operates above Qt widget input.
    topLevelInputContext.TriggerKeyEvent(key);
    if (key.handled) // Convert a Pressed event to a Released event if it was suppressed, so that lower contexts properly let go of the key.
        key.eventType = KeyEvent::KeyReleased;

    // If a widget in the QGraphicsScene has keyboard focus, don't send the keyboard message to the inworld scene (the lower contexts).
    const bool qtWidgetHasKeyboardFocus = (mainView->scene()->focusItem() && key.eventType == KeyEvent::KeyPressed);

    // If the mouse cursor is hidden, we treat each InputContext as if it had TakesKeyboardEventsOverQt true.
    // This is because when the mouse cursor is hidden, no key input should go to the main 2D UI window.

    // Pass the event to all input contexts in the priority order.
    for(InputContextList::iterator iter = registeredInputContexts.begin(); iter != registeredInputContexts.end(); ++iter)
    {
        shared_ptr<InputContext> context = iter->lock();
        if (context.get() && (!qtWidgetHasKeyboardFocus || context->TakesKeyboardEventsOverQt() || !IsMouseCursorVisible()))
            context->TriggerKeyEvent(key);
        if (key.handled)
            key.eventType = KeyEvent::KeyReleased;
    }

    if (qtWidgetHasKeyboardFocus)
        return;

    // If the mouse cursor is hidden, all key events should go to the 'scene' - In that case, suppress all key events from going to the main 2D Qt window.
    if (!IsMouseCursorVisible())
        key.Suppress();
}

void InputAPI::TriggerMouseEvent(MouseEvent &mouse)
{
    assert(mouse.handled == false);

    // Remember where this press occurred, for tracking drag situations.
    if (mouse.eventType == MouseEvent::MousePressed)
        mousePressPositions.Set(mouse.button, mouse.x, mouse.y, mouse.origin);

    // Copy over the set of tracked mouse press positions into the event structure, so that 
    // the client can do proper drag tracking.
    mouse.mousePressPositions = mousePressPositions;

    // First, we pass the event to the global top level input context, which operates above Qt widget input.
    topLevelInputContext.TriggerMouseEvent(mouse);
    if (mouse.handled)
        return;

    // If the mouse cursor is hidden, we treat each InputContext as if it had TakesMouseEventsOverQt true.
    // This is because when the mouse cursor is hidden, no mouse input should go to the main 2D UI window.

    // Pass the event to all input contexts in the priority order.
    for(InputContextList::iterator iter = registeredInputContexts.begin(); iter != registeredInputContexts.end(); ++iter)
    {
        if (mouse.handled)
            break;
        shared_ptr<InputContext> context = iter->lock();
        if (context.get() && (!mouse.itemUnderMouse || context->TakesMouseEventsOverQt() || !IsMouseCursorVisible()))
            context->TriggerMouseEvent(mouse);
    }

    // If the mouse cursor is hidden, all mouse events should go to the 'scene' - In that case, suppress all mouse events from going to the main 2D Qt window.
    if (!IsMouseCursorVisible())
        mouse.Suppress();
}

void InputAPI::TriggerGestureEvent(GestureEvent &gesture)
{
    assert(gesture.handled == false);

    // First, we pass the event to the global top level input context, which operates above Qt widget input.
    topLevelInputContext.TriggerGestureEvent(gesture);
    if (gesture.handled)
        return;

    // Pass the event to all input contexts in the priority order.
    for(InputContextList::iterator iter = registeredInputContexts.begin(); iter != registeredInputContexts.end(); ++iter)
    {
        if (gesture.handled)
            break;
        shared_ptr<InputContext> context = iter->lock();
        if (context.get())
            context->TriggerGestureEvent(gesture);
    }
}

void InputAPI::SetKeyBinding(const QString &actionName, QKeySequence key)
{
    keyboardMappings[actionName] = QKeySequence(key);
}

QKeySequence InputAPI::KeyBinding(const QString &actionName) const
{
    KeyBindingMap::const_iterator iter = keyboardMappings.find(actionName);
    return iter != keyboardMappings.end() ? iter.value() : QKeySequence();
}

QKeySequence InputAPI::KeyBinding(const QString &actionName, QKeySequence defaultKey)
{
    KeyBindingMap::const_iterator iter = keyboardMappings.find(actionName);
    if (iter == keyboardMappings.end())
    {
        SetKeyBinding(actionName, defaultKey);
        return defaultKey;
    }
    return iter.value();
}

void InputAPI::LoadKeyBindingsFromFile()
{
    ConfigAPI &cfg = *framework->Config();
    ConfigData inputConfig(ConfigAPI::FILE_FRAMEWORK, "input");
    for(int i = 0; ; ++i)
    {
        QStringList bindings = cfg.Get(inputConfig, QString("keybinding%1").arg(i)).toString().split('|');
        if (bindings.size() != 2)
            break;
        SetKeyBinding(bindings[0], bindings[1]);
    }
}

void InputAPI::SaveKeyBindingsToFile()
{
    ConfigAPI &cfg = *framework->Config();
    ConfigData inputConfig(ConfigAPI::FILE_FRAMEWORK, "input");
    int i = 0;
    for(KeyBindingMap::const_iterator iter = keyboardMappings.begin(); iter != keyboardMappings.end(); ++iter)
        cfg.Set(inputConfig, QString("keybinding%1").arg(i++), iter.key() + '|' + iter.value().toString());
}

Qt::Key StripModifiersFromKey(int qtKeyWithModifiers)
{
    // Remove the modifier bit flags from the given key with modifiers.
    // See http://doc.qt.nokia.com/4.6/qt.html#Key-enum
    // and http://doc.qt.nokia.com/4.6/qt.html#KeyboardModifier-enum
    return (Qt::Key)(qtKeyWithModifiers & 0x01FFFFFF);
}

QPoint InputAPI::MapPointToMainGraphicsView(QObject *source, const QPoint &point)
{
    QWidget *sender = qobject_cast<QWidget*>(source);
    assert(sender);

    // The mouse coordinates we receive can come from different widgets, and we are interested only in the coordinates
    // in the QGraphicsView client area, so we need to remap them.
    if (sender)
        return mainView->mapFromGlobal(sender->mapToGlobal(point));
    else
        return mainView->mapFromGlobal(QCursor::pos());
}

bool InputAPI::eventFilter(QObject *obj, QEvent *event)
{
    switch(event->type())
    {
    case QEvent::KeyPress:
    {
        QKeyEvent *e = static_cast<QKeyEvent*>(event);

        KeyEvent keyEvent;
        keyEvent.qtEvent = e;
        keyEvent.keyCode = StripModifiersFromKey(e->key());
        keyEvent.sequence = QKeySequence(e->key() | e->modifiers());
        keyEvent.keyPressCount = 1;
        keyEvent.modifiers = e->modifiers();
        keyEvent.text = e->text();
        keyEvent.sequence = QKeySequence(e->key() | e->modifiers()); ///\todo Track multi-key sequences.
        keyEvent.eventType = KeyEvent::KeyPressed;

        // Assign the keys from the heldKeys map to the keyEvent.otherHeldKeys vector
        for (std::map<Qt::Key, KeyPressInformation>::const_iterator current = heldKeys.begin(); current != heldKeys.end(); ++ current)
            keyEvent.otherHeldKeys.push_back((*current).first);

        keyEvent.handled = false;

#ifdef Q_WS_MAC
        /** @hack Recognize arrow keys for mac. Text and sequence will have garbage on mac arrow key
            presses and keyCode is not safe to access (will crash on non-ascii key presses). */
        if (keyEvent.keyCode == Qt::Key_Left)
            keyEvent.text = "Left";
        else if (keyEvent.keyCode == Qt::Key_Right)
            keyEvent.text = "Right";
        else if (keyEvent.keyCode == Qt::Key_Up)
            keyEvent.text = "Up";
        else if (keyEvent.keyCode == Qt::Key_Down)
            keyEvent.text = "Down";
#endif

        currentModifiers = e->modifiers(); // local tracking for mouse events

        // We only take key events from the main QGraphicsView.
        if (obj != qobject_cast<QObject*>(mainView))
            return false;

        std::map<Qt::Key, KeyPressInformation>::iterator keyRecord = heldKeys.find(keyEvent.keyCode);
        if (keyRecord != heldKeys.end())
        {
            if (e->isAutoRepeat()) // If this is a repeat, track the proper keyPressCount.
            {
                keyEvent.keyPressCount = ++keyRecord->second.keyPressCount;
                keyEvent.sequence = QKeySequence(); // Repeated keys do not trigger keysequences.
            }
        }
        else
        {
            KeyPressInformation info;
            info.keyPressCount = 1;
            info.keyState = KeyEvent::KeyPressed;
            //info.firstPressTime = now; ///\todo
            heldKeys[keyEvent.keyCode] = info;
        }

        // Queue up the press event for the polling API, independent of whether any Qt widget has keyboard focus.
        if (keyEvent.keyPressCount == 1) /// \todo The polling API does not get key repeats at all. Should it?
            newKeysPressedQueue.push_back(StripModifiersFromKey(e->key()));

        TriggerKeyEvent(keyEvent);

        return keyEvent.handled; // If we got true here, need to suppress this event from going to Qt.
    }

    case QEvent::KeyRelease:
    {
        // We only take key events from the main window.
        if (obj != qobject_cast<QObject*>(mainWindow))
            return false;

        QKeyEvent *e = static_cast<QKeyEvent *>(event);

        // Our input system policy: Key releases on repeated keys are not transmitted. This means
        // that the client gets always a sequences like "press (first), press(1st repeat), press(2nd repeat), release",
        // instead of "press(first), release, press(1st repeat), release, press(2nd repeat), release".
        if (e->isAutoRepeat())
            return false;

        HeldKeysMap::iterator existingKey = heldKeys.find(StripModifiersFromKey(e->key()));

        // If we received a release on an unknown key we haven't received a press for, don't pass it to the scene,
        // since we didn't pass the press to the scene either (or we've already passed the release before, so don't 
        // pass duplicate releases).
        if (existingKey == heldKeys.end())
            return false;

        KeyEvent keyEvent;
        keyEvent.qtEvent = e;
        keyEvent.keyCode = StripModifiersFromKey(e->key());
        keyEvent.keyPressCount = existingKey->second.keyPressCount;
        keyEvent.modifiers = e->modifiers();
        keyEvent.text = e->text();
        keyEvent.eventType = KeyEvent::KeyReleased;
        //keyEvent.otherHeldKeys = heldKeys; ///\todo
        keyEvent.handled = false;

#ifdef Q_WS_MAC
        /** @hack Recognize arrow keys for mac. Text and sequence will have garbage on mac arrow key 
            presses and keyCode is not safe to access (will crash on non-ascii key presses). */
        if (keyEvent.keyCode == Qt::Key_Left)
            keyEvent.text = "Left";
        else if (keyEvent.keyCode == Qt::Key_Right)
            keyEvent.text = "Right";
        else if (keyEvent.keyCode == Qt::Key_Up)
            keyEvent.text = "Up";
        else if (keyEvent.keyCode == Qt::Key_Down)
            keyEvent.text = "Down";
#endif

        heldKeys.erase(existingKey);
        currentModifiers = e->modifiers(); // local tracking for mouse events

        // Queue up the release event for the polling API, independent of whether any Qt widget has keyboard focus.
        if (keyEvent.keyPressCount == 1) /// \todo The polling API does not get key repeats at all. Should it?
            newKeysReleasedQueue.push_back(StripModifiersFromKey(e->key()));

        TriggerKeyEvent(keyEvent);
        
        return keyEvent.handled; // Suppress this event from going forward.
    }

    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseButtonDblClick:
    {
        // We only take mouse button press and release events from the main QGraphicsView viewport.
        if (obj != qobject_cast<QObject*>(mainView->viewport()))
            return false;

        QMouseEvent *e = static_cast<QMouseEvent *>(event);
        //QGraphicsItem *itemUnderMouse = GetVisibleItemAtCoords(e->x(), e->y());
/*
        // Update the flag that tracks whether the inworld scene or QGraphicsScene is grabbing mouse movement.
        if (event->type() == QEvent::MouseButtonPress)
            sceneMouseCapture = (itemUnderMouse ? QtMouseCapture : SceneMouseCapture);
        else // event type == MouseButtonRelease
            sceneMouseCapture = NoMouseCapture; 
*/
        // We always update the global polled input states, independent of whether any the mouse cursor is
        // on top of any Qt widget.
        if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonDblClick)
        {
            heldMouseButtons |= (MouseEvent::MouseButton)e->button();
            newMouseButtonsPressedQueue |= (MouseEvent::MouseButton)e->button();
        }
        else
        {
            heldMouseButtons &= ~(MouseEvent::MouseButton)e->button();
            newMouseButtonsReleasedQueue |= (MouseEvent::MouseButton)e->button();
        }

        // The mouse coordinates we receive can come from different widgets, and we are interested only in the coordinates
        // in the QGraphicsView client area, so we need to remap them.
        QPoint mousePos = MapPointToMainGraphicsView(obj, e->pos());

        MouseEvent mouseEvent;
        mouseEvent.itemUnderMouse = ItemAtCoords(e->x(), e->y());
        mouseEvent.origin = mouseEvent.itemUnderMouse ? MouseEvent::PressOriginQtWidget : MouseEvent::PressOriginScene;
        switch(event->type())
        {
        case QEvent::MouseButtonPress: mouseEvent.eventType = MouseEvent::MousePressed; break;
        case QEvent::MouseButtonDblClick: mouseEvent.eventType = MouseEvent::MouseDoubleClicked; break;
        case QEvent::MouseButtonRelease: mouseEvent.eventType = MouseEvent::MouseReleased; break;
        }

        mouseEvent.button = (MouseEvent::MouseButton)e->button();
        mouseEvent.x = mousePos.x();
        mouseEvent.y = mousePos.y();
        mouseEvent.z = 0;
        // Mouse presses do not carry relative mouse movement information at the same time.
        // (separate relative movement messages are passed for first-person mode moves)
        mouseEvent.relativeX = 0;
        mouseEvent.relativeY = 0;
        
        mouseEvent.relativeZ = 0;
        mouseEvent.modifiers = currentModifiers;

        lastMouseX = mouseEvent.x;
        lastMouseY = mouseEvent.y;

        mouseEvent.globalX = e->globalX();
        mouseEvent.globalY = e->globalY();

        mouseEvent.otherButtons = e->buttons();

        //mouseEvent.heldKeys = heldKeys; ///\todo
        mouseEvent.handled = false;

        // If the mouse press is going to the inworld scene, clear keyboard focus from the QGraphicsScene widget (if any had it) so key events also go to inworld scene.
        if ((event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonDblClick) && !mouseEvent.itemUnderMouse && mouseCursorVisible)
            mainView->scene()->clearFocus();

        TriggerMouseEvent(mouseEvent);
        return mouseEvent.handled;
    }

    case QEvent::MouseMove:
    {
        // If a mouse button is held down, we get the mouse drag events from the viewport widget.
        // If a mouse button is not held down, the application main window will get the events.
        // Duplicate events are not received, so no need to worry about filtering them here.
        QMouseEvent *e = static_cast<QMouseEvent *>(event);

        //QGraphicsItem *itemUnderMouse = GetVisibleItemAtCoords(e->x(), e->y());
        // If there is a graphicsItem under the mouse, don't pass the move message to the inworld scene, unless the inworld scene has captured it.
        //if (mouseCursorVisible)
        //    if ((itemUnderMouse && sceneMouseCapture != SceneMouseCapture) || sceneMouseCapture == QtMouseCapture)
        //        return false;
        //
        //if (mouseCursorVisible && itemUnderMouse)
        //    return false;

        MouseEvent mouseEvent;
        mouseEvent.eventType = MouseEvent::MouseMove;
        mouseEvent.button = (MouseEvent::MouseButton)e->button();
        mouseEvent.itemUnderMouse = ItemAtCoords(e->x(), e->y());
        mouseEvent.origin = mouseEvent.itemUnderMouse ? MouseEvent::PressOriginQtWidget : MouseEvent::PressOriginScene;

        assert(qobject_cast<QWidget*>(obj));

        // The mouse coordinates we receive can come from different widgets, and we are interested only in the coordinates
        // in the QGraphicsView client area, so we need to remap them.
        QPoint mousePos = MapPointToMainGraphicsView(obj, e->pos());

        mouseEvent.z = 0;
        mouseEvent.relativeX = mousePos.x() - lastMouseX;
        mouseEvent.relativeY = mousePos.y() - lastMouseY;
        mouseEvent.relativeZ = 0;

        if (mouseCursorVisible)
        {
            mouseEvent.x = mousePos.x();
            mouseEvent.y = mousePos.y();
        }
        else
        {
            // If mouse cursor is hidden, we're in relative "crosshair" mode. In this mode,
            // the mouse absolute coordinates are restricted to stay in the center of the screen.
            mouseEvent.x = mainView->size().width()/2;
            mouseEvent.y = mainView->size().height()/2;

            QGraphicsView *view = framework->Ui()->GraphicsView();
            QPoint centeredCursorPosLocal = QPoint(view->size().width()/2, view->size().height()/2);

            // If the main window is not active, the mouse recentering logic is not active either.
            // If that is the case, don't output relative movement information (because it couldn't be calculated properly)
            if (!mainWindow || !mainWindow->isActiveWindow() || centeredCursorPosLocal == mousePos)
            {
                mouseEvent.relativeX = 0;
                mouseEvent.relativeY = 0;
                lastMouseX = centeredCursorPosLocal.x();
                lastMouseY = centeredCursorPosLocal.y();
            }
        }

        // If there wasn't any change to the mouse relative coords in FPS mode, ignore this event.
        if (!mouseCursorVisible && mouseEvent.relativeX == 0 && mouseEvent.relativeY == 0)
            return true;
        
        mouseEvent.globalX = e->globalX(); // Note that these may "jitter" when mouse is in relative movement mode.
        mouseEvent.globalY = e->globalY();
        mouseEvent.otherButtons = e->buttons();
        //mouseEvent.heldKeys = heldKeys; ///\todo
        mouseEvent.handled = false;

        // Save the absolute coordinates to be able to compute the proper relative movement values in the next
        // mouse event.
        lastMouseX = mouseEvent.x;
        lastMouseY = mouseEvent.y;

        TriggerMouseEvent(mouseEvent);

        // In relative mouse movement mode, keep the mouse cursor hidden at screen center at all times.
        if (!mouseCursorVisible)
        {
            RecenterMouse();
            return true; // In relative mouse movement mode, the QGraphicsScene does not receive mouse movement at all.
        }

        return mouseEvent.handled;
    }

    case QEvent::DragMove:
    {
        QDragMoveEvent *e = static_cast<QDragMoveEvent*>(event);
        lastMouseX = e->pos().x();
        lastMouseY = e->pos().y();
        break;
    }

    case QEvent::Wheel:
    {
        // If this event did not originate from the QGraphicsView viewport, we are not interested in it.
        if (obj != qobject_cast<QObject*>(mainView))
            return false;

        QWheelEvent *e = static_cast<QWheelEvent *>(event);
        //QGraphicsItem *itemUnderMouse = GetVisibleItemAtCoords(e->x(), e->y());
        //if (itemUnderMouse)
        //    return false;

        MouseEvent mouseEvent;
        mouseEvent.eventType = MouseEvent::MouseScroll;
        mouseEvent.itemUnderMouse = ItemAtCoords(e->x(), e->y());
        mouseEvent.origin = mouseEvent.itemUnderMouse ? MouseEvent::PressOriginQtWidget : MouseEvent::PressOriginScene;
        mouseEvent.button = MouseEvent::NoButton;
        mouseEvent.otherButtons = e->buttons();
        mouseEvent.x = e->x();
        mouseEvent.y = e->y();
        mouseEvent.z = 0; // Mouse wheel does not have an absolute z position, only relative.
        mouseEvent.relativeX = 0;
        mouseEvent.relativeY = 0;
        mouseEvent.relativeZ = e->delta();
        mouseEvent.globalX = e->globalX();
        mouseEvent.globalY = e->globalY();

        mouseEvent.otherButtons = e->buttons(); ///\todo Can this be trusted?

        //mouseEvent.heldKeys = heldKeys; ///\todo
        mouseEvent.handled = false;

        TriggerMouseEvent(mouseEvent);

        // Always suppress the wheel events from going to the QGraphicsView, or otherwise the wheel will start to
        // scroll the main QGraphicsView UI windows vertically.
        return true;
    }

    case QEvent::Gesture:
    {
        if (!gesturesEnabled)
            gesturesEnabled = true;

        QGestureEvent *e = static_cast<QGestureEvent *>(event);
        QList<QGesture*> gestures = e->gestures();
        foreach(QGesture *gesture, gestures)
        {
            GestureEvent gestureEvent;
            gestureEvent.gesture = gesture;
            gestureEvent.gestureType = gesture->gestureType();
            gestureEvent.eventType = (GestureEvent::EventType)gesture->state();
            TriggerGestureEvent(gestureEvent);
            e->setAccepted(gesture, gestureEvent.handled);
        }
        e->accept();
        return true;
    }
    case QEvent::TouchBegin:
        {
            QTouchEvent* touchEvent = static_cast<QTouchEvent *>(event);
            UpdateTouchPoints(touchEvent);
            emit TouchBegin(touchEvent);
            event->accept();
        }
        return true;
    case QEvent::TouchUpdate:
        {
            QTouchEvent* touchEvent = static_cast<QTouchEvent *>(event);
            UpdateTouchPoints(touchEvent);
            emit TouchUpdate(touchEvent);
            event->accept();
        }
        return true;
    case QEvent::TouchEnd:
        {
            QTouchEvent* touchEvent = static_cast<QTouchEvent *>(event);
            UpdateTouchPoints(0);
            emit TouchEnd(touchEvent);
            event->accept();
        }
        return true;
    } // ~switch

    return QObject::eventFilter(obj, event);
}

QGraphicsItem* InputAPI::ItemAtCoords(int x, int y) const
{
    // If the mouse cursor is hidden, act as if there was no item
    if (IsMouseCursorVisible())
        return framework->Ui()->GraphicsView()->GetVisibleItemAtCoords(x, y);
    else
        return 0;
}

void InputAPI::ClearFocus()
{
    if (mainView)
        mainView->scene()->clearFocus();
}

void InputAPI::Update(float /*frametime*/)
{
    PROFILE(InputAPI_Update);

    // If at any time we don't have main application window focus, release all input
    // so that keys don't get stuck when the window is reactivated. (The key release might be passed
    // to another window instead and our app keeps thinking that the key is being held down.)
    if (!QApplication::activeWindow())
    {
        SceneReleaseAllKeys();
        SceneReleaseMouseButtons();
    }

    // Move all the double-buffered input events to current events.
    pressedKeys = newKeysPressedQueue;
    newKeysPressedQueue.clear();

    releasedKeys = newKeysReleasedQueue;
    newKeysReleasedQueue.clear();

    pressedMouseButtons = newMouseButtonsPressedQueue;
    newMouseButtonsPressedQueue = 0;

    releasedMouseButtons = newMouseButtonsReleasedQueue;
    newMouseButtonsReleasedQueue = 0;

    // Update the new frame start to all input contexts.
    topLevelInputContext.UpdateFrame();

    for(InputContextList::iterator iter = registeredInputContexts.begin();
        iter != registeredInputContexts.end(); ++iter)
    {
        shared_ptr<InputContext> inputContext = (*iter).lock();
        if (inputContext)
            inputContext->UpdateFrame();
    }

    // Guarantee that we are showing the desired mouse cursor.
    ApplyMouseCursorOverride();

    PruneDeadInputContexts();
}

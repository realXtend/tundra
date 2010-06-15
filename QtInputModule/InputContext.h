// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_QtInputModule_InputContext_h
#define incl_QtInputModule_InputContext_h

#include <map>

#include "QtInputModuleApi.h"

#include "QtInputKeyEvent.h"
#include "QtInputMouseEvent.h"

#include "KeyEventSignal.h"

class QTINPUT_MODULE_API InputContext : public QObject
{
    Q_OBJECT

signals:
    void OnKeyEvent(KeyEvent &key);
    void OnMouseEvent(MouseEvent &mouse);

    // The following signals are invoked for all keys.
    void KeyPressed(KeyEvent &key);
    void KeyDown(KeyEvent &key);
    void KeyReleased(KeyEvent &key);

    void MouseLeftPressed(MouseEvent &mouse);
    void MouseMiddlePressed(MouseEvent &mouse);
    void MouseRightPressed(MouseEvent &mouse);

    void MouseMove(MouseEvent &mouse);
    void MouseScroll(MouseEvent &mouse);

    void MouseLeftReleased(MouseEvent &mouse);
    void MouseMiddleReleased(MouseEvent &mouse);
    void MouseRightReleased(MouseEvent &mouse);

public:
    explicit InputContext(const char *name);

    KeyEventSignal &RegisterKeyEvent(Qt::Key keyCode);
    void UnregisterKeyEvent(Qt::Key keyCode);

    void TriggerKeyEvent(KeyEvent &key);
    void TriggerMouseEvent(MouseEvent &mouse);

    /// Returns the user-defined name associated with this InputContext. The name is
    /// read-only, and associated with the context at creation time.
    std::string Name() const { return name; }

private:
    typedef std::map<Qt::Key, KeyEventSignal> KeyEventSignalMap;
    KeyEventSignalMap registeredKeyEventSignals;

    std::string name;

    // InputContext are noncopyable.
    InputContext(const InputContext &);
    void operator=(const InputContext &);
};

#endif

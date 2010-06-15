// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_QtInputModule_KeyEventSignal_h
#define incl_QtInputModule_KeyEventSignal_h

#include <QObject>

#include "QtInputModuleApi.h"

#include "QtInputKeyEvent.h"
#include "QtInputMouseEvent.h"

class QTINPUT_MODULE_API KeyEventSignal : public QObject
{
    Q_OBJECT

signals:
    void KeyPressed(KeyEvent &key);
    void KeyDown(KeyEvent &key);
    void KeyReleased(KeyEvent &key);

public:
    /// The Qt keycode that this signal object corresponds to.
    Qt::Key keyCode;

    void OnKeyPressed(KeyEvent &key) { emit KeyPressed(key); }
    void OnKeyDown(KeyEvent &key) { emit KeyDown(key); }
    void OnKeyReleased(KeyEvent &key) { emit KeyReleased(key); }
};

#endif


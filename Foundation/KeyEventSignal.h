// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_QtInputModule_KeyEventSignal_h
#define incl_QtInputModule_KeyEventSignal_h

#include <QObject>

#include "QtInputKeyEvent.h"
#include "QtInputMouseEvent.h"

class KeyEventSignal : public QObject
{
    Q_OBJECT

signals:
    void SequencePressed(KeyEvent &key);

    void SequenceReleased(KeyEvent &key);

public:
    explicit KeyEventSignal(QKeySequence keySequence_)
    :keySequence(keySequence_)
    {}

    /// This is the key sequence that this key signal is triggered for.
    const QKeySequence keySequence;

public slots:
    void OnKeyPressed(KeyEvent &key) { emit SequencePressed(key); }
//    void OnKeyDown(KeyEvent &key) { emit KeyDown(key); }
    void OnKeyReleased(KeyEvent &key) { emit SequenceReleased(key); }

private:
    // KeyEventSignal is noncopyable.
    KeyEventSignal(const KeyEventSignal &);
    void operator=(const KeyEventSignal &);
};

#endif


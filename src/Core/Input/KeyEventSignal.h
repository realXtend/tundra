// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include <QObject>

#include "KeyEvent.h"
#include "MouseEvent.h"

/// A signal object for input events to a specific key on the keyboard.
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


